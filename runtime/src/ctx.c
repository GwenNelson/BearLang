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

bl_val_t* bl_ctx_new_std() {
   bl_val_t* retval = bl_ctx_new(NULL);

   bl_ctx_set(retval,bl_mk_symbol("*VERSION*"), bl_mk_str("0.WHATEVER")); // TODO - change this and use a real versioning system

   bl_ctx_set(retval,   bl_mk_symbol( "None"), bl_mk_null());
   bl_ctx_set(retval,       bl_mk_symbol("+"), bl_mk_native_oper(&bl_oper_add));
   bl_ctx_set(retval,       bl_mk_symbol("-"), bl_mk_native_oper(&bl_oper_sub));
   bl_ctx_set(retval,       bl_mk_symbol("*"), bl_mk_native_oper(&bl_oper_mult));
   bl_ctx_set(retval,       bl_mk_symbol("/"), bl_mk_native_oper(&bl_oper_div));
   bl_ctx_set(retval,       bl_mk_symbol("%"), bl_mk_native_oper(&bl_oper_mod));
   bl_ctx_set(retval,       bl_mk_symbol("="), bl_mk_native_oper(&bl_oper_set));
   bl_ctx_set(retval,      bl_mk_symbol("fn"), bl_mk_native_oper(&bl_oper_fn));
   bl_ctx_set(retval,     bl_mk_symbol("fun"), bl_mk_native_oper(&bl_oper_fun));
   bl_ctx_set(retval,     bl_mk_symbol("map"), bl_mk_native_oper(&bl_oper_map));
   bl_ctx_set(retval,    bl_mk_symbol("oper"), bl_mk_native_oper(&bl_oper_oper));
   bl_ctx_set(retval,      bl_mk_symbol("eq"), bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,      bl_mk_symbol("lt"), bl_mk_native_oper(&bl_oper_lt));
   bl_ctx_set(retval,      bl_mk_symbol("gt"), bl_mk_native_oper(&bl_oper_gt));
   bl_ctx_set(retval,      bl_mk_symbol("=="), bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,   bl_mk_symbol("print"), bl_mk_native_oper(&bl_oper_print));
   bl_ctx_set(retval,     bl_mk_symbol("and"), bl_mk_native_oper(&bl_oper_and));
   bl_ctx_set(retval,     bl_mk_symbol("not"), bl_mk_native_oper(&bl_oper_not));
   bl_ctx_set(retval,      bl_mk_symbol("or"), bl_mk_native_oper(&bl_oper_or));
   bl_ctx_set(retval,     bl_mk_symbol("xor"), bl_mk_native_oper(&bl_oper_xor));
   bl_ctx_set(retval,   bl_mk_symbol("first"), bl_mk_native_oper(&bl_oper_first));
   bl_ctx_set(retval,  bl_mk_symbol("second"), bl_mk_native_oper(&bl_oper_second));
   bl_ctx_set(retval,   bl_mk_symbol("third"), bl_mk_native_oper(&bl_oper_third));
   bl_ctx_set(retval,    bl_mk_symbol("rest"), bl_mk_native_oper(&bl_oper_rest));
   bl_ctx_set(retval,  bl_mk_symbol("append"), bl_mk_native_oper(&bl_oper_append));
   bl_ctx_set(retval, bl_mk_symbol("prepend"), bl_mk_native_oper(&bl_oper_prepend));
   bl_ctx_set(retval, bl_mk_symbol("reverse"), bl_mk_native_oper(&bl_oper_reverse));
   bl_ctx_set(retval,     bl_mk_symbol("car"), bl_mk_native_oper(&bl_oper_first));
   bl_ctx_set(retval,     bl_mk_symbol("cdr"), bl_mk_native_oper(&bl_oper_rest));
   bl_ctx_set(retval, bl_mk_symbol("include"), bl_mk_native_oper(&bl_oper_include));
   bl_ctx_set(retval,  bl_mk_symbol("import"), bl_mk_native_oper(&bl_oper_import));
   bl_ctx_set(retval,   bl_mk_symbol("isset"), bl_mk_native_oper(&bl_oper_isset));
   bl_ctx_set(retval,  bl_mk_symbol("serexp"), bl_mk_native_oper(&bl_oper_serexp));
   bl_ctx_set(retval,     bl_mk_symbol("inc"), bl_mk_native_oper(&bl_oper_inc));
   bl_ctx_set(retval,     bl_mk_symbol("dec"), bl_mk_native_oper(&bl_oper_dec));


   bl_ctx_set(retval, bl_mk_symbol("True"),  bl_mk_bool(true));
   bl_ctx_set(retval, bl_mk_symbol("False"), bl_mk_bool(false));

   return retval;
}

