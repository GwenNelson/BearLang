#include <bearlang/common.h>
#include <bearlang/sexp.h>
#include <bearlang/mpc.h>
#include <bearlang/list_ops.h>

#include <stdio.h>

mpc_parser_t* Number;
mpc_parser_t* String;
mpc_parser_t* Comment;
mpc_parser_t* Symbol;
mpc_parser_t* Sexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lispy;

void bl_init_parser() {
     Number  = mpc_new("number");
     String  = mpc_new("string");
     Comment = mpc_new("comment");
     Symbol  = mpc_new("symbol");
     Sexpr   = mpc_new("sexpr");
     Expr    = mpc_new("expr");
     Lispy   = mpc_new("lispy");

     mpca_lang(MPCA_LANG_DEFAULT,
      "                                            \
        number : /-?[0-9]+/ ;                      \
	symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/; \
	string  : /\"(\\\\.|[^\"])*\"/ ;           \
        comment : /;[^\\r\\n]*/ ;                    \
        sexpr  : '(' <expr>* ')' ;                 \
        expr   : <number> | <symbol> | <string> | <comment> | <sexpr> ;   \
        lispy  : /^/ <expr>* /$/ ;                 \
      ",
      Number, Symbol, String, Comment, Sexpr, Expr, Lispy);
}

bl_ast_node_t* mpc_to_bl(mpc_ast_t* T) {
      bl_ast_node_t* retval = (bl_ast_node_t*)GC_MALLOC(sizeof(bl_ast_node_t));
      char* s = NULL;

      if(strstr(T->tag,"number")) {
         retval->node_val.type      = BL_VAL_TYPE_NUMBER;
         retval->node_val.i_val = atoi(T->contents);
         return retval;
      }

      if(strstr(T->tag,"string")) {
         int content_len = strlen(T->contents);
	 retval->node_val.type  = BL_VAL_TYPE_STRING;
	 retval->node_val.s_val = (char*)GC_MALLOC(content_len+1);
	 snprintf(retval->node_val.s_val,content_len-1,"%s",T->contents+1);
         retval->node_val.s_val = mpcf_unescape(retval->node_val.s_val);
	 return retval;
      }

      if(strstr(T->tag,"symbol")) {
         int content_len = strlen(T->contents);
         retval->node_val.type      = BL_VAL_TYPE_SYMBOL;
         retval->node_val.s_val = (char*)GC_MALLOC(content_len+1);
         snprintf(retval->node_val.s_val,content_len+1,"%s",T->contents);
         return retval;
      }

      if((strcmp(T->tag,">") == 0) || strstr(T->tag, "sexpr")) {
         retval->node_val.type = BL_VAL_TYPE_AST_LIST;
         retval->children      = (bl_ast_node_t**)GC_MALLOC(sizeof(bl_ast_node_t*)*T->children_num);
      }
      
      int i=0;
      int c=0;
      for(i=0; i < T->children_num; i++) {
          if (strcmp(T->children[i]->contents, "(") == 0) { continue; }
          if (strcmp(T->children[i]->contents, ")") == 0) { continue; }
          if (strcmp(T->children[i]->tag,  "regex") == 0) { continue; }
	  if (strstr(T->children[i]->tag, "comment")) { continue; }
          bl_ast_node_t* parsed = mpc_to_bl(T->children[i]);
          retval->children[c] = parsed;
          c++;
      }
      retval->child_count = c;
      return retval;
}

bl_ast_node_t* bl_parse_sexp(char* sexp) {
      mpc_result_t   r;
      bl_ast_node_t* retval = NULL;
      // TODO - come up with a proper error handling approach
      if(mpc_parse("input",sexp, Lispy, &r)) {
         mpc_ast_t* mpc_ast = r.output;
         
	 retval = mpc_to_bl(mpc_ast->children[1]);
         mpc_ast_delete(r.output);
      } else {
         mpc_err_print(r.error);
         mpc_err_delete(r.error);
         
      }
      return retval;
}

