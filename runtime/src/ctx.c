#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/builtins.h>
#include <bearlang/list_ops.h>
#include <stdlib.h>
#include <stdio.h>

bl_val_t* bl_ctx_new_std() {
   bl_val_t* retval = bl_ctx_new(NULL);

   bl_val_t* builtin_oper_add  = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   bl_val_t* builtin_oper_sub  = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   bl_val_t* builtin_oper_mult = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   bl_val_t* builtin_oper_div  = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   bl_val_t* builtin_oper_set  = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));

   builtin_oper_add->type  = BL_VAL_TYPE_OPER_NATIVE;
   builtin_oper_sub->type  = BL_VAL_TYPE_OPER_NATIVE;
   builtin_oper_mult->type = BL_VAL_TYPE_OPER_NATIVE;
   builtin_oper_div->type  = BL_VAL_TYPE_OPER_NATIVE;
   builtin_oper_set->type  = BL_VAL_TYPE_OPER_NATIVE;

   builtin_oper_add->code_ptr  = &bl_oper_add;
   builtin_oper_sub->code_ptr  = &bl_oper_sub;
   builtin_oper_mult->code_ptr = &bl_oper_mult;
   builtin_oper_div->code_ptr  = &bl_oper_div;
   builtin_oper_fn->code_ptr   = &bl_oper_fn;
   builtin_oper_set->code_ptr  = &bl_oper_set;

   bl_ctx_set(retval,  "+", builtin_oper_add);
   bl_ctx_set(retval,  "-", builtin_oper_sub);
   bl_ctx_set(retval,  "*", builtin_oper_mult);
   bl_ctx_set(retval,  "/", builtin_oper_div);
   bl_ctx_set(retval,  "=", builtin_oper_set);

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

bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr) {
    if(expr->car == NULL) { // should never happen with a non-null cdr
       return expr;
    }
    if(expr->car->type == BL_VAL_TYPE_SYMBOL) {
       bl_val_t* symval = bl_ctx_get(ctx, expr->car->s_val);
       switch(symval->type) {
         case BL_VAL_TYPE_OPER_NATIVE:
          return symval->code_ptr(ctx, expr->cdr);
	 break;
	 default:
          return bl_eval_cons(ctx, symval);
	 break;
       }
    } else if (expr != NULL) {
       bl_val_t* retval = NULL;
       bl_val_t* i = expr;
       while(i->car != NULL) {
          retval = bl_list_append(retval,bl_ctx_eval(ctx,i->car));
          i = i->cdr;
       }
       if(i->car != NULL) {
          retval = bl_list_append(retval,bl_ctx_eval(ctx,i->car));
       }

    }
}

bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr) {
    bl_val_t* retval = NULL;
    bl_val_t* symval = NULL;
    switch(expr->type) {
      case BL_VAL_TYPE_CONS:
           return bl_eval_cons(ctx, expr);
      break;
      case BL_VAL_TYPE_SYMBOL:
           symval = bl_ctx_get(ctx, expr->s_val);
	   return bl_ctx_eval(ctx,symval);
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
      if(ctx->parent) {
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
