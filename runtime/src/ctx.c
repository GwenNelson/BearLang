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

   bl_ctx_set(retval,"*VERSION*", bl_mk_str("0.WHATEVER")); // TODO - change this and use a real versioning system

   bl_ctx_set(retval,       "+", bl_mk_native_oper(&bl_oper_add));
   bl_ctx_set(retval,       "-", bl_mk_native_oper(&bl_oper_sub));
   bl_ctx_set(retval,       "*", bl_mk_native_oper(&bl_oper_mult));
   bl_ctx_set(retval,       "/", bl_mk_native_oper(&bl_oper_div));
   bl_ctx_set(retval,       "=", bl_mk_native_oper(&bl_oper_set));
   bl_ctx_set(retval,      "fn", bl_mk_native_oper(&bl_oper_fn));
   bl_ctx_set(retval,     "fun", bl_mk_native_oper(&bl_oper_fun));
   bl_ctx_set(retval,    "oper", bl_mk_native_oper(&bl_oper_oper));
   bl_ctx_set(retval,      "eq", bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,      "==", bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,      "if", bl_mk_native_oper(&bl_oper_if));
   bl_ctx_set(retval,   "print", bl_mk_native_oper(&bl_oper_print));
   bl_ctx_set(retval,     "and", bl_mk_native_oper(&bl_oper_and));
   bl_ctx_set(retval,     "not", bl_mk_native_oper(&bl_oper_not));
   bl_ctx_set(retval,      "or", bl_mk_native_oper(&bl_oper_or));
   bl_ctx_set(retval,     "xor", bl_mk_native_oper(&bl_oper_xor));
   bl_ctx_set(retval,   "first", bl_mk_native_oper(&bl_oper_first));
   bl_ctx_set(retval,  "second", bl_mk_native_oper(&bl_oper_second));
   bl_ctx_set(retval,   "third", bl_mk_native_oper(&bl_oper_third));
   bl_ctx_set(retval,    "rest", bl_mk_native_oper(&bl_oper_rest));
   bl_ctx_set(retval,     "car", bl_mk_native_oper(&bl_oper_first));
   bl_ctx_set(retval,     "cdr", bl_mk_native_oper(&bl_oper_rest));
   bl_ctx_set(retval, "include", bl_mk_native_oper(&bl_oper_include));
   bl_ctx_set(retval,   "isset", bl_mk_native_oper(&bl_oper_isset));
   bl_ctx_set(retval,  "serexp", bl_mk_native_oper(&bl_oper_serexp));

   bl_ctx_set(retval, "True",  bl_mk_bool(true));
   bl_ctx_set(retval, "False", bl_mk_bool(false));

   bl_ctx_set(retval,      "do", bl_mk_val(BL_VAL_TYPE_OPER_DO));
   return retval;
}

bl_val_t* bl_ctx_new(bl_val_t* parent) {
   bl_val_t* retval  = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type      = BL_VAL_TYPE_CTX;
   retval->parent    = parent;
   retval->secondary = NULL;
   retval->hash_val  = NULL;
   retval->write_to_parent = false;
   return retval;
}

void bl_ctx_close(bl_val_t* ctx) {
    GC_FREE(ctx);
}

// binds variables into a context by copying them
void bl_set_params(bl_val_t* ctx, bl_val_t* param_names, bl_val_t* param_vals) {
    bl_val_t* argsk_i = param_names;
    bl_val_t* argsv_i = param_vals;
    while(argsk_i->cdr != NULL) {
       if(argsk_i->car != NULL) {
	       bl_ctx_set(ctx, argsk_i->car->s_val, bl_ctx_eval(ctx,argsv_i->car));
       }
       argsk_i = argsk_i -> cdr;
       argsv_i = argsv_i -> cdr;
    }
    if(argsk_i->car != NULL) {
       bl_ctx_set(ctx, argsk_i->car->s_val, bl_ctx_eval(ctx,argsv_i->car));
    }
}

