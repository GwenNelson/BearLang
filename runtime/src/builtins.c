// LCOV_EXCL_START
#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/builtins.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/types.h>
#include <bearlang/error_tools.h>
#include <bearlang/utils.h>
#include <bearlang/string_ops.h>
#include <bearlang/eval.h>
#include <glob.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <dlfcn.h>
#include <gmp.h>
#include <stdbool.h>
#include <libgen.h>
#include <time.h>
// LCOV_EXCL_STOP LCOV_EXCL_LINE


#define PARAM_LEN_CHECK(min,max) bl_val_t* retval = bl_errif_invalid_len(params,min,max); if(retval != NULL) return retval; // LCOV_EXCL_BR_LINE
// LCOV_EXCL_START
bl_val_t* bl_oper_profile(bl_val_t* ctx, bl_val_t* params) {
   clock_t start = clock();
   bl_eval(ctx,params);
   clock_t end = clock();
   double elapsed_time = (end-start)/(double)CLOCKS_PER_SEC;
   char outbuf[512];
   snprintf(outbuf,512,"Took %f seconds", elapsed_time);
   return bl_mk_str(outbuf);
} // LCOV_EXCL_STOP LCOV_EXCL_LINE

bl_val_t* bl_oper_throw(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   PARAM_LEN_CHECK(2,2) // LCOV_EXCL_BR_LINE
   bl_val_t* errtype = bl_list_first(params);
   bl_val_t* errval  = bl_eval(ctx,bl_list_second(params));
   if(errval == NULL) errval = bl_mk_null(); // LCOV_EXCL_BR_LINE
   retval = bl_mk_err(BL_ERR_BL_CUSTOM);
   retval->err_val.err_sym = errtype;
   retval->err_val.err_val = errval;
   return retval; }

// LCOV_EXCL_START
bl_val_t* bl_oper_listbuiltins(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   PARAM_LEN_CHECK(0,0) // LCOV_EXCL_BR_LINE
   #define BUILTIN_RETURN(builtin_name,builtin_symbol,builtin_docstr) retval=bl_list_prepend(retval,bl_mk_symbol(builtin_symbol));
   #define BUILTIN_X BUILTIN_RETURN
   #include <bearlang/builtins.inc>
   #undef BUILTIN_X
   return bl_list_reverse(retval); } // LCOV_EXCL_STOP LCOV_EXCL_LINE

bl_val_t* bl_oper_pmatch(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   PARAM_LEN_CHECK(2,2) // LCOV_EXCL_BR_LINE
   params      = bl_eval(ctx,params);
   retval = bl_mk_null();
   bl_val_t* S = bl_list_first(params);
   bl_val_t* L = bl_list_second(params);
   for(; L != NULL; L=L->cdr) {
       char* prefix = L->car->car->s_val;
       if(strstr(S->s_val,prefix)==S->s_val) { 
 		return L->car->cdr->car;
       }

   }
   return retval;
}

