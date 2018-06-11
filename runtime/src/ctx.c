#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/builtins.h>
#include <bearlang/list_ops.h>
#include <bearlang/utils.h>
#include <bearlang/error_tools.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define BUILTIN_STRUCT(builtin_name,builtin_symbol,builtin_docstr) bl_val_t native_oper_docstr_ ## builtin_name = { .s_val = builtin_docstr }; \
				                    bl_val_t native_oper_ ## builtin_name = { .type = BL_VAL_TYPE_OPER_NATIVE, \
											     .code_ptr = &bl_oper_ ## builtin_name, \
											     .docstr   = &native_oper_docstr_ ## builtin_name};

#define ADD_BUILTIN(builtin_name,builtin_symbol,builtin_docstr) bl_ctx_set(retval,bl_mk_symbol(builtin_symbol), &native_oper_ ## builtin_name);

#define BUILTIN_X BUILTIN_STRUCT
#include "builtins.inc"
#undef BUILTIN_X


bl_val_t* bl_ctx_new_std() { // LCOV_EXCL_LINE

   bl_val_t* retval = bl_ctx_new(NULL);

   char* path_env = getenv("BEARLANGPATH");
   if(path_env != NULL) {
      bl_val_t* path_val = NULL;
      char* path_dir;
      char* rest = path_env;
      while((path_dir = strtok_r(rest, ":", &rest))) {
         path_val = bl_list_append(path_val,bl_mk_str(path_dir));
      }
      bl_ctx_set(retval,bl_mk_symbol("*PATH*"), path_val);
   } else {
      bl_ctx_set(retval,bl_mk_symbol("*PATH*"), bl_mk_list(2,bl_mk_str("."),bl_mk_str("./stdlib")));
   }

   bl_ctx_set(retval,bl_mk_symbol("*VERSION*"), bl_mk_str("0.WHATEVER")); // TODO - change this and use a real versioning system
   bl_ctx_set(retval,   bl_mk_symbol( "None"), bl_mk_null());
#define BUILTIN_X ADD_BUILTIN
#include "builtins.inc"
#undef BUILTIN_X

   // most builtins are defined in the include file above, these are defined here as aliases
   bl_ctx_set(retval,     bl_mk_symbol("car"), &native_oper_first);
   bl_ctx_set(retval,     bl_mk_symbol("cdr"), &native_oper_rest);
   bl_ctx_set(retval,     bl_mk_symbol("=="),  &native_oper_eq);

   bl_ctx_set(retval, bl_mk_symbol("True"),  bl_mk_bool(true));
   bl_ctx_set(retval, bl_mk_symbol("False"), bl_mk_bool(false));

   bl_ctx_set(retval, bl_mk_symbol("TYPE_SYMBOL"), bl_mk_type(BL_VAL_TYPE_SYMBOL));
   bl_ctx_set(retval, bl_mk_symbol("TYPE_CTX"),    bl_mk_type(BL_VAL_TYPE_CTX));

   bl_ctx_set(retval, bl_mk_symbol("ERR_ANY"),              bl_mk_err(BL_ERR_ANY));
   bl_ctx_set(retval, bl_mk_symbol("ERR_UNKNOWN"),          bl_mk_err(BL_ERR_UNKNOWN));
   bl_ctx_set(retval, bl_mk_symbol("ERR_PARSE"),            bl_mk_err(BL_ERR_PARSE));
   bl_ctx_set(retval, bl_mk_symbol("ERR_INSUFFICIENT_ARGS"),bl_mk_err(BL_ERR_INSUFFICIENT_ARGS));
   bl_ctx_set(retval, bl_mk_symbol("ERR_TOOMANY_ARGS"),     bl_mk_err(BL_ERR_TOOMANY_ARGS));
   bl_ctx_set(retval, bl_mk_symbol("ERR_INVALID_ARGTYPE"),  bl_mk_err(BL_ERR_INVALID_ARGTYPE));
   bl_ctx_set(retval, bl_mk_symbol("ERR_SYMBOL_NOTFOUND"),  bl_mk_err(BL_ERR_SYMBOL_NOTFOUND));
   bl_ctx_set(retval, bl_mk_symbol("ERR_DIVIDE_BY_ZERO"),   bl_mk_err(BL_ERR_DIVIDE_BY_ZERO));
   bl_ctx_set(retval, bl_mk_symbol("ERR_CUSTOM"),           bl_mk_err(BL_ERR_CUSTOM));
   bl_ctx_set(retval, bl_mk_symbol("ERR_IO"),               bl_mk_err(BL_ERR_IO));
   bl_ctx_set(retval, bl_mk_symbol("ERR_MODULE_NOTFOUND"),  bl_mk_err(BL_ERR_MODULE_NOTFOUND));

   return retval;
}

bl_val_t* bl_ctx_new(bl_val_t* parent) { // LCOV_EXCL_LINE

   bl_val_t* retval   = bl_mk_val(BL_VAL_TYPE_CTX);
   retval->parent     = parent;
   retval->secondary  = NULL;
   retval->vals_count = 8;
   retval->vals       = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*retval->vals_count);
   retval->keys       = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*retval->vals_count);
   retval->write_to_parent = false;
   return retval;
}

void bl_ctx_close(bl_val_t* ctx) { // LCOV_EXCL_LINE

}

// binds variables into a context by copying them
void bl_set_params(bl_val_t* ctx, bl_val_t* param_names, bl_val_t* param_vals) { // LCOV_EXCL_LINE

    bl_val_t* argsk_i = param_names;
    bl_val_t* argsv_i = param_vals;

//TODO - add a test for null params and then remove the exclusion below
    for(argsk_i=param_names; argsk_i != NULL; argsk_i=argsk_i->cdr) { // LCOV_EXCL_LINE

        bl_ctx_set(ctx,argsk_i->car, argsv_i->car);
	argsv_i = argsv_i->cdr;
	if(argsv_i == NULL) return;
    }
}

bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr) { // LCOV_EXCL_LINE
    if(expr==NULL) return bl_mk_null();
    bl_val_t* retval = NULL;
    bl_val_t* L_start = NULL;
    bl_val_t* L      = NULL;
    bl_val_t* i = NULL;
	    for(i=expr; i != NULL; i=i->cdr) {
	        retval = bl_ctx_eval(ctx,i->car);
	        if(retval->type == BL_VAL_TYPE_ERROR) return retval;
		if(L==NULL) {
		   L=bl_mk_val(BL_VAL_TYPE_CONS);
		   L->car = retval;
		   L->cdr = NULL;
		   L_start = L;
		} else {
		   L->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
		   L->cdr->car = retval;
		   L=L->cdr;
		}
	    }
            if(L_start==NULL) return bl_mk_null();
	    return L_start;
    return NULL;
}

bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr) { // LCOV_EXCL_LINE
    if(expr==NULL) return bl_mk_null();
    bl_val_t* retval = NULL;
    bool in_func = false;
    bool in_oper = false;
    while(true) {
	    if(expr == NULL) return bl_mk_null();
	    if(expr->type == BL_VAL_TYPE_ERROR) return expr;
	    bl_val_t* symval  = NULL;
	    bl_val_t* car     = NULL;
	    bl_val_t* retval  = NULL;
	    bl_val_t* new_closure = NULL;
	    bl_val_t* cond = NULL;
	    bl_val_t* args = NULL;
	    bl_val_t* i    = NULL;
	    switch(expr->type) {
	      case BL_VAL_TYPE_CONS:
                          if(expr->car == NULL) return bl_mk_null();
                          if(expr->car->type == BL_VAL_TYPE_SYMBOL) {
	 			car = bl_ctx_get(ctx, expr->car);
				if(car == NULL)  return bl_err_symnotfound(expr->car->s_val);
			   } else {
				car = expr->car;
			   }
			   switch(car->type) {
				case BL_VAL_TYPE_ERROR:
					return car;
				break;
   				case BL_VAL_TYPE_OPER_NATIVE:
				     if(expr->cdr != NULL) {
            				expr->cdr->invoked_sym = expr->car; // let the operator know what symbol was used to invoke it
					expr->cdr->custom_data = car->custom_data; // pass any custom data
				     }
			             retval = car->code_ptr(ctx, expr->cdr);
				     return retval;
				break;
				case BL_VAL_TYPE_OPER_DO:
					expr = expr->cdr;
					for(i=expr; i != NULL; i=i->cdr) {
						retval = bl_ctx_eval(ctx,i->car);
					}
					return retval;
				break;
				case BL_VAL_TYPE_OPER_IF:
					cond = bl_ctx_eval(ctx,bl_list_first(expr->cdr));
					if(cond->b_val) {
						expr = bl_list_second(expr->cdr);
					} else {
						expr = bl_list_third(expr->cdr);
					}
				break;
				case BL_VAL_TYPE_OPER_WHILE:
					cond = bl_list_first(expr->cdr);
					while(bl_ctx_eval(ctx, cond)->b_val==true) {
						for(i=bl_list_rest(expr->cdr); i != NULL; i=i->cdr) {
    							retval = bl_ctx_eval(ctx,i->car);
							if(retval != NULL) {
	    							if(retval->type == BL_VAL_TYPE_ERROR) return retval;
							}
						}
					}
					return bl_mk_null();
				break;
				case BL_VAL_TYPE_FUNC_BL:
					args        = bl_ctx_eval(ctx,expr->cdr);
					car->inner_closure = bl_ctx_new(car->lexical_closure);
					if(bl_list_len(expr) > 1) bl_set_params(car->inner_closure,car->bl_funcargs_ptr,args);
					ctx = car->inner_closure;
					expr = car->bl_func_ptr;
					for(i=expr; i != NULL; i=i->cdr) {
						retval = bl_ctx_eval(ctx,i->car);
					}
					return retval;
				break;
				case BL_VAL_TYPE_OPER_BL:
					ctx = bl_ctx_new(ctx);
					ctx->write_to_parent = true;
					expr = car->bl_oper_ptr;
					in_oper = true;
				break;

				default:
				

					retval = bl_eval_cons(ctx, expr);
					return retval;
				break;
			   }
              break;
	      case BL_VAL_TYPE_SYMBOL:
	           symval = bl_ctx_get(ctx, expr);
	           if(symval == NULL) {
	              return bl_err_symnotfound(expr->s_val);
		   } else {
	              return symval;
		   }
	      break;
	      default:
		   return expr;
	      break;
	   }
    }
}


bl_val_t* bl_ctx_get(bl_val_t* ctx, bl_val_t* key) { // LCOV_EXCL_LINE
   if(ctx->ctx_get != NULL) return ctx->ctx_get(ctx,key);
   if(key->s_val[0]=='\'') return bl_mk_symbol(key->s_val+1);
// LCOV_EXCL_START
   if(strstr(key->s_val,"::")) {
     char* tmp = strdup(key->s_val);
     strstr(tmp,"::")[0]='\0';
     char* ctx_key = tmp;
     char* sym_key = tmp+strlen(ctx_key)+2;
     bl_val_t* other_ctx = bl_ctx_get(ctx, bl_mk_symbol(ctx_key));
     free(tmp);
     if(other_ctx == NULL) { 
   	return NULL;
     }
   
     return bl_ctx_get(other_ctx,bl_mk_symbol(sym_key));

   }
// LCOV_EXCL_STOP
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
