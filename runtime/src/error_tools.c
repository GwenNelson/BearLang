#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/error_tools.h>

#include <bearlang/sexp.h>
#include <bearlang/utils.h>

bl_val_t* bl_errif_invalid_len(bl_val_t* L, uint64_t min, uint64_t max) {  // LCOV_EXCL_LINE
     if(L->type == BL_VAL_TYPE_ERROR) return L; // LCOV_EXCL_LINE
   	uint64_t L_len = bl_list_len(L);

     if(L_len == min && (min == max)) return NULL;
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

bl_val_t* bl_errif_invalid_fixed_args(bl_val_t* params, const bl_val_type_t* expected_types, uint64_t args_len) {  // LCOV_EXCL_LINE
      bl_val_t* retval = bl_errif_invalid_len(params, args_len, args_len);
      if(retval != NULL) return retval;

      uint64_t i          = 0;
      bl_val_t* list_iter = params;
      bool is_good        = true;
      for(i=0; i < args_len; i++) {
          if(list_iter->car->type == BL_VAL_TYPE_ERROR) return list_iter->car; // LCOV_EXCL_LINE
          if(expected_types[i] != list_iter->car->type) {
            if(expected_types[i] != BL_VAL_TYPE_ANY) { // LCOV_EXCL_START
	       is_good=false;
	    } // LCOV_EXCL_STOP
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


bl_val_t* bl_err_symnotfound(char* sym) { // LCOV_EXCL_LINE
      bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_ERROR);

      retval->err_val.type = BL_ERR_SYMBOL_NOTFOUND;
      retval->err_val.symbol_name = (char*)GC_MALLOC_ATOMIC(sizeof(char)*(strlen(sym)+1));

      snprintf(retval->err_val.symbol_name,strlen(sym)+1,"%s",sym);
      return retval;
}

bl_val_t* bl_err_modnotfound(char* mod) { // LCOV_EXCL_LINE
      bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_ERROR);

      retval->err_val.type = BL_ERR_MODULE_NOTFOUND;
      retval->err_val.symbol_name = (char*)GC_MALLOC_ATOMIC(sizeof(char)*(strlen(mod)+1));

      snprintf(retval->err_val.symbol_name,strlen(mod)+1,"%s",mod);
      return retval;
}

bl_val_t* bl_err_divzero() { // LCOV_EXCL_LINE
      bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_ERROR);
      retval->err_val.type = BL_ERR_DIVIDE_BY_ZERO;
      return retval;
}

bl_val_t* bl_mk_err(bl_err_type_t T) { // LCOV_EXCL_LINE
      bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_ERROR);
      retval->err_val.type = T;
      return retval;
}

// LCOV_EXCL_START
bl_val_t* bl_errif_invalid_firstarg(bl_val_t* params, bl_val_type_t expected_type) { // LCOV_EXCL_LINE
      bl_val_t* retval = bl_errif_invalid_len(params,1,BL_LONGEST_LIST);
      if(retval != NULL) return retval;

      bl_val_t* first = bl_list_first(params);
      if(first->type == expected_type) return NULL;
      
      uint64_t params_len = bl_list_len(params);

      retval = bl_mk_val(BL_VAL_TYPE_ERROR);
      retval->err_val.type = BL_ERR_INVALID_ARGTYPE;
      retval->err_val.min_args = 1;
      retval->err_val.max_args = BL_LONGEST_LIST;
      retval->err_val.provided_args = params_len;

      retval->err_val.expected_types = (bl_val_type_t*)GC_MALLOC_ATOMIC(sizeof(bl_val_type_t)*params_len);
      retval->err_val.provided_types = (bl_val_type_t*)GC_MALLOC_ATOMIC(sizeof(bl_val_type_t)*params_len);
      int i=1;
      bl_val_t* list_iter = params->cdr;
      for(i=1; i<params_len; i++) {
          retval->err_val.expected_types[i] = BL_VAL_TYPE_ANY;
	  retval->err_val.provided_types[i] = list_iter->car->type;
      }
      retval->err_val.expected_types[0] = expected_type;
      retval->err_val.provided_types[0] = first->type;
      return retval;

}
// LCOV_EXCL_STOP

// LCOV_EXCL_START

char* bl_ser_type(bl_val_type_t t) {
      switch(t) {
         case BL_VAL_TYPE_NULL:
	      return "NULL";
	 break;
         case BL_VAL_TYPE_ERROR:
	      return "ERROR";
	 break;
	 case BL_VAL_TYPE_SYMBOL:
	      return "SYMBOL";
	 break;
	 case BL_VAL_TYPE_NUMBER:
	      return "NUMBER";
	 break;
	 case BL_VAL_TYPE_BOOL:
              return "BOOL";
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
	 case BL_VAL_TYPE_CPTR:
	      return "CPTR";
	 break;
	 case BL_VAL_TYPE_ANY:
	      return "ANY";
	 break;
	 default:
	      return "UNKNOWN";
	 break;
      }
}

char* bl_ser_types(uint64_t count, bl_val_type_t* types) {
      int i=0;
      size_t maxlen = sizeof(char)*32*count;
      char* retbuf = (char*)GC_MALLOC_ATOMIC(maxlen);
      retbuf[0] = NULL;
      for(i=0; i<count; i++) {
          char* s = bl_ser_type(types[i]);
          snprintf(retbuf + strlen(retbuf),maxlen,"%s,",s);
      }
      char* retval = (char*)GC_MALLOC_ATOMIC(maxlen+4);
      snprintf(retval,maxlen+4,"(%s", retbuf);
      retval[strlen(retval)-1]=')';
      return retval;
}

char* bl_errmsg(bl_val_t* E) {
      // TODO - nested errors - return an error if E is not an error ;)

      char* retval = (char*)GC_MALLOC_ATOMIC(sizeof(char)*1024);
      switch(E->err_val.type) {
          case BL_ERR_UNKNOWN:
   		  snprintf(retval,1023,"Unknown error!");
	  break;
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
	  case BL_ERR_SYMBOL_NOTFOUND:
	       snprintf(retval,1023,"Symbol %s not found in current environment or any parent environment", E->err_val.symbol_name);
	  break;
	  case BL_ERR_DIVIDE_BY_ZERO:
	       snprintf(retval,1023,"Divide by zero!");
	  break;
	  case BL_ERR_CUSTOM:
	       snprintf(retval,1023,"Error code %llu, %s",E->err_val.errnum, E->err_val.errmsg);
	  break;
	  case BL_ERR_IO:
	       snprintf(retval,1023,"General I/O error");
	  break;
	  case BL_ERR_MODULE_NOTFOUND:
	       snprintf(retval,1023,"Module %s could not be found", E->err_val.symbol_name);
	  break;
	  case BL_ERR_BL_CUSTOM:
	       snprintf(retval,1023,"%s: %s", E->err_val.err_sym->s_val, bl_ser_sexp(E->err_val.err_val));
	  break;
      }
      return retval;
}

// LCOV_EXCL_STOP