bl_val_t* bl_eval_bloper(bl_val_t* ctx, bl_val_t* oper, bl_val_t* params) {
    bl_val_t* retval   = NULL;
    bl_val_t* closure  = bl_ctx_new(ctx);
    closure->write_to_parent = true;

    bl_val_t* argsk_i  = oper->bl_operargs_ptr;
    bl_val_t* argsv_i  = params;
    while(argsk_i->cdr != NULL) {
       if(argsk_i->car != NULL) {
     	    bl_ctx_set(closure, argsk_i->car->s_val, argsv_i->car);
       }
       argsk_i = argsk_i -> cdr;
       argsv_i = argsv_i -> cdr;
    }
    if(argsk_i->car != NULL) {
       bl_ctx_set(closure, argsk_i->car->s_val, argsv_i->car);
    }

    bl_val_t* i = oper->bl_oper_ptr;
    while(i-> cdr != NULL) {
       if(i-> car != NULL) {
          retval = bl_ctx_eval(closure,i->car);
       }
       i = i->cdr;
    }
    if(i->car != NULL) {
       retval = bl_ctx_eval(closure,i->car);
    }
    return retval;
}

bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr) {
    bl_val_t* meta   = NULL;
    bl_val_t* retval = NULL;
    bl_val_t* i = expr;
    while(i->cdr != NULL) {
          if(i->car != NULL) {
		meta = bl_ctx_eval(ctx,i->car);
		retval = bl_list_append(retval,meta);
          }
     	  i = i->cdr;
    }
    if(i->car != NULL) {
		meta = bl_ctx_eval(ctx,i->car);
		retval = bl_list_append(retval,meta);
    }
    if(retval) retval->eval_last = meta;
    return retval;
}

bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr) {
    bl_val_t* retval = NULL;
    bool in_func = false;
    while(true) {
	    if(expr == NULL) return bl_mk_null();
	    bl_val_t* symval  = NULL;
	    bl_val_t* car     = NULL;
	    bl_val_t* retval  = NULL;
	    bl_val_t* new_closure = NULL;
	    switch(expr->type) {
	      case BL_VAL_TYPE_CONS:
		   if(expr->car==NULL) {
                      return expr;
		   } else {
			   if(expr->car->type == BL_VAL_TYPE_SYMBOL) {
	 			car = bl_ctx_get(ctx, expr->car->s_val);
				if(car == NULL)  return bl_err_symnotfound(expr->car->s_val);
			   } else {
				car = expr->car;
			   }
			   switch(car->type) {
				case BL_VAL_TYPE_OPER_NATIVE:
					expr = car->code_ptr(ctx, expr->cdr);
				break;
				case BL_VAL_TYPE_OPER_DO:
					expr = expr->cdr;
					in_func = true;
				break;
				case BL_VAL_TYPE_FUNC_BL:
					new_closure = bl_ctx_new(car->lexical_closure);
					bl_set_params(new_closure,car->bl_funcargs_ptr,expr->cdr);
					ctx = new_closure;
					expr = car->bl_func_ptr;
					in_func = true;
				break;
				default:
					retval = bl_eval_cons(ctx, expr);
					if(in_func) return retval->eval_last;
					return retval;
				break;
			   }
		}
              break;
	      case BL_VAL_TYPE_SYMBOL:
	           symval = bl_ctx_get(ctx, expr->s_val);
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

bl_val_t* bl_ctx_get(bl_val_t* ctx, char* key) {
   if(ctx->secondary != NULL) {
      bl_val_t* s_V = bl_ctx_get(ctx->secondary, key);
      if(s_V != NULL) return s_V;
   }
   struct bl_hash_t* ht = ctx->hash_val;
   struct bl_hash_t* val = NULL;
   HASH_FIND_STR(ht, key, val);
   if(!val) {
      if(ctx->parent != NULL) {
	 bl_val_t* V = bl_ctx_get(ctx->parent, key);
	 if(V != NULL) return V;
      } else {
         return NULL;
      }
   } else {
      return val->val;
   }
}

bl_val_t* bl_ctx_set(bl_val_t* ctx, char* key, bl_val_t* val) {
   if(ctx->parent != NULL) {
      if(ctx->write_to_parent) return bl_ctx_set(ctx->parent, key, val);
   }
   struct bl_hash_t* ht_val = (struct bl_hash_t*)GC_MALLOC(sizeof(struct bl_hash_t));
   snprintf(ht_val->key,32,"%s",key);
   ht_val->val = val;
   struct bl_hash_t* ignored = NULL;
   HASH_REPLACE_STR(ctx->hash_val,key,ht_val,ignored);
   return val;
}
