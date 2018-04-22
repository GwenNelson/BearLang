#include <bearlang/common.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>
#include <bearlang/utils.h>

#include <bl_lexer.h>
#include <stdio.h>

void bl_init_parser() {
}

bl_val_t* read_form(yyscan_t scanner);

bl_val_t* read_list(yyscan_t scanner) {
    bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_CONS);
    retval->car = read_form(scanner);
    retval->cdr = NULL;
    
    if(retval->car->type == BL_VAL_TYPE_LIST_END) retval->car = NULL;

    if(retval->car == NULL) return retval;

    bl_val_t* L = retval;
    bl_val_t* e = bl_mk_null();
    while(e->type != BL_VAL_TYPE_LIST_END) {
       e = read_form(scanner);
       if(e != NULL) if(e->type != BL_VAL_TYPE_LIST_END) {
          L->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
	  L->cdr->car = e;
	  L->cdr->cdr = NULL;
	  L = L->cdr;
       }
    }
    return retval;
}

bl_val_t* read_form(yyscan_t scanner) {
    bl_token_type_t tok = yylex(scanner);
    if(tok == 0) return NULL;
    switch(tok) {
	case BL_TOKEN_STRING:
		return bl_mk_str(yyget_text(scanner));
	break;
	case BL_TOKEN_LPAREN:
		return read_list(scanner);
	break;
	case BL_TOKEN_RPAREN:
		return bl_mk_val(BL_VAL_TYPE_LIST_END);
	break;
	case BL_TOKEN_FLOAT:
		return bl_mk_float(atof(yyget_text(scanner)));
	break;
	case BL_TOKEN_INTEGER:
		return bl_mk_number(atoi(yyget_text(scanner)));
	break;
	case BL_TOKEN_SYMBOL:
		return bl_mk_symbol(yyget_text(scanner));
	break;
   }
 
}

bl_val_t* bl_parse_sexp(char* sexp) {
   yyscan_t scanner;
   yylex_init(&scanner);
   yy_scan_string(sexp,scanner);
   bl_val_t* retval = read_form(scanner);
   yylex_destroy(scanner);
   return retval;
}

bl_val_t* bl_parse_file(char* filename, FILE* fd) {
   yyscan_t scanner;
   yylex_init(&scanner);
   YY_BUFFER_STATE buf = yy_create_buffer(fd, YY_BUF_SIZE,scanner);
   yy_switch_to_buffer(buf,scanner);
   bl_val_t* retval = read_list(scanner);
   yylex_destroy(scanner);
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
           retval = (char*)GC_MALLOC(sizeof(char)*6);
           snprintf(retval, 5, "None");
         break;
	 case BL_VAL_TYPE_CTX:
	   retval = (char*)GC_MALLOC(sizeof(char)*6);
	   snprintf(retval,5,"<ctx>");
	 case BL_VAL_TYPE_ERROR:
           retval = (char*)GC_MALLOC(sizeof(char)*10);
	   snprintf(retval, 9, "<error>");
	 break;
         case BL_VAL_TYPE_SYMBOL:
           retval = (char*)GC_MALLOC(sizeof(char)*(strlen(expr->s_val)+2));
           snprintf(retval,strlen(expr->s_val)+1,"%s",expr->s_val);
         break;
         case BL_VAL_TYPE_STRING:
           retval = (char*)GC_MALLOC(sizeof(char)*(strlen(expr->s_val)+5));
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
           retval = (char*)GC_MALLOC(sizeof(char)*10);
	   snprintf(retval,5,"OPER");
	 break;
	 case BL_VAL_TYPE_CONS:
           retval = (char*)GC_MALLOC(sizeof(char)*1024);
	   retval[0]='(';
           if((expr->car == NULL) && (expr->cdr == NULL)) {
     	       snprintf(retval,4,"%s","()");
	   } else {
               bl_val_t* L=expr;
	       while(L->cdr != NULL) {
                  if(L->car != NULL) {
                     char* newval = bl_ser_sexp(L->car);
                     snprintf(retval,1024,"%s%s ", retval, newval);
		  }
                  L = L->cdr;
	       }
               if(L->car != NULL) {
                  char* newval = bl_ser_sexp(L->car);
                  snprintf(retval,1024,"%s%s)", retval, newval);
	       }
	   }

         break;
      }
      return retval;
}

char* bl_ser_naked_sexp(bl_val_t* expr) {
      char* retval = bl_ser_sexp(expr);
      if(expr->type == BL_VAL_TYPE_STRING) {
         retval++;
	 retval[strlen(retval)-1]='\0';
      }
      return retval++;
}

