#include <stdint.h>
#include <stdbool.h>

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
      }
      return retval;
}
