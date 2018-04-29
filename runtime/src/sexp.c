#include <bearlang/common.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>
#include <bearlang/utils.h>
#include <bearlang/error_tools.h>
#include <bl_lexer.h>
#include <stdio.h>

void bl_init_parser() {
}

bl_val_t* read_form(yyscan_t scanner);

bl_val_t* read_list(yyscan_t scanner) {
    bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_CONS);
    retval->car = read_form(scanner);
    retval->cdr = NULL;
    if(retval->car == NULL) return retval; 
    if(retval->car->type == BL_VAL_TYPE_LIST_END) {
       retval->car = NULL;
       return retval;
    }
    


    bl_val_t* L = retval;
    bl_val_t* e = bl_mk_null();
    while(e->type != BL_VAL_TYPE_LIST_END) {
       e = read_form(scanner);
       if(e==NULL) return retval;
       if(e->type != BL_VAL_TYPE_LIST_END) {
          L->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
	  L->cdr->car = e;
	  L->cdr->cdr = NULL;
	  L = L->cdr;
       }
    }
    return retval;
}

char* unescape(char* s) {
    size_t count = strlen(s);
    char* retval = GC_MALLOC_ATOMIC(sizeof(s) * count+1);
    bool slash;
    int i, j;
    for(i=j=0; s[i] != '\0'; i++) {
        switch(s[i]) {
	   case '\\':
	     slash = true;
	   break;
	   case 'n':
	      if(slash) {
	         retval[j++] = '\n';
		 slash = false;
	         break;
	      }
	   case 't':
	      if(slash) {
		 retval[j++] = '\t';
		 slash = false;
	         break;
	      }
	   default:
	      retval[j++] = s[i];
	      break;
	}
    }
    retval[j] = '\0';
    return retval;
}

bl_val_t if_oper_val  = {.type = BL_VAL_TYPE_OPER_IF};
bl_val_t do_oper_val  = {.type = BL_VAL_TYPE_OPER_DO};
bl_val_t end_list_val = {.type = BL_VAL_TYPE_LIST_END};

bl_val_t* read_form(yyscan_t scanner) {
    char* s = NULL;
    bl_token_type_t tok = yylex(scanner);
    if(tok == 0) return NULL;
    switch(tok) {
	case BL_TOKEN_STRING:
		return bl_mk_str(unescape(yyget_text(scanner)));
	break;
	case BL_TOKEN_LPAREN:
		return read_list(scanner);
	break;
	case BL_TOKEN_RPAREN:
		return &end_list_val;
	break;
	case BL_TOKEN_FLOAT:
		return bl_mk_float(atof(yyget_text(scanner)));
	break;
	case BL_TOKEN_INTEGER:
		return bl_mk_number(atoi(yyget_text(scanner)));
	break;
	case BL_TOKEN_SYMBOL:
		s = yyget_text(scanner);
		if(strncmp(s,"if",2)==0) return &if_oper_val;
		if(strncmp(s,"do",2)==0) return &do_oper_val;
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
   yyset_in(fd,scanner);
   bl_val_t* retval = read_list(scanner);
   yylex_destroy(scanner);
   return retval;
}

char* bl_ser_sexp(bl_val_t* expr) {
      if(expr == NULL) return "";
      char* retval=GC_MALLOC_ATOMIC(1024);
      char* s="";
      switch(expr->type) {
         case BL_VAL_TYPE_ANY:
	   snprintf(retval,5,"<any>");
	 break;
         case BL_VAL_TYPE_OPER_IF:
	   snprintf(retval,3,"if");
	 break;
	 case BL_VAL_TYPE_FUNC_NATIVE:
	   snprintf(retval,32,"<nativefunction>");
	 case BL_VAL_TYPE_NULL:
           snprintf(retval, 5, "None");
         break;
	 case BL_VAL_TYPE_CTX:
	   snprintf(retval,6,"<ctx>");
	 break;
	 case BL_VAL_TYPE_ERROR:
           retval = bl_errmsg(expr);
	 break;
         case BL_VAL_TYPE_SYMBOL:
           snprintf(retval,strlen(expr->s_val)+1,"%s",expr->s_val);
         break;
         case BL_VAL_TYPE_STRING:
           snprintf(retval,strlen(expr->s_val)+3,"\"%s\"",expr->s_val);
         break;
         case BL_VAL_TYPE_NUMBER:
           snprintf(retval,10,"%llu",expr->i_val);
         break;
	 case BL_VAL_TYPE_BOOL:
           if(expr->i_val == 1) {
              snprintf(retval,10,"True");
	   } else {
              snprintf(retval,10,"False");
	   }
	 break;
         case BL_VAL_TYPE_FUNC_BL:
	   snprintf(retval,1024,"(fn %s %s)",bl_ser_sexp(expr->bl_funcargs_ptr), bl_ser_sexp(expr->bl_func_ptr));
	 break;
         case BL_VAL_TYPE_OPER_NATIVE:
	   snprintf(retval,5,"OPER");
	 break;
	 case BL_VAL_TYPE_CONS:
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