bl_val_t* bl_ctx_new(bl_val_t* parent) {
   bl_val_t* retval   = bl_mk_val(BL_VAL_TYPE_CTX);
   retval->parent     = parent;
   retval->secondary  = NULL;
   retval->hash_val   = NULL;
   retval->vals_count = 64;
   retval->vals       = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*retval->vals_count);
   retval->write_to_parent = false;
   return retval;
}

void bl_ctx_close(bl_val_t* ctx) {
}

// binds variables into a context by copying them
void bl_set_params(bl_val_t* ctx, bl_val_t* param_names, bl_val_t* param_vals) {
    bl_val_t* argsk_i = param_names;
    bl_val_t* argsv_i = param_vals;
    
    for(argsk_i=param_names; argsk_i != NULL; argsk_i=argsk_i->cdr) {
        bl_ctx_set(ctx,argsk_i->car, argsv_i->car);
	argsv_i = argsv_i->cdr;
	if(argsv_i == NULL) return;
    }
}

bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr, bool build_new_list) {
    bl_val_t* retval = NULL;
    bl_val_t* L_start = NULL;
    bl_val_t* L      = NULL;
    bl_val_t* i = NULL;
    if(build_new_list) {
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
	    return L_start;
    } else {
	    for(i=expr; i != NULL; i=i->cdr) {
	        retval = bl_ctx_eval(ctx,i->car);
	        if(retval->type == BL_VAL_TYPE_ERROR) return retval;

	    }
    }
    return NULL;
}

bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr) {
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
		   if(expr->car==NULL) {
                      return bl_mk_null();
		   } else {
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
						    if(retval->type == BL_VAL_TYPE_ERROR) return retval;
						}
					}
					return bl_mk_null();
				break;
				case BL_VAL_TYPE_FUNC_BL:
					args        = bl_ctx_eval(ctx,expr->cdr);
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
				

					retval = bl_eval_cons(ctx, expr, true);
					if(retval==NULL) return bl_mk_null();
					return retval;
				break;
			   }
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


bl_val_t* bl_ctx_get(bl_val_t* ctx, bl_val_t* key) {

/*   if(strstr(key->s_val,"::")) {
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

   }*/
   if(ctx->secondary != NULL) {
      bl_val_t* s_V = bl_ctx_get(ctx->secondary, key);
      if(s_V != NULL) return s_V;
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

bl_val_t* bl_ctx_set(bl_val_t* ctx, bl_val_t* key, bl_val_t* val) {
   if(ctx->write_to_parent) {
      if(ctx->parent != NULL) {
         if(ctx->write_to_parent) ctx = ctx->parent;
      }
   }
   if(key->sym_id >= ctx->vals_count) {
     uint16_t old_count = ctx->vals_count;
     ctx->vals_count = key->sym_id+8;
     bl_val_t** old_vals = ctx->vals;
     ctx->vals = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*ctx->vals_count);
     int i=0;
     
     for(i=0; i<old_count; i++) ctx->vals[i] = old_vals[i];

   }
   ctx->vals[key->sym_id] = val;
   return val;


}
