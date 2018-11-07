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
	        retval = bl_eval(ctx,i->car);
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
            if(L_start==NULL) return bl_mk_null(); // LCOV_EXCL_BR_LINE
	    return L_start;
    return bl_mk_null();
}

// TODO - clean up this fucking mess
// TODO - add support for tail recursive when using if statements
bl_val_t* bl_eval_funcbody(bl_val_t* func, bl_val_t* i) {
    bl_val_t* retval = bl_mk_null();
    bl_val_t* symval = NULL;
    bool cont = false;
    while(true) {

           if(i->car->type == BL_VAL_TYPE_CONS) {
		   if(i->car->car->type == BL_VAL_TYPE_SYMBOL) {
           symval = bl_ctx_get(func->inner_closure,i->car->car);
           if(symval == func) {

             bl_val_t* args = bl_eval(func->inner_closure,i->car->cdr);
	     if(bl_list_len(args)>0) bl_set_params(func->inner_closure,func->bl_funcargs_ptr,args);
             i=func->bl_func_ptr;
	     cont = true;
	   } else { 
             retval = bl_eval(func->inner_closure,i->car);
	     i = i->cdr;
             if(i != NULL) cont = true;
	   }
	} else {
          retval = bl_eval(func->inner_closure,i->car);
	  i = i->cdr;
          if(i != NULL) cont = true;
	}
       } else {
          retval = bl_eval(func->inner_closure,i->car);
	  i = i->cdr;
          if(i != NULL) cont = true;
       }
       if(cont) {
	  cont=false;
       } else {
	  return retval;
       }

    }
}

bl_val_t* bl_eval(bl_val_t* ctx, bl_val_t* expr) { // LCOV_EXCL_LINE
    if(expr==NULL) return bl_mk_null();
    bl_val_t* retval = bl_mk_null();
    bool in_func = false;
    bool in_oper = false;
    while(true) {
	    if(expr == NULL) return bl_mk_null(); // LCOV_EXCL_BR_LINE
	    if(expr->type == BL_VAL_TYPE_ERROR) return expr;
	    bl_val_t* symval  = NULL;
	    bl_val_t* car     = NULL;
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
				     if(expr->cdr == NULL) expr->cdr = bl_mk_null();
            			     expr->cdr->invoked_sym = expr->car; // let the operator know what symbol was used to invoke it
				     expr->cdr->custom_data = car->custom_data; // pass any custom data
				     retval = car->code_ptr(ctx, expr->cdr);
				     if(retval == NULL) return bl_mk_null();
				     return retval;
				break;
				case BL_VAL_TYPE_OPER_DO:
					expr = expr->cdr;
					for(i=expr; i != NULL; i=i->cdr) {
						retval = bl_eval(ctx,i->car);
						if(retval != NULL) { 
							if(retval->type == BL_VAL_TYPE_ERROR) return retval;
						}
					}
					if(retval == NULL) return bl_mk_null();
					return retval;
				break;
				case BL_VAL_TYPE_OPER_IF:
					cond = bl_eval(ctx,bl_list_first(expr->cdr));
					if(cond != NULL) {
						if(cond->type == BL_VAL_TYPE_ERROR) return cond;
						if(cond->type != BL_VAL_TYPE_BOOL) return bl_mk_err(BL_ERR_PARSE);
					
						if(cond->b_val) {
							expr = bl_list_second(expr->cdr);
						} else {
							expr = bl_list_third(expr->cdr);
						}
					}
				break;
				case BL_VAL_TYPE_OPER_WHILE:
					cond = bl_list_first(expr->cdr);
					while(bl_eval(ctx, cond)->b_val==true) {
						for(i=bl_list_rest(expr->cdr); i != NULL; i=i->cdr) {
    							retval = bl_eval(ctx,i->car);
							if(retval != NULL) { // LCOV_EXCL_BR_LINE
	    							if(retval->type == BL_VAL_TYPE_ERROR) return retval;
							}
						}
					}
					return bl_mk_null();
				break;
				case BL_VAL_TYPE_FUNC_BL:
					args        = bl_eval(ctx,expr->cdr);
					//car->inner_closure = bl_ctx_new(car->lexical_closure);
					if(bl_list_len(expr) > 1) bl_set_params(car->inner_closure,car->bl_funcargs_ptr,args);
					retval = bl_eval_funcbody(car,car->bl_func_ptr);
/*
					ctx = car->inner_closure;
					expr = car->bl_func_ptr;

					for(i=expr; i != NULL; i=i->cdr) {
						retval = bl_eval(ctx,i->car);
						if(retval != NULL) { 
							if(retval->type == BL_VAL_TYPE_ERROR) return retval;
						}
					}*/
					if(retval==NULL) return bl_mk_null();
					return retval;
					
				break;
				case BL_VAL_TYPE_OPER_BL:
					args        = expr->cdr;
					car->inner_closure = bl_ctx_new(car->lexical_closure);
					if(bl_list_len(expr) > 1) bl_set_params(car->inner_closure,car->bl_operargs_ptr,args);
					ctx = car->inner_closure;
					ctx->write_to_parent = true;
					expr = car->bl_oper_ptr;
					for(i=expr; i != NULL; i=i->cdr) {
						retval = bl_eval(ctx,i->car);
						if(retval != NULL) { 
							if(retval->type == BL_VAL_TYPE_ERROR) return retval;
						}
					}
					if(retval==NULL) return bl_mk_null();
					return retval;
				break;

				default:
				

					retval = bl_eval_cons(ctx, expr);
					if(retval==NULL) return bl_mk_null();
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

