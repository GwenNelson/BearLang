#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/error_tools.h>

bool bl_is_valid_len(bl_val_t* L, uint64_t min, uint64_t max) {
     uint64_t L_len = bl_list_len(L);
     if(L_len < min) return false;
     if(L_len > max) return false;
     return true;
}

bool bl_is_valid_funcarg_len(bl_val_t* f, bl_val_t* L) {
     uint64_t L_len = bl_list_len(L);
     uint64_t funcargs_len = bl_list_len(f->bl_funcargs_ptr);
     if(L_len == funcargs_len) return true;
     return false;
}

bl_val_t* bl_errif_invalid_len(bl_val_t* L, uint64_t min, uint64_t max) {
     uint64_t L_len = bl_list_len(L);
     if((L_len >= min) && (L_len <= max)) return NULL;

     bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
     retval->type     = BL_VAL_TYPE_ERROR;

     retval->err_val.min_args      = min;
     retval->err_val.max_args      = max;
     retval->err_val.provided_args = L_len;

     if(L_len < min) {
	retval->err_val.type = BL_ERR_INSUFFICIENT_ARGS;
     } else {
        retval->err_val.type = BL_ERR_TOOMANY_ARGS;
     }
     return retval;
}

bl_val_t* bl_errif_invalid_fixed_args(bl_val_t* params, const bl_val_type_t* expected_types, uint64_t args_len) {
      bl_val_t* retval = bl_errif_invalid_len(params, args_len, args_len);
      if(retval != NULL) return retval;

      uint64_t i          = 0;
      bl_val_t* list_iter = params;
      bool is_good        = true;
      for(i=0; i < args_len; i++) {
          is_good=false;
          if(expected_types[i] != list_iter->car->type) {
            if(expected_types[i] != BL_VAL_TYPE_ANY) { 
	       is_good=false;
	    }
	  } 
          list_iter = list_iter->cdr;
      }

      if(is_good) return NULL;

      retval           = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
      retval->type     = BL_VAL_TYPE_ERROR;

      retval->err_val.type           = BL_ERR_INVALID_ARGTYPE;
      retval->err_val.min_args       = args_len;
      retval->err_val.max_args       = args_len;

      retval->err_val.expected_types = (bl_val_type_t*)GC_MALLOC(sizeof(bl_val_type_t*)*args_len);
      retval->err_val.provided_args  = args_len; // if this isn't true, the above bl_errif_invalid_len will have returned already
      retval->err_val.provided_types = (bl_val_type_t*)GC_MALLOC(sizeof(bl_val_type_t)*args_len);
      
      list_iter = params;
      for(i=0; i < args_len; i++) {
          retval->err_val.expected_types[i] = expected_types[i];
          retval->err_val.provided_types[i] = list_iter->car->type;
	  list_iter = list_iter->cdr;
      }

      return retval;
}

char* bl_ser_type(bl_val_type_t t) {
      switch(t) {
         case BL_VAL_TYPE_NULL:
	      return "NULL";
	 break;
         case BL_VAL_TYPE_ERROR:
	      return "ERROR";
	 break;
	 case BL_VAL_TYPE_AST_LIST:
	      return "AST_LIST";
	 break;
	 case BL_VAL_TYPE_SYMBOL:
	      return "SYMBOL";
	 break;
	 case BL_VAL_TYPE_NUMBER:
	      return "NUMBER";
	 break;
	 case BL_VAL_TYPE_STRING:
	      return "STRING";
	 break;
	 case BL_VAL_TYPE_CONS:
	      return "CONS";
	 break;
	 case BL_VAL_TYPE_OPER_NATIVE:
	      return "OPER";
	 break;
	 case BL_VAL_TYPE_FUNC_BL:
	      return "FUNCTION";
	 break;
	 case BL_VAL_TYPE_FUNC_NATIVE:
	      return "FUNCTION";
	 break;
	 case BL_VAL_TYPE_CTX:
	      return "CONTEXT";
	 break;
	 case BL_VAL_TYPE_ANY:
	      return "ANY";
	 break;
      }
}

char* bl_ser_types(uint64_t count, bl_val_type_t* types) {
      int i=0;
      size_t maxlen = sizeof(char)*32*count;
      char* retbuf = (char*)GC_MALLOC(maxlen);
      for(i=0; i<count; i++) {
          char* s = bl_ser_type(types[i]);
          snprintf(retbuf + strlen(retbuf),maxlen,"%s,",s);
      }
      char* retval = (char*)GC_MALLOC(maxlen+4);
      snprintf(retval,maxlen+4,"(%s", retbuf);
      retval[strlen(retval)-1]=')';
      return retval;
}

char* bl_errmsg(bl_val_t* E) {
      // TODO - nested errors - return an error if E is not an error ;)

      char* retval = (char*)GC_MALLOC(sizeof(char)*1024);
      switch(E->err_val.type) {
          case BL_ERR_PARSE:
               snprintf(retval,1023,"Could not parse: %s", "TODO: implement this");
	  break;
	  case BL_ERR_INSUFFICIENT_ARGS:
               snprintf(retval,1023,"Insufficient arguments, expected at least %llu, received %llu", E->err_val.min_args, E->err_val.provided_args);
	  break;
	  case BL_ERR_TOOMANY_ARGS:
               snprintf(retval,1023,"Too many arguments, expected no more than %llu, received %llu", E->err_val.max_args, E->err_val.provided_args);
	  break;
	  case BL_ERR_INVALID_ARGTYPE:
               snprintf(retval,1023,"Invalid arguments, expected %s but got %s", bl_ser_types(E->err_val.provided_args,E->err_val.expected_types),
			                                                      bl_ser_types(E->err_val.provided_args,E->err_val.provided_types));
	  break;
      }
      return retval;
}
