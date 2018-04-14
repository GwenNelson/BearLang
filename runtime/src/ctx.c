#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/builtins.h>
#include <bearlang/list_ops.h>
#include <stdlib.h>
#include <stdio.h>

bl_val_t* bl_ctx_new_std() {
   return NULL;
}

bl_val_t* bl_ctx_new(bl_val_t* parent) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_CTX;
   retval->parent   = parent;
   retval->hash_val = NULL;
   return retval;
}

void bl_ctx_close(bl_val_t* ctx) {
}

bl_val_t* bl_eval_cons(bl_val_t* ctx, bl_val_t* expr) {
    if(expr->car == NULL) { // should never happen with a non-null cdr
       return expr;
    }
    if(expr->car->type == BL_VAL_TYPE_SYMBOL) {
       // TODO - move into environment instead of hardcoded
       if(strncmp(expr->car->s_val,"+",1)==0) {
          return bl_oper_add(ctx, expr->cdr);
       } else if (strncmp(expr->car->s_val,"-",1)==0) {
          return bl_oper_sub(ctx, expr->cdr);
       } else if (strncmp(expr->car->s_val,"*",1)==0) {
          return bl_oper_mult(ctx, expr->cdr);
       } else if (strncmp(expr->car->s_val,"/",1)==0) {
          return bl_oper_div(ctx, expr->cdr);
       }
    } else {
       bl_val_t* retval = NULL;
       bl_val_t* i = expr->car;
       while(i->car != NULL) {
          retval = bl_list_append(retval,bl_ctx_eval(ctx,i->car));
          i = i->cdr;
       }

     	    // TODO - eval every element here
    }
}

bl_val_t* bl_ctx_eval(bl_val_t* ctx, bl_val_t* expr) {
    bl_val_t* retval = NULL;
    switch(expr->type) {
      case BL_VAL_TYPE_CONS:
           return bl_eval_cons(ctx, expr);
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