bl_val_t* bl_oper_startswith(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_eval(ctx,params);
   bl_val_t* first  = bl_list_first(params);
   bl_val_t* second = bl_list_second(params);
   if(strstr(first->s_val,second->s_val)==first->s_val) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_split(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params           = bl_eval(ctx,params);
   bl_val_t* first  = bl_list_first(params);
   bl_val_t* second = bl_list_second(params);
   return split_str(first->s_val,second->s_val);
}

bl_val_t* bl_oper_aget(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* sym    = bl_eval(ctx,bl_list_first(params));
   bl_val_t* L      = bl_eval(ctx,bl_list_second(params));
   bl_val_t* retval = bl_err_symnotfound(sym->s_val);
   bl_val_t* L_car;
   for(; L != NULL; L=L->cdr) {
       L_car = L->car;
       if(L_car->type != BL_VAL_TYPE_CONS) { // LCOV_EXCL_START
          return bl_mk_err(BL_ERR_PARSE);
       } // LCOV_EXCL_LINE LCOV_EXCL_STOP
       if(strcmp(L_car->car->s_val,sym->s_val)==0) retval = bl_list_second(L_car);
   }
   return retval;
}

bl_val_t* bl_oper_foreach(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* sym  = bl_list_first(params);
   bl_val_t* L    = bl_eval(ctx,bl_list_second(params));
   bl_val_t* expr = bl_list_third(params);
   for(; L != NULL; L=L->cdr) {
      bl_ctx_set(ctx,sym,L->car);
      bl_eval(ctx,expr);
   }
   bl_ctx_set(ctx,sym,NULL);
   return bl_mk_null();
}

bl_val_t* bl_oper_filtered(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* L     = bl_eval(ctx,bl_list_first(params));
   bl_val_t* items = bl_eval(ctx,bl_list_second(params));
   // TODO implement an "in" operation and set data type and shit LCOV_EXCL_LINE
   bl_val_t* retval = NULL;
   bl_val_t* I      = items;
   for(; L != NULL; L=L->cdr) {
       bool is_in = false;
       for(I=items; I != NULL; I=I->cdr) {
  	   bl_val_t* eq_result = bl_oper_eq(ctx,bl_mk_list(2,L->car,I->car));
	   if(eq_result->b_val==true) is_in=true;
       }
       if(!is_in) retval = bl_list_prepend(retval,L->car);
   }
   return bl_list_reverse(retval); // LCOV_EXCL_LINE
} // LCOV_EXCL_LINE

bl_val_t* bl_oper_parse(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_eval_cons(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params; // LCOV_EXCL_LINE

   bl_val_type_t expected_types[1] = {BL_VAL_TYPE_STRING};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,1);
   if(retval != NULL) return retval; // LCOV_EXCL_LINE
   return bl_parse_sexp(params->car->s_val); // LCOV_EXCL_LINE
} // LCOV_EXCL_LINE

bl_val_t* bl_oper_quote(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   if(bl_list_len(params)==1) return bl_list_first(params);
   return params;
}
// LCOV_EXCL_LINE LCOV_EXCL_START
// basic syntax:
// (try SOME_EXPR   ; optionally could be a (do) type expression
//      (catch SOME_ERR SOME_EXPR)
//      (catch SOME_OTHER_ERR SOME_EXPR)
//      (finally SOME_EXPR))
//
// the SOME_ERR types are defined in the default environment as ERR_* symbols
// e.g ERR_INSUFFICIENT_ARGS ERR_TOOMANY_ARGS etc
// while evaluating the catch blocks, *ERR* is set to the actual error
// LCOV_EXCL_LINE LCOV_EXCL_STOP
bl_val_t* bl_oper_try(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* try_expr    = bl_list_first(params);
   bl_val_t* try_result  = bl_eval(ctx, try_expr);
   if(try_result->type != BL_VAL_TYPE_ERROR) return try_result;

   bl_val_t* other_exprs = bl_list_rest(params);
   bl_val_t* i=other_exprs;
   bl_val_t* catch_sym   = bl_mk_symbol("catch");
   bl_val_t* finally_sym = bl_mk_symbol("finally");
   bl_val_t* catch_err    = NULL;
   bl_val_t* catch_expr   = try_result;
   bl_val_t* finally_expr = NULL;

   bl_ctx_set(ctx,bl_mk_symbol("*ERR*"),try_result);

   for(i=other_exprs; i != NULL; i=i->cdr) {
       // LCOV_EXCL_START
       if(i->car->type != BL_VAL_TYPE_CONS) {
          return bl_mk_err(BL_ERR_PARSE);
       }
       if(bl_list_len(i->car) != 3) {
          return bl_mk_err(BL_ERR_PARSE);
       }

       if(i->car->car == catch_sym) {
     	  catch_err  = bl_ctx_get(ctx,bl_list_second(i->car));
	  if((catch_err->err_val.type == try_result->err_val.type) || (catch_err->err_val.type == BL_ERR_ANY)) {
            catch_expr = bl_list_third(i->car);
	  }
       } else if (i->car->car == finally_sym) {
          finally_expr = bl_list_second(i->car);
       } else {
	 return bl_mk_err(BL_ERR_PARSE);
       }
   }

   bl_val_t* retval = bl_eval(ctx,catch_expr);
   if(finally_expr != NULL) retval = bl_eval(ctx,finally_expr);
   // LCOV_EXCL_STOP
   return retval;
}

// LCOV_EXCL_START
bl_val_t* bl_oper_eval(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_eval(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params;
   if(bl_list_len(params)==1) return bl_eval(ctx,bl_list_first(params));
   return bl_eval(ctx,params);
}
// LCOV_EXCL_STOP


bl_val_t* bl_oper_map(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_eval_cons(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params;

   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_FUNC_BL,BL_VAL_TYPE_CONS};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   bl_val_t* func = bl_list_first(params);
   bl_val_t* L    = bl_list_second(params);

   bl_val_t* func_expr = NULL;
   bl_val_t* retval_L = NULL;

   // TODO - make this support native functions etc
   bl_val_t* i = NULL;
   bl_val_t* j = NULL;
   bl_val_t* tmp=bl_mk_val(BL_VAL_TYPE_CONS);
   for(i=L; i != NULL; i=i->cdr) {
       bl_ctx_set(func->inner_closure,func->bl_funcargs_ptr->car,i->car);
       for(j = func->bl_func_ptr; j != NULL; j=j->cdr) {
           if(retval_L == NULL) {
              retval_L = bl_mk_val(BL_VAL_TYPE_CONS);
	      retval_L->car = bl_eval(func->inner_closure,j->car);
              if(retval_L->car->type == BL_VAL_TYPE_ERROR) return retval_L->car;
	      retval_L->cdr = NULL;
	      retval = retval_L;
	   } else {
              retval_L->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
	      retval_L->cdr->car = bl_eval(func->inner_closure,j->car);
              if(retval_L->cdr->car->type == BL_VAL_TYPE_ERROR) return retval_L->cdr->car;
	      retval_L=retval_L->cdr;
	   }
       }
   }
   return retval;


}

bl_val_t* bl_oper_add(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   if(params->type == BL_VAL_TYPE_NULL) return bl_errif_invalid_len(bl_mk_null(),1,BL_LONGEST_LIST);
   bl_val_t* L = bl_eval_cons(ctx,params);
   if(L->type == BL_VAL_TYPE_ERROR) return L;
   bl_val_t* retval = bl_errif_invalid_len(L,1,BL_LONGEST_LIST);
   if(retval != NULL) return retval;
   bl_val_t* first = bl_eval(ctx,bl_list_first(params));

   if((first->type == BL_VAL_TYPE_CONS) && (bl_list_len(params)==1)) {
      params = first;
      first  = bl_list_first(params);
   }
   bl_val_t* x = NULL;
   switch(first->type) {
      case BL_VAL_TYPE_NUMBER:
           retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
      break;
      case BL_VAL_TYPE_STRING:
           retval = bl_mk_str("");
      break;
      case BL_VAL_TYPE_CONS:
           retval = NULL;
	   for(L=params; L != NULL; L=L->cdr) {
              x = bl_eval(ctx,L->car);
              if(x->type == BL_VAL_TYPE_CONS) {
                for(; x != NULL; x=x->cdr) {
                    retval = bl_list_prepend(retval, x->car);
		}
	      } else {
                retval = bl_list_prepend(retval,x);
	      }
	   }
	   return bl_list_reverse(retval);
      break;
      default:
         retval = bl_mk_str("");
      break;
   }


   char*     s = NULL;
   size_t    c = 0;

   L = params;
   for(L=params; L!= NULL; L=L->cdr) {
           x = bl_eval(ctx,L->car);
        switch(retval->type) {
            case BL_VAL_TYPE_NUMBER:
                    mpz_add(retval->i_val, retval->i_val, x->i_val);
	    break;
	    default:
		 retval->s_val = safe_strcat(retval->s_val,bl_ser_naked_sexp(x));
	    break;
	}
   }
   return retval;
}

bl_val_t* bl_oper_sub(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_eval(ctx,params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;
     
   retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
   retval->fix_int = 0;

   bl_val_t* first  = bl_list_first(params);
   bl_val_t* second = bl_list_second(params);


   mpz_sub(retval->i_val, first->i_val, second->i_val);
//   retval->fix_int = first->fix_int - second->fix_int;
   return retval;
}

bl_val_t* bl_oper_mult(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_eval(ctx, params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
   retval->fix_int = 0;

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

   mpz_mul(retval->i_val, first->i_val, second->i_val);
//   retval->fix_int = first->fix_int * second->fix_int;
   return retval;
}

bl_val_t* bl_oper_div(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_eval_cons(ctx, params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;


      retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
      retval->fix_int = 0;
   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

   if (mpz_get_d(second->i_val)==0) return bl_err_divzero();

   mpz_tdiv_q(retval->i_val, first->i_val, second->i_val);
//   retval->fix_int = first->fix_int / second->fix_int;
   return retval;
}

bl_val_t* bl_oper_mod(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   params = bl_eval(ctx,params);
/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;*/

      bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
      retval->fix_int = 0;

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

   mpz_init(retval->i_val);
   mpz_mod(retval->i_val, first->i_val, second->i_val);
//   retval->fix_int = first->fix_int % second->fix_int;
   return retval;
}

bl_val_t* bl_oper_lt(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   params = bl_eval(ctx,params);
/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;*/

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

//   if(first->fix_int < second->fix_int) return bl_mk_bool(true);
   if(mpz_cmp(first->i_val,second->i_val)>=0) return bl_mk_bool(false);
   return bl_mk_bool(true);
}

bl_val_t* bl_oper_gt(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   params = bl_eval(ctx,params);
/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;*/


   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

//   if(first->fix_int > second->fix_int) return bl_mk_bool(true);
   if(mpz_cmp(first->i_val,second->i_val)<=0) return bl_mk_bool(false);
   return bl_mk_bool(true);
}

bl_val_t* bl_oper_set(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_SYMBOL,BL_VAL_TYPE_ANY};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;*/
    
   bl_val_t* retval = bl_eval(ctx,bl_list_second(params));
   bl_val_t* name   = bl_list_first(params); // TODO: Handle the case where this isn't a symbol

   bl_ctx_set(ctx, name, retval);
   return retval;
}

//LCOV_EXCL_START
bl_val_t* bl_oper_print(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* i = bl_eval_cons(ctx,params);
   if(i->type == BL_VAL_TYPE_ERROR) return i;

   for(i=params; i!= NULL; i=i->cdr) {
     	printf("%s", bl_ser_naked_sexp(bl_eval(ctx,i->car)));

   }
   return bl_mk_null();
}
//LCOV_EXCL_STOP


bl_val_t* bl_oper_fn(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* retval        = bl_mk_val(BL_VAL_TYPE_FUNC_BL);
   retval->bl_funcargs_ptr = bl_list_first(params);
   retval->bl_func_ptr     = bl_list_rest(params);
   retval->lexical_closure = ctx;
   retval->inner_closure   = bl_ctx_new(ctx);
   retval->sym = bl_mk_symbol("anonymous-lambda");
   return retval;
}

bl_val_t* bl_oper_fun(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* retval        = bl_mk_val(BL_VAL_TYPE_FUNC_BL);
   retval->bl_funcargs_ptr = bl_list_second(params);
   retval->bl_func_ptr     = bl_list_rest(bl_list_rest(params)); // the rest of the rest is better than the rest
   if(retval->bl_func_ptr->car->type == BL_VAL_TYPE_DOCSTRING) {
      retval->docstr = retval->bl_func_ptr->car;
      retval->bl_func_ptr = retval->bl_func_ptr->cdr;
   }
   retval->lexical_closure = ctx;
   retval->inner_closure   = bl_ctx_new(ctx);
   retval->sym = bl_list_first(params); 
   
   bl_ctx_set(ctx, retval->sym, retval);
   return retval;
}

bl_val_t* bl_oper_oper(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* retval        = bl_mk_val(BL_VAL_TYPE_OPER_BL);
   retval->bl_operargs_ptr = bl_list_second(params);
   retval->bl_oper_ptr     = bl_list_rest(bl_list_rest(params));

   // LCOV_EXCL_START
  if(retval->bl_func_ptr->car->type == BL_VAL_TYPE_DOCSTRING) {
      retval->docstr = retval->bl_oper_ptr->car;
      retval->bl_oper_ptr = retval->bl_oper_ptr->cdr;
   }
  // LCOV_EXCL_STOP

   retval->lexical_closure = ctx;
   retval->inner_closure   = bl_ctx_new(ctx);

   bl_val_t* name = bl_list_first(params);
   bl_ctx_set(ctx, name, retval);
   return retval;
}

bl_val_t* bl_oper_eq(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

   // LCOV_EXCL_START
   if(first->type == BL_VAL_TYPE_TYPE && second->type == BL_VAL_TYPE_TYPE) {
      if(first->ref_type == second->ref_type) {
         return bl_mk_bool(true);
      } else {
         return bl_mk_bool(false);
      }
   }
   // LCOV_EXCL_STOP


   if(first->type == BL_VAL_TYPE_STRING && second->type == BL_VAL_TYPE_STRING) {
      if(strcmp(first->s_val, second->s_val)==0) {
         return bl_mk_bool(true);
      } else {
         return bl_mk_bool(false);
      }
   }
/*   if(first->fix_int == second->fix_int) {
 	return bl_mk_bool(true);
   } else {
	return bl_mk_bool(false);
   }*/
   if(mpz_cmp(first->i_val, second->i_val)==0) {
      return bl_mk_bool(true);
   } else {
      return bl_mk_bool(false);
   }
}

bl_val_t* bl_oper_and(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

   if((first->b_val) && (second->b_val)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_not(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));

   if(first->b_val) return bl_mk_bool(false);
   return bl_mk_bool(true);
}

bl_val_t* bl_oper_or(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

   if(first->b_val) return bl_mk_bool(true);
   if(second->b_val) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_xor(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_eval(ctx,bl_list_second(params));

   if((first->b_val) ^ (second->b_val)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_first(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_eval(ctx,params);
   if(bl_list_len(params)==1) return bl_list_first(bl_list_first(params));
   return bl_list_first(params);
}

bl_val_t* bl_oper_second(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_eval(ctx,params);
   if(bl_list_len(params)==1) return bl_list_second(bl_list_first(params));
   bl_val_t* retval = bl_list_second(params);
   return retval;
}

bl_val_t* bl_oper_third(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   if(bl_list_len(params)==1) params = bl_eval(ctx,bl_list_first(params));
   bl_val_t* retval = bl_list_third(params);
   return retval;
}

bl_val_t* bl_oper_rest(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE


   params = bl_eval(ctx,params);

   if(bl_list_len(params)==1) params = bl_eval(ctx,bl_list_first(params));
   bl_val_t* retval = bl_list_rest(params);
   if(retval == NULL) return bl_mk_null();
   return retval;
}

bl_val_t* bl_oper_include(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* filename = bl_eval(ctx,bl_list_first(params));

   FILE* fd         = fopen(filename->s_val,"r");
   bl_val_t* retval = bl_eval_file(ctx, filename->s_val, fd);
   fclose(fd);
   return retval;
}

typedef bl_val_t* (*mod_init_fn)(bl_val_t*);

bl_val_t* bl_oper_import(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
     bl_val_t* first = bl_list_first(params);
     bl_val_t* module_name = first;
     bl_val_t* new_ctx;



     // check if we're importing a module from another module
     if(strstr(module_name->s_val,"::")!=NULL) {
        bl_val_t* split = split_str(module_name->s_val,"::");
	char* other_ctx_name = split->car->s_val;
	bl_val_t* other_ctx_sym = bl_mk_symbol(other_ctx_name); 
	bl_val_t* other_ctx     = bl_ctx_get(ctx,other_ctx_sym);

	if(other_ctx==NULL) {
           return bl_err_modnotfound(other_ctx_name); // the first entry in the list is the parent module, if it isn't present return an error
        }

        char* other_ctx_kname;
	bl_val_t* other_ctx_key;

	if(bl_list_len(split)>2) { // we just recurse into the list, because it's nested >2 layers
           other_ctx_kname  = join_str(split->cdr,"::");
           other_ctx_key = bl_mk_symbol(other_ctx_kname);
           new_ctx = bl_oper_import(other_ctx,bl_mk_list(1,other_ctx_key));
	
	} else {
           other_ctx_kname = split->cdr->car->s_val;
           other_ctx_key = bl_mk_symbol(other_ctx_kname);
	   new_ctx = bl_ctx_get(other_ctx,other_ctx_key);

	}
	   if(new_ctx == NULL) return bl_err_modnotfound(other_ctx_kname);
	   bl_ctx_set(ctx, bl_mk_symbol(bl_list_last(split)->s_val), new_ctx);
	   return new_ctx;
     }

     // see if we already imported, and if so just return the already imported module
     // LCOV_EXCL_START
     if(first->type == BL_VAL_TYPE_SYMBOL) {
        module_name = bl_ctx_get(ctx,first);
	if(module_name == NULL) module_name = first;
	if(module_name->type == BL_VAL_TYPE_CTX) return module_name;
     }
     // LCOV_EXCL_STOP


     // if we get here, we need to glob for the module lib or source

     glob_t globbuf;

     bl_val_t* p_i;
     int i=0;

     // get the current path and module filename, and if not set, set some defaults
     bl_val_t* cur_path     = bl_ctx_get(ctx,bl_mk_symbol("*PATH*"));
     bl_val_t* mod_filename = bl_ctx_get(ctx,bl_mk_symbol("*FILENAME*"));
     if(cur_path==NULL)     cur_path = bl_mk_str("."); // LCOV_EXCL_BR_LINE
     if(mod_filename==NULL) mod_filename = bl_mk_str(""); // LCOV_EXCL_BR_LINE

     // get the directory where the currently running module is located, we need this so that modules can do relative imports
     char* mod_filename_s = strdup(mod_filename->s_val);
     char* mod_dirname = NULL;
     if(strlen(mod_filename_s)>0) {
        mod_dirname  = dirname(mod_filename_s);
     }
     free(mod_filename_s);

     // start building the glob pattern, starting with the directory where the current module is located
     char* pattern = "{";
     if(mod_dirname != NULL) {
        if(strlen(mod_dirname)>0) { // LCOV_EXCL_BR_LINE
           pattern = safe_strcat(pattern,mod_dirname);
           pattern = safe_strcat(pattern,",");
	}
     }

     // add the directories listed in *PATH*
     for(p_i=cur_path; p_i != NULL; p_i = p_i->cdr) {
         pattern = safe_strcat(pattern,p_i->car->s_val);
	 pattern = safe_strcat(pattern,",");
     }

     // close the bracket for directories and start building rest of pattern (filenames)
     pattern[strlen(pattern)-1] = '}';
     pattern = safe_strcat(pattern,"/{");
     
     // <module>.bl
     pattern = safe_strcat(pattern,module_name->s_val);
     pattern = safe_strcat(pattern,".bl,");

     // lib<module>.so / .dylib
     pattern = safe_strcat(pattern,"lib");
     pattern = safe_strcat(pattern,module_name->s_val);
     pattern = safe_strcat(pattern,"{.dylib,.so},");

     // <module>/module.bl
     pattern = safe_strcat(pattern,module_name->s_val);
     pattern = safe_strcat(pattern,"/module.bl,");

     // <module>/lib<module>.so / .dylib
     pattern = safe_strcat(pattern,module_name->s_val);
     pattern = safe_strcat(pattern,"/lib");
     pattern = safe_strcat(pattern,module_name->s_val);
     pattern = safe_strcat(pattern,"{.dylib,.so}");

     // close up
     pattern = safe_strcat(pattern,"}");

     // at this point, the glob pattern is something like {.,./stdlib}/{libhello{.so,.dylib},hello/module.bl,hello/libhello{.so,.dylib}}
     // that glob pattern should match any possible source for the module

     char* found_path = "";
     glob(pattern,GLOB_TILDE|GLOB_MARK|GLOB_BRACE,NULL,&globbuf);
         for(i=0; i<globbuf.gl_pathc; i++) {
             if(globbuf.gl_pathv[i][strlen(globbuf.gl_pathv[i])-1] != '/') {  // LCOV_EXCL_BR_LINE
		     found_path = GC_MALLOC_ATOMIC(4096);
	             snprintf(found_path,4096,"%s",globbuf.gl_pathv[i]);
                     break; // break out on first match to preserve path priority
             }
	 } // LCOV_EXCL_LINE
	 globfree(&globbuf);

     if(strlen(found_path)==0) {
        return bl_err_modnotfound(module_name->s_val); // if glob failed, return an error
     }

     // get the file extension so we can figure out the appropriate way to import
     char* fname = GC_MALLOC_ATOMIC(4096);
     snprintf(fname,4096,"%s", found_path);
     fname = basename(fname);
     char* fext = strrchr(fname, '.')+1;

     if(strcmp(fext,"bl")!=0) {
      bl_val_t* dylib_val = bl_mk_val(BL_VAL_TYPE_CPTR);
      dylib_val->c_ptr = dlopen(found_path,RTLD_NOW);
      if(!dylib_val->c_ptr) fprintf(stderr, "dlopen error: %s\n", dlerror()); // TODO: add proper error handling here LCOV_EXCL_LINE
      mod_init_fn mod_init = dlsym(dylib_val->c_ptr, "bl_mod_init");
      char* err = dlerror();
      if(err) { 
         bl_val_t* ret_err = bl_mk_err(BL_ERR_CUSTOM);
	 ret_err->err_val.errmsg = strdup(err);
	 return ret_err;
      }

      // assuming the above all worked ok, we now just init the module, bind it and return
      new_ctx = mod_init(ctx);
      bl_ctx_set(new_ctx, bl_mk_symbol("*FILENAME*"), bl_mk_str(found_path));
      bl_ctx_set(ctx, bl_mk_symbol(module_name->s_val), new_ctx);
      return new_ctx;

     } else { // .bl module time!

      FILE* fd = fopen(found_path,"r");
      bl_val_t* new_ctx = bl_ctx_new(ctx);
      bl_ctx_set(new_ctx, bl_mk_symbol("*FILENAME*"), bl_mk_str(found_path));
      bl_val_t* result = bl_eval_file(new_ctx, fname, fd);
      fclose(fd);

      // if we had an error while evaluating the source code, return the error
      if(result->type == BL_VAL_TYPE_ERROR) return result; // LCOV_EXCL_LINE

      // otherwise, just bind it and return
      bl_ctx_set(ctx, bl_mk_symbol(module_name->s_val), new_ctx);
      return new_ctx;

     }
     return bl_err_modnotfound(module_name->s_val); // we should never actually hit this, but if we do something went badly wrong and the import definitely failed

}

// syntax
// (using stdio::fprintf) binds fprintf symbol in the current context
bl_val_t* bl_oper_using(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* sym = bl_list_first(params);
   char* s = sym->s_val;
   bl_val_t* split = split_str(s,"::");
   bl_val_t* lastsym  = bl_mk_symbol(bl_list_last(split)->s_val); // this is the symbol we're importing, or *

   // get the whole split list except for the last bit
   bl_val_t* rev_split = bl_list_reverse(split);
   bl_val_t* all_but_last = bl_list_reverse(bl_list_rest(rev_split));

   // turn it back into one symbol so we can grab the ctx, not very efficient, but doesn't need to be
   // also far simpler to read and debug
   bl_val_t* other_ctx_sym = bl_mk_symbol(join_str(all_but_last,"::"));

   // get the other ctx
   bl_val_t* other_ctx = bl_ctx_get(ctx,other_ctx_sym);
   if(other_ctx == NULL) return bl_err_modnotfound(other_ctx_sym->s_val);

   // if lastsym is *, we grab all symbols from the other ctx and import them
   // otherwise, we just import the one symbol
   if(strcmp(lastsym->s_val,"*")==0) {
     int i=0;
     for(i=0; i < other_ctx->vals_count; i++) {
         if(other_ctx->vals[i] != NULL) {
            bl_ctx_set(ctx,other_ctx->keys[i],other_ctx->vals[i]);
	 }
     }
     return bl_mk_null();
   } else {
     bl_val_t* symval = bl_ctx_get(other_ctx,lastsym);
     if(symval == NULL) return bl_err_symnotfound(lastsym->s_val); // if not found, return suitable error
     bl_ctx_set(ctx,lastsym,symval); // bind into current ctx
     return symval;
   }
}

bl_val_t* bl_oper_isset(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* sym    = bl_list_first(params);
   bl_val_t* symval = bl_ctx_get(ctx, sym);
   if(symval == NULL) return bl_mk_bool(false);
   return bl_mk_bool(true);
}


bl_val_t* bl_oper_serexp (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_eval(ctx,params);
   // LCOV_EXCL_START
   if(params->type == BL_VAL_TYPE_CONS) {
      if(bl_list_len(params)==1) params = bl_list_first(params);
   }
   // LCOV_EXCL_STOP
   bl_val_t* retval = bl_mk_str(bl_ser_sexp(params));
   return retval;
}

bl_val_t* bl_oper_append(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* L = bl_eval(ctx,bl_list_first(params));
   return bl_list_append(L, bl_eval(ctx,bl_list_second(params)));
}

bl_val_t* bl_oper_prepend(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* L = bl_eval(ctx,bl_list_first(params));
   return bl_list_prepend(L, bl_eval(ctx,bl_list_second(params)));
}

bl_val_t* bl_oper_reverse(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE


   params = bl_eval(ctx,params);

   if(bl_list_len(params)==1) params = bl_eval(ctx,bl_list_first(params));
   bl_val_t* retval = bl_list_reverse(params);
   return retval;
}

bool has_init_integer_one = false;
static mpz_t integer_one;

bl_val_t* bl_oper_inc(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   if(!has_init_integer_one) { 
      mpz_init_set_ui(integer_one,1);
      has_init_integer_one = true;
   }
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_get(ctx,symname);
   //symval->fix_int++;
   mpz_add(symval->i_val, symval->i_val, integer_one);
   return symval;
}

bl_val_t* bl_oper_dec(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   if(!has_init_integer_one) {
      mpz_init_set_ui(integer_one,1);
      has_init_integer_one = true;
   }
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_get(ctx,symname);
   //symval->fix_int--;
   mpz_sub(symval->i_val, symval->i_val, integer_one);
   return symval;
}

//LCOV_EXCL_START
// utility function used by below oper to generate documentation for a module
bl_val_t* bl_genmod_doc(bl_val_t* ctx, bl_val_t* sym, bl_val_t* mod) { // LCOV_EXCL_LINE
     bl_val_t* NAME = bl_ctx_get(mod,bl_mk_symbol("*NAME*"));
     if(NAME==NULL) NAME=sym;

     char* retval = "NAME\n\t";
     retval = safe_strcat(retval,NAME->s_val);

     bl_val_t* SUMMARY = bl_ctx_get(mod,bl_mk_symbol("*SUMMARY*"));
     if(SUMMARY!=NULL) {
        retval = safe_strcat(retval," - ");
	retval = safe_strcat(retval,SUMMARY->s_val);
     }

     retval = safe_strcat(retval,"\n");

     bl_val_t* DESCRIPTION = bl_ctx_get(mod, bl_mk_symbol("*DESCRIPTION*"));
     if(DESCRIPTION!=NULL) {
	retval = safe_strcat(retval,"\nDESCRIPTION\n");
        bl_val_t* split_desc = split_str(DESCRIPTION->s_val,"\n");
	for(; split_desc != NULL; split_desc=split_desc->cdr) {
	    retval = safe_strcat(retval,"\t");
            retval = safe_strcat(retval,split_desc->car->s_val);
	    retval = safe_strcat(retval,"\n");
	}
	retval = safe_strcat(retval,"\n");
     }

     bl_val_t* EXAMPLE = bl_ctx_get(mod, bl_mk_symbol("*EXAMPLE*"));
     if(EXAMPLE!=NULL) {
	retval = safe_strcat(retval,"\nEXAMPLE\n");
        bl_val_t* split_desc = split_str(EXAMPLE->s_val,"\n");
	for(; split_desc != NULL; split_desc=split_desc->cdr) {
	    retval = safe_strcat(retval,"\t");
            retval = safe_strcat(retval,split_desc->car->s_val);
	    retval = safe_strcat(retval,"\n");
	}
     }

     return bl_mk_str(retval);
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
bl_val_t* bl_oper_doc (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

     bl_val_t* first      = bl_list_first(params);
     bl_val_t* first_eval = bl_eval(ctx,first);

     if(first_eval->type == BL_VAL_TYPE_SYMBOL) first_eval = bl_ctx_get(ctx,first_eval);
     char* retval = "";

     // first, check if the symbol exists
     if(first_eval==NULL) {
        return bl_err_symnotfound(first->s_val);
     }

     // now check if there's a docstring
     bl_val_t* docstr = first_eval->docstr;

     if(docstr == NULL) {
        if(first_eval->type == BL_VAL_TYPE_CTX) {
           docstr = bl_ctx_get(first_eval,bl_mk_symbol("DOC"));
	}
     } else {
        return bl_mk_str(docstr->s_val);
     }

     char* buf = NULL;
     if(docstr == NULL) { // if it doesn't exist, generate one dynamically
        switch(first_eval->type) {
            case BL_VAL_TYPE_FUNC_BL:
                    if(first->type == BL_VAL_TYPE_SYMBOL) {
           		    docstr = bl_mk_str(bl_ser_sexp(bl_list_prepend(first_eval->bl_funcargs_ptr,first)));
		    } else {
                            docstr = bl_mk_str(bl_ser_sexp(bl_list_prepend(first_eval->bl_funcargs_ptr,first_eval->sym)));
		    }
	    break;
	    case BL_VAL_TYPE_CTX:
		    docstr = bl_genmod_doc(ctx, first,first_eval);
	    break;
	    default:
		    docstr = bl_mk_str(bl_ser_sexp(first_eval));
	    break;
	}
     }

     retval = safe_strcat(retval,docstr->s_val);

     if(first_eval->type == BL_VAL_TYPE_CTX) {
        retval = safe_strcat(retval,"\nMEMBERS");
        int i=0;
        for(i=0; i<first_eval->vals_count; i++) {
           if(first_eval->vals[i] != NULL) {
              if(strcmp(first_eval->keys[i]->s_val,"DOC") != 0) if ( first_eval->keys[i]->s_val[0] != '*') {
                 if(first_eval->vals[i]->type != BL_VAL_TYPE_CTX) {
			 retval=safe_strcat(retval,"\n\t");
		         retval=safe_strcat(retval,first_eval->keys[i]->s_val);
			 retval=safe_strcat(retval,"\n\t");
	                 retval=safe_strcat(retval,bl_oper_doc(ctx, bl_mk_list(1,first_eval->vals[i]))->s_val);
			 retval=safe_strcat(retval,"\n");
		 }
              }
            }
        }
     }

     return bl_mk_str(retval);
}

// LCOV_EXCL_STOP

// LCOV_EXCL_START

bl_val_t* bl_oper_dir (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_eval(ctx,bl_ctx_get(ctx,symname));
   if(symval->type != BL_VAL_TYPE_CTX) return NULL;
   bl_val_t* retval  = NULL;
   int i=0;
   for(i=0; i<symval->vals_count; i++) {
       if(symval->vals[i] != NULL) {
           if( strcmp(symval->keys[i]->s_val,"DOC") != 0)
     	       retval = bl_list_prepend(retval,bl_mk_str(symval->keys[i]->s_val));
       }
   }
   return retval;
}


bl_val_t* bl_oper_mksym  (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_eval(ctx,params);
   bl_val_t* first = bl_list_first(params);
   return bl_mk_symbol(first->s_val);
}

bl_val_t* bl_oper_ctxget (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_eval(ctx,bl_ctx_get(ctx,symname));
   if(symval->type != BL_VAL_TYPE_CTX) return NULL;
   bl_val_t* second = bl_list_second(params);
   if(second->type == BL_VAL_TYPE_CONS) second = bl_eval(ctx,second);
   return bl_ctx_get(symval,second);
}

bl_val_t* bl_oper_type (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_eval(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params;
   return bl_mk_type(bl_list_first(params)->type);
}

// LCOV_EXCL_STOP
