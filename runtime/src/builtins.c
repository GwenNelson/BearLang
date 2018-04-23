#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/builtins.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/types.h>
#include <bearlang/error_tools.h>
#include <bearlang/utils.h>

#include <stdio.h>

bl_val_t* bl_oper_add(bl_val_t* ctx, bl_val_t* params) {

   bl_val_t* L = bl_eval_cons(ctx,params);

   bl_val_t* retval = bl_errif_invalid_len(L,1,BL_LONGEST_LIST);
   if(retval != NULL) return retval;

   bl_val_t* first = bl_ctx_eval(ctx,bl_list_first(params));

   if((first->type == BL_VAL_TYPE_CONS) && (bl_list_len(params)==1)) {
      params = first;
      first  = bl_list_first(params);
   }


   if(first->type == BL_VAL_TYPE_NUMBER) {
      retval = bl_mk_number(0);
   } else {
      retval = bl_mk_str("");
   }

   bl_val_t* x = NULL;
   char*     s = NULL;
   size_t    c = 0;
   char*     buf = NULL;

   L = params;

   while(L->cdr != NULL) {
        if(L->car != NULL) {
           x = bl_ctx_eval(ctx,L->car);
	   if(retval->type == BL_VAL_TYPE_NUMBER) {
 	     retval->i_val += x->i_val;
	   } else {
             s = bl_ser_naked_sexp(x);
	     c = strlen(s) + strlen(retval->s_val)+5;
             buf = GC_MALLOC(c);
             snprintf(buf,c,"%s%s", retval->s_val,s);
	     retval->s_val = buf; 
	   }
	}
	L = L->cdr;
   }
   if(L->car != NULL) {
        x = bl_ctx_eval(ctx,L->car);
	   if(retval->type == BL_VAL_TYPE_NUMBER) {
             retval->i_val += x->i_val;
	   } else {
             s = bl_ser_naked_sexp(x);
	     c = strlen(s) + strlen(retval->s_val)+5;
             buf = GC_MALLOC(c);
             snprintf(buf,c,"%s%s", retval->s_val,s);
	     retval->s_val = buf; 
	   }
   }
   return retval;
}

bl_val_t* bl_oper_sub(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx,params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;
     
   retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));

   retval->type     = BL_VAL_TYPE_NUMBER;

   bl_val_t* first  = bl_list_first(params);
   bl_val_t* second = bl_list_second(params);

   retval->i_val = first->i_val - second->i_val;
   return retval;
}

bl_val_t* bl_oper_mult(bl_val_t* ctx, bl_val_t* params) {
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_NUMBER;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   retval->i_val = first->i_val * second->i_val;
   return retval;
}

bl_val_t* bl_oper_div(bl_val_t* ctx, bl_val_t* params) {
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;

   retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_NUMBER;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   retval->i_val = first->i_val / second->i_val;
   return retval;
}

bl_val_t* bl_oper_set(bl_val_t* ctx, bl_val_t* params) {
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_SYMBOL,BL_VAL_TYPE_ANY};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;
    
   retval = bl_ctx_eval(ctx,bl_list_second(params));
   bl_val_t* name   = bl_list_first(params); // TODO: Handle the case where this isn't a symbol

   bl_ctx_set(ctx, name->s_val, retval);
   return retval;
}

bl_val_t* bl_oper_print(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* i = bl_ctx_eval(ctx,params);
   while(i->cdr != NULL) {
      if(i->car != NULL) {
	 if(i->car->type == BL_VAL_TYPE_STRING) {
            printf("%s",i->car->s_val);
	 } else {
     	    printf("%s", bl_ser_naked_sexp(i->car));
	 }
      }
      i = i->cdr;
   }
   if(i->car != NULL) {
      if(i->car->type == BL_VAL_TYPE_STRING) {
         printf("%s", i->car->s_val);
      } else {
  	 printf("%s", bl_ser_naked_sexp(i->car));
      }
   }
   return bl_mk_null();
}

bl_val_t* bl_oper_fn(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval        = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type            = BL_VAL_TYPE_FUNC_BL;
   retval->bl_funcargs_ptr = bl_list_first(params);
   retval->bl_func_ptr     = bl_list_rest(params);
   retval->lexical_closure = ctx;
   return retval;
}

bl_val_t* bl_oper_fun(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval        = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type            = BL_VAL_TYPE_FUNC_BL;
   retval->bl_funcargs_ptr = bl_list_second(params);
   retval->bl_func_ptr     = bl_list_rest(bl_list_rest(params)); // the rest of the rest is better than the rest
   retval->lexical_closure = ctx;

   bl_val_t* name = bl_list_first(params);
   bl_ctx_set(ctx, name->s_val, retval);
   return retval;
}

bl_val_t* bl_oper_oper(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval        = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type            = BL_VAL_TYPE_OPER_BL;
   retval->bl_operargs_ptr = bl_list_second(params);
   retval->bl_oper_ptr     = bl_list_rest(bl_list_rest(params));

   bl_val_t* name = bl_list_first(params);
   bl_ctx_set(ctx, name->s_val, retval);
   return retval;
}

bl_val_t* bl_oper_eq(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   bl_val_t* retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_BOOL;

   if(first->type == BL_VAL_TYPE_STRING && second->type == BL_VAL_TYPE_STRING) {
      if(strcmp(first->s_val, second->s_val)==0) {
         retval->i_val = 1;
      } else {
         retval->i_val = 0;
      }
      return retval;
   }
   if(first->i_val == second->i_val) {
      retval->i_val = 1;
   } else {
      retval->i_val = 0;
   }
   return retval;
}

bl_val_t* bl_oper_if(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* cond        = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* then_action = bl_list_second(params);
   bl_val_t* else_action = bl_list_third(params);
   if(cond->i_val == 1) {
      return then_action; 
   } else {
      if(else_action != NULL) return else_action;
   }
   return bl_mk_null();
}

bl_val_t* bl_oper_and(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->i_val == 1) && (second->i_val == 1)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_not(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));

   if(first->i_val == 1) return bl_mk_bool(false);
   return bl_mk_bool(true);
}

bl_val_t* bl_oper_or(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->i_val == 1) || (second->i_val == 1)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_xor(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->i_val == 1) ^ (second->i_val == 1)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_first(bl_val_t* ctx, bl_val_t* params) {
   if(bl_list_len(params)==1) {
      return bl_list_first(bl_ctx_eval(ctx,params));
   }
   return bl_list_first(params);
}

bl_val_t* bl_oper_second(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval = bl_list_second(params);
   return retval;
}

bl_val_t* bl_oper_third(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval = bl_list_third(params);
   return retval;
}

bl_val_t* bl_oper_rest(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval = bl_list_rest(params);
   return retval;
}

bl_val_t* bl_oper_include(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* filename = bl_list_first(params);

   FILE* fd         = fopen(filename->s_val,"r");
   bl_val_t* retval = bl_eval_file(ctx, filename->s_val, fd);
   fclose(fd);
   return retval;
}

bl_val_t* bl_oper_isset(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* sym    = bl_list_first(params);
   bl_val_t* symval = bl_ctx_get(ctx, sym->s_val);
   if(symval == NULL) return bl_mk_bool(false);
   return bl_mk_bool(true);
}


bl_val_t* bl_oper_serexp (bl_val_t* ctx, bl_val_t* params) {
   if(bl_list_len(params)==1) params = bl_list_first(params);
   bl_val_t* retval = bl_mk_str(bl_ser_sexp(params));
   return retval;
}
