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

   bl_ctx_set(retval,    "None", bl_mk_null());
   bl_ctx_set(retval,       "+", bl_mk_native_oper(&bl_oper_add));
   bl_ctx_set(retval,       "-", bl_mk_native_oper(&bl_oper_sub));
   bl_ctx_set(retval,       "*", bl_mk_native_oper(&bl_oper_mult));
   bl_ctx_set(retval,       "/", bl_mk_native_oper(&bl_oper_div));
   bl_ctx_set(retval,       "%", bl_mk_native_oper(&bl_oper_mod));
   bl_ctx_set(retval,       "=", bl_mk_native_oper(&bl_oper_set));
   bl_ctx_set(retval,      "fn", bl_mk_native_oper(&bl_oper_fn));
   bl_ctx_set(retval,     "fun", bl_mk_native_oper(&bl_oper_fun));
   bl_ctx_set(retval,     "map", bl_mk_native_oper(&bl_oper_map));
   bl_ctx_set(retval,    "oper", bl_mk_native_oper(&bl_oper_oper));
   bl_ctx_set(retval,      "eq", bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,      "lt", bl_mk_native_oper(&bl_oper_lt));
   bl_ctx_set(retval,      "gt", bl_mk_native_oper(&bl_oper_gt));
   bl_ctx_set(retval,   "while", bl_mk_native_oper(&bl_oper_while));
   bl_ctx_set(retval,      "==", bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,   "print", bl_mk_native_oper(&bl_oper_print));
   bl_ctx_set(retval,     "and", bl_mk_native_oper(&bl_oper_and));
   bl_ctx_set(retval,     "not", bl_mk_native_oper(&bl_oper_not));
   bl_ctx_set(retval,      "or", bl_mk_native_oper(&bl_oper_or));
   bl_ctx_set(retval,     "xor", bl_mk_native_oper(&bl_oper_xor));
   bl_ctx_set(retval,   "first", bl_mk_native_oper(&bl_oper_first));
   bl_ctx_set(retval,  "second", bl_mk_native_oper(&bl_oper_second));
   bl_ctx_set(retval,   "third", bl_mk_native_oper(&bl_oper_third));
   bl_ctx_set(retval,    "rest", bl_mk_native_oper(&bl_oper_rest));
   bl_ctx_set(retval,  "append", bl_mk_native_oper(&bl_oper_append));
   bl_ctx_set(retval, "prepend", bl_mk_native_oper(&bl_oper_prepend));
   bl_ctx_set(retval, "reverse", bl_mk_native_oper(&bl_oper_reverse));
   bl_ctx_set(retval,     "car", bl_mk_native_oper(&bl_oper_first));
   bl_ctx_set(retval,     "cdr", bl_mk_native_oper(&bl_oper_rest));
   bl_ctx_set(retval, "include", bl_mk_native_oper(&bl_oper_include));
   bl_ctx_set(retval,  "import", bl_mk_native_oper(&bl_oper_import));
   bl_ctx_set(retval,   "isset", bl_mk_native_oper(&bl_oper_isset));
   bl_ctx_set(retval,  "serexp", bl_mk_native_oper(&bl_oper_serexp));

   bl_ctx_set(retval, "True",  bl_mk_bool(true));
   bl_ctx_set(retval, "False", bl_mk_bool(false));

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
	       bl_ctx_set(ctx, argsk_i->car->s_val, argsv_i->car);
       }
       argsk_i = argsk_i -> cdr;
       argsv_i = argsv_i -> cdr;
    }
    if(argsk_i->car != NULL) {
       bl_ctx_set(ctx, argsk_i->car->s_val, argsv_i->car);
    }
}

bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr, bool build_new_list) {
    bl_val_t* meta   = NULL;
    bl_val_t* retval = NULL;
    bl_val_t* i = expr;
    while(i->cdr != NULL) {
          if(i->car != NULL) {
		meta = bl_ctx_eval(ctx,i->car);
		if(meta == NULL) return retval;
		if(meta->type == BL_VAL_TYPE_ERROR) return meta;
		if(build_new_list) retval = bl_list_append(retval,meta);
          }
     	  i = i->cdr;
    }
    if(i->car != NULL) {
		meta = bl_ctx_eval(ctx,i->car);
		if(meta->type == BL_VAL_TYPE_ERROR) return meta;
		if(build_new_list) retval = bl_list_append(retval,meta);
    }
    if(!build_new_list && (retval==NULL)) {
        retval = bl_mk_null();
	retval->eval_last = meta;
    }
    if(retval) retval->eval_last = meta;
    return retval;
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
	    switch(expr->type) {
	      case BL_VAL_TYPE_CONS:
		   if(expr->car==NULL) {
                      return bl_mk_null();
		   } else {
			   if(expr->car->type == BL_VAL_TYPE_SYMBOL) {
	 			car = bl_ctx_get(ctx, expr->car->s_val);
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
					in_func = true;
				break;
				case BL_VAL_TYPE_OPER_IF:
					cond = bl_ctx_eval(ctx,bl_list_first(expr->cdr));
					if(cond->b_val) {
						expr = bl_list_second(expr->cdr);
					} else {
						expr = bl_list_third(expr->cdr);
					}
				break;
				case BL_VAL_TYPE_FUNC_BL:
					new_closure = bl_ctx_new(car->lexical_closure);
					args        = bl_ctx_eval(ctx,expr->cdr);
					if(bl_list_len(expr) > 1) bl_set_params(new_closure,car->bl_funcargs_ptr,args);
					ctx = new_closure;
					expr = car->bl_func_ptr;
					in_func = true;
				break;
				case BL_VAL_TYPE_OPER_BL:
					ctx = bl_ctx_new(ctx);
					ctx->write_to_parent = true;
					expr = car->bl_oper_ptr;
					in_oper = true;
				break;
				default:
					if(in_func) { 
						retval = bl_eval_cons(ctx,expr,false);
						if(retval != NULL) {
							if(retval->type==BL_VAL_TYPE_ERROR) return retval;
							return retval->eval_last;
						}
					}
					retval = bl_eval_cons(ctx, expr, true);
					if(retval==NULL) return bl_mk_null();
					if(retval->type == BL_VAL_TYPE_ERROR) return retval;
					if(in_oper) expr = retval->eval_last;

					
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
   if(strstr(key,"::")) {

     strstr(key,"::")[0]='\0';
     char* ctx_key = key;
     char* sym_key = key+strlen(ctx_key)+2;
     bl_val_t* other_ctx = bl_ctx_get(ctx, ctx_key);
     if(other_ctx == NULL) return NULL;
     return bl_ctx_get(other_ctx,sym_key);

   }
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
   return NULL;
}

bl_val_t* bl_ctx_set(bl_val_t* ctx, char* key, bl_val_t* val) {
   if(ctx->parent != NULL) {
      if(ctx->write_to_parent) ctx = ctx->parent;
   }

   struct bl_hash_t* cur_val = NULL;
   HASH_FIND_STR(ctx->hash_val, key, cur_val);
   if(!cur_val) {
           struct bl_hash_t* ht_val = (struct bl_hash_t*)GC_MALLOC(sizeof(struct bl_hash_t));
           snprintf(ht_val->key,32,"%s",key);
           ht_val->val = val;
     	   HASH_ADD_STR(ctx->hash_val, key, ht_val);
   } else {
      cur_val->val = val;
   }

   return val;
}
