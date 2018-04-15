#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/builtins.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>

#include <stdio.h>

bl_val_t* bl_oper_add(bl_val_t* ctx, bl_val_t* params) {

   bl_val_t* L=params;

   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type  = BL_VAL_TYPE_NUMBER;
   retval->i_val = 0;

   bl_val_t* x = NULL;

   while(L->cdr != NULL) {
        if(L->car != NULL) {
           x = bl_ctx_eval(ctx,L->car);
	   retval->i_val += x->i_val;
	}
	L = L->cdr;
   }
   if(L->car != NULL) {
      x = bl_ctx_eval(ctx,L->car);
      retval->i_val += x->i_val;
   }
   return retval;
}

bl_val_t* bl_oper_sub(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_NUMBER;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   // TODO: handle alternate data types in here and in oper_add() above
   // TODO: add exceptions etc
   retval->i_val = first->i_val - second->i_val;
   return retval;
}

bl_val_t* bl_oper_mult(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_NUMBER;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   retval->i_val = first->i_val * second->i_val;
   return retval;
}

bl_val_t* bl_oper_div(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_NUMBER;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   retval->i_val = first->i_val / second->i_val;
   return retval;
}

bl_val_t* bl_oper_set(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval = bl_ctx_eval(ctx,bl_list_second(params));
   bl_val_t* name   = bl_list_first(params); // TODO: Handle the case where this isn't a symbol

   bl_ctx_set(ctx, name->s_val, retval);
   return retval;
}

bl_val_t* bl_oper_print(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* i;
   bl_val_t* s;
   i = params;
   while(i->cdr != NULL) {
      if(i->car != NULL) {
	 if(i->car->type == BL_VAL_TYPE_STRING) {
            printf("%s",s->s_val);
	 } else {
     	    printf("%s", bl_ser_sexp(bl_ctx_eval(ctx,i->car)));
	 }
      }
      i = i->cdr;
   }
   if(i->car != NULL) {
      if(i->car->type == BL_VAL_TYPE_STRING) {
         printf("%s", i->car->s_val);
      } else {
  	 printf("%s", bl_ser_sexp(bl_ctx_eval(ctx,i->car)));
      }
   }
   return NULL;
}
