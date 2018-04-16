#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/builtins.h>
#include <bearlang/list_ops.h>
#include <bearlang/utils.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

bl_val_t* bl_ctx_new_std() {
   bl_val_t* retval = bl_ctx_new(NULL);

   bl_ctx_set(retval,     "+", bl_mk_native_oper(&bl_oper_add));
   bl_ctx_set(retval,     "-", bl_mk_native_oper(&bl_oper_sub));
   bl_ctx_set(retval,     "*", bl_mk_native_oper(&bl_oper_mult));
   bl_ctx_set(retval,     "/", bl_mk_native_oper(&bl_oper_div));
   bl_ctx_set(retval,     "=", bl_mk_native_oper(&bl_oper_set));
   bl_ctx_set(retval,    "fn", bl_mk_native_oper(&bl_oper_fn));
   bl_ctx_set(retval,   "fun", bl_mk_native_oper(&bl_oper_fun));
   bl_ctx_set(retval,    "eq", bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,    "==", bl_mk_native_oper(&bl_oper_eq));
   bl_ctx_set(retval,    "if", bl_mk_native_oper(&bl_oper_if));
   bl_ctx_set(retval,    "do", bl_mk_native_oper(&bl_oper_do));
   bl_ctx_set(retval, "print", bl_mk_native_oper(&bl_oper_print));

   bl_ctx_set(retval, "True", bl_mk_bool(true));
   bl_ctx_set(retval, "False", bl_mk_bool(false));
   return retval;
}

bl_val_t* bl_ctx_new(bl_val_t* parent) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_CTX;
   retval->parent   = parent;
   retval->hash_val = NULL;
   return retval;
}

void bl_ctx_close(bl_val_t* ctx) {
    GC_FREE(ctx);
}

bl_val_t* bl_eval_blfunc(bl_val_t* ctx, bl_val_t* f, bl_val_t* params) {
    bl_val_t* retval   = NULL;
    bl_val_t* closure  = bl_ctx_new(ctx);
    bl_val_t* argsk_i  = f->bl_funcargs_ptr;
    bl_val_t* argsv_i  = params;
    while(argsk_i->cdr != NULL) {
       if(argsk_i->car != NULL) {
     	    bl_ctx_set(closure, argsk_i->car->s_val, bl_ctx_eval(ctx,argsv_i->car));
       }
       argsk_i = argsk_i -> cdr;
       argsv_i = argsv_i -> cdr;
    }
    if(argsk_i->car != NULL) {
       bl_ctx_set(closure, argsk_i->car->s_val, bl_ctx_eval(ctx,argsv_i->car));
    }

    bl_val_t* i = f->func_ptr;
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
    bl_val_t* retval = NULL;
    if(expr->car == NULL) { // should never happen with a non-null cdr
    }
    bool eval_all = false;
    if(expr->car->type == BL_VAL_TYPE_SYMBOL) {
       bl_val_t* symval = bl_ctx_get(ctx, expr->car->s_val);
       switch(symval->type) {
         case BL_VAL_TYPE_OPER_NATIVE:
          retval = symval->code_ptr(ctx, expr->cdr);
	 break;

         case BL_VAL_TYPE_FUNC_BL:
          retval = bl_eval_blfunc(ctx,symval,expr->cdr);
	 break;

	 default:
          eval_all = true;
	 break;
       }
    } else {
       eval_all = true;
    } 
    if(eval_all==true) {    
       bl_val_t* retval = NULL;
       bl_val_t* i = expr;
       while(i->cdr != NULL) {
          if(i->car != NULL) {
     	    retval = bl_list_append(retval,bl_ctx_eval(ctx,i->car));
          }
     	  i = i->cdr;
       }
       if(i->car != NULL) {
          retval = bl_list_append(retval,bl_ctx_eval(ctx,i->car));
       }
       return retval;
    }
    return retval;
}

bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr) {
    bl_val_t* retval = NULL;
    bl_val_t* symval = NULL;
    switch(expr->type) {
      case BL_VAL_TYPE_CONS:
           retval = bl_eval_cons(ctx, expr);
      break;
      case BL_VAL_TYPE_SYMBOL:
           symval = bl_ctx_get(ctx, expr->s_val);
	   retval = bl_ctx_eval(ctx,symval);
      break;
      default:
           retval = expr;
      break;
   }
   return retval;
}

bl_val_t* bl_ctx_get(bl_val_t* ctx, char* key) {
   struct bl_hash_t* ht = ctx->hash_val;
   struct bl_hash_t* val = NULL;
   HASH_FIND_STR(ht, key, val);
   if(!val) {
      if(ctx->parent != NULL) {
	 return bl_ctx_get(ctx->parent, key);
      } else {
         return NULL;
      }
   } else {
      return val->val;
   }
}

bl_val_t* bl_ctx_set(bl_val_t* ctx, char* key, bl_val_t* val) {
   struct bl_hash_t* ht_val = (struct bl_hash_t*)GC_MALLOC(sizeof(struct bl_hash_t));
   snprintf(ht_val->key,32,"%s",key);
   ht_val->val = val;
   struct bl_hash_t* ignored = NULL;
   HASH_REPLACE_STR(ctx->hash_val,key,ht_val,ignored);
   return val;
}