bl_ast_node_t* bl_parse_file(char* filename, FILE* fd) {
      mpc_result_t   r;
      bl_ast_node_t* retval = NULL;
      if(mpc_parse_file(filename, fd,Lispy, &r)) {
         mpc_ast_t* mpc_ast = r.output;
         
	 retval = mpc_to_bl(mpc_ast);
         mpc_ast_delete(r.output);
      } else {
         mpc_err_print(r.error);
         mpc_err_delete(r.error);
         
      }
      return retval;

}

char* bl_ser_ast(bl_ast_node_t* ast) {
      char* retval = "";
      switch(ast->node_val.type) {
         case BL_VAL_TYPE_NULL:
           retval = (char*)GC_MALLOC(sizeof(char)*5);
           snprintf(retval, 5, "None");
         break;
         case BL_VAL_TYPE_AST_LIST:
           retval = (char*)GC_MALLOC(sizeof(char)*3);
           snprintf(retval,2,"(");
           int i=0;
           for(i=0; i < ast->child_count; i++) {
               char* tmpbuf = bl_ser_ast(ast->children[i]);
               retval = (char*)GC_realloc(retval,strlen(retval)+strlen(tmpbuf)+3);
               strncat(retval, (const char*)tmpbuf,strlen(tmpbuf));
               if(i < (ast->child_count-1)) strncat(retval, " ",1);
           }
           strncat(retval,")",1);
         break;
         case BL_VAL_TYPE_SYMBOL:
           retval = (char*)GC_MALLOC(sizeof(char)*(strlen(ast->node_val.s_val)+1));
           snprintf(retval,strlen(ast->node_val.s_val)+1,"%s",ast->node_val.s_val);
         break;
         case BL_VAL_TYPE_STRING:
           retval = (char*)GC_MALLOC(sizeof(char)*(strlen(ast->node_val.s_val)+1));
           snprintf(retval,strlen(ast->node_val.s_val)+3,"\"%s\"",ast->node_val.s_val);
         break;
         case BL_VAL_TYPE_NUMBER:
           retval = (char*)GC_MALLOC(sizeof(char)*10); // TODO - switch numbers to libgmp
           snprintf(retval,10,"%llu",ast->node_val.i_val);
         break;
	 default:

	 break;
      }
      return retval;
}

