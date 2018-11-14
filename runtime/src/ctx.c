#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/builtins.h>
#include <bearlang/list_ops.h>
#include <bearlang/utils.h>
#include <bearlang/error_tools.h>
#include <bl_build_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define BUILTIN_STRUCT(builtin_name,builtin_symbol,builtin_docstr) bl_val_t native_oper_docstr_ ## builtin_name = { .s_val = builtin_docstr }; \
				                    bl_val_t native_oper_ ## builtin_name = { .type = BL_VAL_TYPE_OPER_NATIVE, \
											      .code_ptr = &bl_oper_ ## builtin_name, \
											      .docstr   = &native_oper_docstr_ ## builtin_name};

#define ADD_BUILTIN(builtin_name,builtin_symbol,builtin_docstr) bl_ctx_set(retval,bl_mk_symbol(builtin_symbol), &native_oper_ ## builtin_name);

#define ADD_ERR(err_name,err_val) bl_ctx_set(retval, bl_mk_symbol("ERR_" #err_name ), bl_mk_err(BL_ERR_ ## err_name));

#define BUILTIN_X BUILTIN_STRUCT
#include <bearlang/builtins.inc>
#undef BUILTIN_X


#define ADD_TYPE(type_name) bl_ctx_set(retval, bl_mk_symbol("TYPE_" #type_name ), bl_mk_type(BL_VAL_TYPE_ ## type_name));

bl_val_t* bl_ctx_new_std() { // LCOV_EXCL_LINE

   bl_val_t* retval = bl_ctx_new(NULL);
   char* path_env = getenv("BEARLANGPATH");

   if(path_env != NULL) { // LCOV_EXCL_BR_LINE
      path_env = strdup(getenv("BEARLANGPATH")); //LCOV_EXCL_LINE
      bl_val_t* path_val = NULL; // LCOV_EXCL_LINE
      char* path_dir;
      char* rest = path_env; // LCOV_EXCL_LINE
      while((path_dir = strtok_r(rest, ":", &rest))) { // LCOV_EXCL_LINE
         path_val = bl_list_append(path_val,bl_mk_str(path_dir)); // LCOV_EXCL_LINE
      }
      bl_ctx_set(retval,bl_mk_symbol("*PATH*"), path_val);
   } else { // LCOV_EXCL_LINE
      bl_ctx_set(retval,bl_mk_symbol("*PATH*"), bl_mk_list(4,bl_mk_str("."),bl_mk_str(BL_STDLIB_PATH1),bl_mk_str(BL_STDLIB_PATH2),bl_mk_str(BL_STDLIB_PATH3)));
   }
   free(path_env);

   bl_ctx_set(retval,bl_mk_symbol("*VERSION*"), bl_mk_str(BL_VERSION)); // TODO - change this and use a real versioning system
   bl_ctx_set(retval,   bl_mk_symbol( "None"), bl_mk_null());
#define BUILTIN_X ADD_BUILTIN
#include <bearlang/builtins.inc>
#undef BUILTIN_X

   // most builtins are defined in the include file above, these are defined here as aliases
   bl_ctx_set(retval,     bl_mk_symbol("car"), &native_oper_first);
   bl_ctx_set(retval,     bl_mk_symbol("cdr"), &native_oper_rest);
   bl_ctx_set(retval,     bl_mk_symbol("=="),  &native_oper_eq);

   bl_ctx_set(retval, bl_mk_symbol("True"),  bl_mk_bool(true));
   bl_ctx_set(retval, bl_mk_symbol("False"), bl_mk_bool(false));

#define TYPE_X ADD_TYPE
#include <bearlang/data_types.inc>
#undef TYPE_X


#define ERR_X ADD_ERR
#include <bearlang/err_types.inc>
#undef ERR_X

   return retval;
}

bl_val_t* bl_ctx_new(bl_val_t* parent) { // LCOV_EXCL_LINE

   bl_val_t* retval   = bl_mk_val(BL_VAL_TYPE_CTX);
   retval->parent     = parent;
   retval->secondary  = NULL;
   if(parent==NULL) {
	   retval->vals_count = 8;
   } else {
	   retval->vals_count = parent->vals_count;
   }
   retval->vals       = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*retval->vals_count);
   retval->keys       = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*retval->vals_count);
   retval->write_to_parent = false;

   return retval;
}

void bl_ctx_close(bl_val_t* ctx) { // LCOV_EXCL_LINE

}



bl_val_t* bl_ctx_get(bl_val_t* ctx, bl_val_t* key) { // LCOV_EXCL_LINE
   if(key == NULL) return bl_mk_err(BL_ERR_INTERNAL); // LCOV_EXCL_BR_LINE
   if(ctx->ctx_get != NULL) return ctx->ctx_get(ctx,key); // LCOV_EXCL_BR_LINE
   if(key->type != BL_VAL_TYPE_SYMBOL) return bl_mk_err(BL_ERR_INTERNAL); // LCOV_EXCL_BR_LINE
   if(key->s_val[0]=='\'') return bl_mk_symbol(key->s_val+1); // LCOV_EXCL_BR_LINE
   if(strlen(key->s_val)>=1) {
     if(strstr(key->s_val,"::")) {
        char* tmp = strdup(key->s_val);
        strstr(tmp,"::")[0]='\0';
        char* ctx_key = tmp;
        char* sym_key = tmp+strlen(ctx_key)+2;
        bl_val_t* other_ctx = bl_ctx_get(ctx, bl_mk_symbol(ctx_key));
        free(tmp);
        // LCOV_EXCL_START
        if(other_ctx == NULL) { 
           return NULL;
        }
        // LCOV_EXCL_STOP 
        return bl_ctx_get(other_ctx,bl_mk_symbol(sym_key));

      }
   }
   bl_val_t* retval = NULL;
   if(key->sym_id < ctx->vals_count) {
    	   retval = ctx->vals[key->sym_id];

   }
   if(retval != NULL) {
      return retval;
   } else {
      if(ctx->parent != NULL) return bl_ctx_get(ctx->parent, key);
   }
   return NULL;

}

bl_val_t* bl_ctx_set(bl_val_t* ctx, bl_val_t* key, bl_val_t* val) { // LCOV_EXCL_LINE

   if(ctx->write_to_parent) {
         ctx = ctx->parent;
   }
   if(key->sym_id >= ctx->vals_count) {
     uint64_t old_count = ctx->vals_count;
     ctx->vals_count = key->sym_id+8;
     bl_val_t** old_vals = ctx->vals;
     bl_val_t** old_keys = ctx->keys;
     ctx->vals = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*(ctx->vals_count));
     ctx->keys = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*(ctx->vals_count));
     int i=0;
     
     for(i=0; i<old_count; i++) {
         ctx->vals[i] = old_vals[i];
         ctx->keys[i] = old_keys[i];
     }

   }
   ctx->keys[key->sym_id] = key;
   ctx->vals[key->sym_id] = val;
   return val;


}