char* bl_ser_sexp(bl_val_t* expr) {
      if(expr == NULL) return "";
      char* retval="";
      char* s="";
      switch(expr->type) {
         case BL_VAL_TYPE_ANY:
           retval = (char*)GC_MALLOC(sizeof(char)*6);
	   snprintf(retval,5,"<any>");
	 break;
         case BL_VAL_TYPE_FUNC_NATIVE:
           retval = (char*)GC_MALLOC(sizeof(char)*64);
	   snprintf(retval,32,"<nativefunction>");
         case BL_VAL_TYPE_AST_LIST:
           retval = (char*)GC_MALLOC(sizeof(char)*10);
	   snprintf(retval,10,"<astlist>");
	 break;
	 case BL_VAL_TYPE_NULL:
           retval = (char*)GC_MALLOC(sizeof(char)*5);
           snprintf(retval, 5, "None");
         break;
	 case BL_VAL_TYPE_CTX:
	   retval = (char*)GC_MALLOC(sizeof(char)*5);
	   snprintf(retval,5,"<ctx>");
	 case BL_VAL_TYPE_ERROR:
           retval = (char*)GC_MALLOC(sizeof(char)*6);
	   snprintf(retval, 6, "<error>");
	 break;
         case BL_VAL_TYPE_SYMBOL:
           retval = (char*)GC_MALLOC(sizeof(char)*(strlen(expr->s_val)+1));
           snprintf(retval,strlen(expr->s_val)+1,"%s",expr->s_val);
         break;
         case BL_VAL_TYPE_STRING:
           retval = (char*)GC_MALLOC(sizeof(char)*(strlen(expr->s_val)+1));
           snprintf(retval,strlen(expr->s_val)+3,"\"%s\"",expr->s_val);
         break;
         case BL_VAL_TYPE_NUMBER:
           retval = (char*)GC_MALLOC(sizeof(char)*10); // TODO - switch numbers to libgmp
           snprintf(retval,10,"%llu",expr->i_val);
         break;
	 case BL_VAL_TYPE_BOOL:
           retval = (char*)GC_MALLOC(sizeof(char)*10);
           if(expr->i_val == 1) {
              snprintf(retval,10,"True");
	   } else {
              snprintf(retval,10,"False");
	   }
	 break;
         case BL_VAL_TYPE_FUNC_BL:
           // TODO - dynamically figure out the length of the string here
           retval = (char*)GC_MALLOC(1024);
	   snprintf(retval,1024,"(fn %s %s)",bl_ser_sexp(expr->bl_funcargs_ptr), bl_ser_sexp(expr->bl_func_ptr));
	 break;
         case BL_VAL_TYPE_OPER_NATIVE:
           retval = (char*)GC_MALLOC(sizeof(char)*4);
	   snprintf(retval,4,"OPER");
	 break;
	 case BL_VAL_TYPE_CONS:
           retval = (char*)GC_MALLOC(sizeof(char)*3);
	   retval[0]='(';
           if((expr->car == NULL) && (expr->cdr == NULL)) {
     	       snprintf(retval,4,"%s","()");
	   } else {
               bl_val_t* L=expr;
	       while(L->cdr != NULL) {
                  if(L->car != NULL) {
                     char* newval = bl_ser_sexp(L->car);
                     size_t newsize =  sizeof(char) * (strlen(retval)+strlen(newval)+4);
                     retval = (char*)GC_realloc(retval, newsize);
                     snprintf(retval,newsize,"%s%s ", retval, newval);
		  }
                  L = L->cdr;
	       }
               if(L->car != NULL) {
                  char* newval = bl_ser_sexp(L->car);
                  size_t newsize =  sizeof(char) * (strlen(retval)+strlen(newval)+4);
                  retval = (char*)GC_realloc(retval, newsize);
                  snprintf(retval,newsize,"%s%s)", retval, newval);
	       }
	   }

         break;
      }
      return retval;
}

bl_val_t* bl_read_ast(bl_ast_node_t* ast) {
      
      if(ast==NULL) return NULL;
      bl_val_t* retval = NULL;
      switch(ast->node_val.type) {
         case BL_VAL_TYPE_NULL:
              return NULL;
	 break;
	 case BL_VAL_TYPE_SYMBOL:
              retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
              retval->type = BL_VAL_TYPE_SYMBOL;
	      retval->s_val = (char*)GC_MALLOC(sizeof(char)*(strlen(ast->node_val.s_val)+1));
	      snprintf(retval->s_val,strlen(ast->node_val.s_val)+1,"%s",ast->node_val.s_val);
	      return retval;
	 break;
	 case BL_VAL_TYPE_STRING:
              retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
              retval->type = BL_VAL_TYPE_STRING;
	      retval->s_val = (char*)GC_MALLOC(sizeof(char)*(strlen(ast->node_val.s_val)+1));
	      snprintf(retval->s_val,strlen(ast->node_val.s_val)+1,"%s",ast->node_val.s_val);
	      return retval;
	 break;
	 case BL_VAL_TYPE_AST_LIST:
              retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
              retval->type = BL_VAL_TYPE_CONS;
              retval->car  = bl_read_ast(ast->children[0]);
	      retval->cdr  = NULL;
	      int i=1;
	      for(i=1; i < ast->child_count; i++) {
                  retval = bl_list_append(retval, bl_read_ast(ast->children[i]));
	      }
	      return retval;
	 break;
	 case BL_VAL_TYPE_NUMBER:
              retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
              retval->type  = BL_VAL_TYPE_NUMBER;
	      retval->i_val = ast->node_val.i_val;
	      return retval;
	 break;
	 default:
	      retval = NULL; // TODO - throw an error here or something
	 break;
      }
      return retval;
}
