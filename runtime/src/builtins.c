#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/builtins.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/types.h>
#include <bearlang/error_tools.h>
#include <bearlang/utils.h>

#include <glob.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <dlfcn.h>
#include <gmp.h>
#include <stdbool.h>
#include <libgen.h>

bl_val_t* bl_oper_parse(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_eval_cons(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params;

   bl_val_type_t expected_types[1] = {BL_VAL_TYPE_STRING};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,1);
   if(retval != NULL) return retval;

   return bl_parse_sexp(params->car->s_val);
}

bl_val_t* bl_oper_quote(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   if(bl_list_len(params)==1) return bl_list_first(params);
   return params;
}

// basic syntax:
// (try SOME_EXPR   ; optionally could be a (do) type expression
//      (catch SOME_ERR SOME_EXPR)
//      (catch SOME_OTHER_ERR SOME_EXPR)
//      (finally SOME_EXPR))
//
// the SOME_ERR types are defined in the default environment as ERR_* symbols
// e.g ERR_INSUFFICIENT_ARGS ERR_TOOMANY_ARGS etc
// while evaluating the catch blocks, *ERR* is set to the actual error
bl_val_t* bl_oper_try(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* try_expr    = bl_list_first(params);
   bl_val_t* try_result  = bl_ctx_eval(ctx, try_expr);
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
   bl_val_t* retval = bl_ctx_eval(ctx,catch_expr);
   if(finally_expr != NULL) retval = bl_ctx_eval(ctx,finally_expr);
   return retval;
}

bl_val_t* bl_oper_eval(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_ctx_eval(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params;
   if(bl_list_len(params)==1) return bl_ctx_eval(ctx,bl_list_first(params));
   return bl_ctx_eval(ctx,params);
}

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
	      retval_L->car = bl_ctx_eval(func->inner_closure,j->car);
              if(retval_L->car->type == BL_VAL_TYPE_ERROR) return retval_L->car;
	      retval_L->cdr = NULL;
	      retval = retval_L;
	   } else {
              retval_L->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
	      retval_L->cdr->car = bl_ctx_eval(func->inner_closure,j->car);
              if(retval_L->cdr->car->type == BL_VAL_TYPE_ERROR) return retval_L->cdr->car;
	      retval_L=retval_L->cdr;
	   }
       }
   }
   return retval;


}

bl_val_t* bl_oper_add(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* L = bl_eval_cons(ctx,params);
   if(L->type == BL_VAL_TYPE_ERROR) return L;
   bl_val_t* retval = bl_errif_invalid_len(L,1,BL_LONGEST_LIST);
   if(retval != NULL) return retval;
   bl_val_t* first = bl_ctx_eval(ctx,bl_list_first(params));

   if((first->type == BL_VAL_TYPE_CONS) && (bl_list_len(params)==1)) {
      params = first;
      first  = bl_list_first(params);
   }
   bl_val_t* x = NULL;
   switch(first->type) {
      case BL_VAL_TYPE_NUMBER:
           retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
	   retval->fix_int = 0;
      break;
      case BL_VAL_TYPE_STRING:
           retval = bl_mk_str("");
      break;
      case BL_VAL_TYPE_CONS:
           retval = NULL;
	   for(L=params; L != NULL; L=L->cdr) {
              x = bl_ctx_eval(ctx,L->car);
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
   char*     buf = NULL;

   L = params;

   for(L=params; L!= NULL; L=L->cdr) {
           x = bl_ctx_eval(ctx,L->car);
        switch(retval->type) {
            case BL_VAL_TYPE_NUMBER:
           	 retval->fix_int = retval->fix_int + x->fix_int;
	    break;
	    default:
                 s = bl_ser_naked_sexp(x);
		 c = strlen(s) + strlen(retval->s_val)+5;
                 retval->s_val = GC_REALLOC(retval->s_val,c);
                 snprintf(retval->s_val,c,"%s%s", retval->s_val,s);
	    break;
	}
   }
   return retval;
}

bl_val_t* bl_oper_sub(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_ctx_eval(ctx,params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;
     
   retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
   retval->fix_int = 0;

   bl_val_t* first  = bl_list_first(params);
   bl_val_t* second = bl_list_second(params);


//   mpz_sub(retval->i_val, first->i_val, second->i_val);
   retval->fix_int = first->fix_int - second->fix_int;
   return retval;
}

bl_val_t* bl_oper_mult(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_ctx_eval(ctx, params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
   retval->fix_int = 0;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

//   mpz_mul(retval->i_val, first->i_val, second->i_val);
   retval->fix_int = first->fix_int * second->fix_int;
   return retval;
}

bl_val_t* bl_oper_div(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_eval_cons(ctx, params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;


      retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
      retval->fix_int = 0;
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if(second->fix_int == 0) return bl_err_divzero();

//   mpz_tdiv_q(retval->i_val, first->i_val, second->i_val);
   retval->fix_int = first->fix_int / second->fix_int;
   return retval;
}

bl_val_t* bl_oper_mod(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   params = bl_ctx_eval(ctx,params);
/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;*/

      bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_NUMBER);
      retval->fix_int = 0;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

/*   mpz_init(retval->i_val);
   mpz_mod(retval->i_val, first->i_val, second->i_val);*/
   retval->fix_int = first->fix_int % second->fix_int;
   return retval;
}

bl_val_t* bl_oper_lt(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   params = bl_ctx_eval(ctx,params);
/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;*/

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if(first->fix_int < second->fix_int) return bl_mk_bool(true);
//   if(mpz_cmp(first->i_val,second->i_val)>=0) return bl_mk_bool(false);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_gt(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   params = bl_ctx_eval(ctx,params);
/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;*/


   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if(first->fix_int > second->fix_int) return bl_mk_bool(true);
  //   if(mpz_cmp(first->i_val,second->i_val)<0) return bl_mk_bool(false);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_set(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

/*   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_SYMBOL,BL_VAL_TYPE_ANY};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;*/
    
   bl_val_t* retval = bl_ctx_eval(ctx,bl_list_second(params));
   bl_val_t* name   = bl_list_first(params); // TODO: Handle the case where this isn't a symbol

   bl_ctx_set(ctx, name, retval);
   return retval;
}

//LCOV_EXCL_START
bl_val_t* bl_oper_print(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* i = bl_eval_cons(ctx,params);
   if(i->type == BL_VAL_TYPE_ERROR) return i;

   for(i=params; i!= NULL; i=i->cdr) {
     	printf("%s", bl_ser_naked_sexp(bl_ctx_eval(ctx,i->car)));

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

   bl_val_t* name = bl_list_first(params);
   bl_ctx_set(ctx, name, retval);
   return retval;
}

bl_val_t* bl_oper_eq(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));


   if(first->type == BL_VAL_TYPE_STRING && second->type == BL_VAL_TYPE_STRING) {
      if(strcmp(first->s_val, second->s_val)==0) {
         return bl_mk_bool(true);
      } else {
         return bl_mk_bool(false);
      }
   }
   if(first->fix_int == second->fix_int) {
 	return bl_mk_bool(true);
   } else {
	return bl_mk_bool(false);
   }
/*   if(mpz_cmp(first->i_val, second->i_val)==0) {
      return bl_mk_bool(true);
   } else {
      return bl_mk_bool(false);
   }*/
}

bl_val_t* bl_oper_and(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->b_val) && (second->b_val)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_not(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));

   if(first->b_val) return bl_mk_bool(false);
   return bl_mk_bool(true);
}

bl_val_t* bl_oper_or(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if(first->b_val) return bl_mk_bool(true);
   if(second->b_val) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_xor(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->b_val) ^ (second->b_val)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_first(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_ctx_eval(ctx,params);
   if(bl_list_len(params)==1) return bl_list_first(bl_list_first(params));
   return bl_list_first(params);
}

bl_val_t* bl_oper_second(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   params = bl_ctx_eval(ctx,params);
   if(bl_list_len(params)==1) return bl_list_second(bl_list_first(params));
   bl_val_t* retval = bl_list_second(params);
   return retval;
}

bl_val_t* bl_oper_third(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   if(bl_list_len(params)==1) params = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* retval = bl_list_third(params);
   return retval;
}

bl_val_t* bl_oper_rest(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE


   params = bl_ctx_eval(ctx,params);

   if(bl_list_len(params)==1) params = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* retval = bl_list_rest(params);
   return retval;
}

bl_val_t* bl_oper_include(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* filename = bl_ctx_eval(ctx,bl_list_first(params));

   FILE* fd         = fopen(filename->s_val,"r");
   bl_val_t* retval = bl_eval_file(ctx, filename->s_val, fd);
   fclose(fd);
   return retval;
}

typedef bl_val_t* (*mod_init_fn)(bl_val_t*);

bl_val_t* bl_oper_import(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
     bl_val_t* first = bl_list_first(params);
     bl_val_t* module_name = first;
     if(first->type == BL_VAL_TYPE_SYMBOL) {
        module_name = bl_ctx_get(ctx,first);
	if(module_name == NULL) module_name = first;
     }

     glob_t globbuf;

     bl_val_t* p_i;
     int i=0;
     bl_val_t* cur_path = bl_ctx_get(ctx,bl_mk_symbol("*PATH*"));
     char* pattern = GC_MALLOC_ATOMIC(4096);
     pattern[0] = NULL;
     snprintf(pattern,4096,"{");
     for(p_i=cur_path; p_i != NULL; p_i = p_i->cdr) {
         snprintf(pattern,4096,"%s%s,",pattern,p_i->car->s_val);
     }
     pattern[strlen(pattern)-1] = '}';
     snprintf(pattern,4096,"%s/{%s.bl,lib%s.so,lib%s.dylib,%s/module.bl,%s/lib%s.so,%s/lib%s.dylib}",pattern,module_name->s_val,
		                          module_name->s_val,module_name->s_val,module_name->s_val,module_name->s_val,module_name->s_val,
					  module_name->s_val,module_name->s_val);
     char* found_path = "";
     glob(pattern,GLOB_TILDE|GLOB_MARK|GLOB_BRACE,NULL,&globbuf);
         for(i=0; i<globbuf.gl_pathc; i++) {
             if(globbuf.gl_pathv[i][strlen(globbuf.gl_pathv[i])-1] != '/') { 
		     found_path = GC_MALLOC_ATOMIC(4096);
	             snprintf(found_path,4096,"%s",globbuf.gl_pathv[i]);
                     break;
             }
	 }
	 globfree(&globbuf);
     if(strlen(found_path)==0) {
        return bl_err_modnotfound(module_name->s_val);
     }

     char* fname = GC_MALLOC_ATOMIC(4096);
     snprintf(fname,4096,"%s", found_path);
     fname = basename(fname);
     char* fext = strrchr(fname, '.')+1;
     if(strcmp(fext,"dylib")==0 || strcmp(fext,"so")==0) { // dlopen time!
      bl_val_t* dylib_val = bl_mk_val(BL_VAL_TYPE_CPTR);
      dylib_val->c_ptr = dlopen(found_path,RTLD_NOW);
      if(!dylib_val->c_ptr) fprintf(stderr, "dlopen error: %s\n", dlerror());
      mod_init_fn mod_init = dlsym(dylib_val->c_ptr, "bl_mod_init");
      char* err = dlerror();
      if(err) fprintf(stderr,"dlsym failed: %s\n", err);
      bl_val_t* new_ctx = mod_init(ctx);
      bl_ctx_set(ctx, bl_mk_symbol(module_name->s_val), new_ctx);
      return new_ctx;
     } else { // .bl module time!
      FILE* fd = fopen(found_path,"r");
      bl_val_t* new_ctx = bl_ctx_new(ctx);
      bl_eval_file(new_ctx, fname, fd);
      fclose(fd);
      bl_ctx_set(ctx, bl_mk_symbol(module_name->s_val), new_ctx);
      return new_ctx;

     }
     return bl_mk_null();
}

// syntax
// (using stdio::fprintf) binds fprintf symbol in the current context
// TODO: implement (using stdio::*)
bl_val_t* bl_oper_using(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* sym = bl_list_first(params);
   char* s = sym->s_val;
   char* last = strrchr(s,':');
   bl_val_t* bindsym = NULL;
   if(last != NULL) {
      if(last != s) {
         last--;
         if(last[0]==':') {
            bindsym = bl_mk_symbol(last+2);
            if(strcmp(bindsym->s_val,"*")==0) {
               last[0] = '\0';
               bl_val_t* other_ctx = bl_ctx_get(ctx,bl_mk_symbol(s));
               if(other_ctx->type == BL_VAL_TYPE_ERROR) return other_ctx;
               int i=0;
               for(i=0; i<other_ctx->vals_count; i++) {
                   if(other_ctx->vals[i] != NULL) {
                      bl_ctx_set(ctx,other_ctx->keys[i],other_ctx->vals[i]);
                   }
               }
 	       return bl_mk_null();
            } else {
               bl_val_t* bindval = bl_ctx_get(ctx,sym);
               if(bindval->type == BL_VAL_TYPE_ERROR) return bindval;
               bl_ctx_set(ctx,bindsym,bindval);
            }
            
         }
      }
   }
   return bl_ctx_get(ctx,sym);
}

bl_val_t* bl_oper_isset(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* sym    = bl_list_first(params);
   bl_val_t* symval = bl_ctx_get(ctx, sym);
   if(symval == NULL) return bl_mk_bool(false);
   return bl_mk_bool(true);
}


bl_val_t* bl_oper_serexp (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_ctx_eval(ctx,params);
   if(params->type == BL_VAL_TYPE_CONS) {
      if(bl_list_len(params)==1) params = bl_list_first(params);
   }
   bl_val_t* retval = bl_mk_str(bl_ser_sexp(params));
   return retval;
}

bl_val_t* bl_oper_append(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* L = bl_ctx_eval(ctx,bl_list_first(params));
   return bl_list_append(L, bl_ctx_eval(ctx,bl_list_second(params)));
}

bl_val_t* bl_oper_prepend(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

   bl_val_t* L = bl_ctx_eval(ctx,bl_list_first(params));
   return bl_list_prepend(L, bl_ctx_eval(ctx,bl_list_second(params)));
}

bl_val_t* bl_oper_reverse(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE


   params = bl_ctx_eval(ctx,params);

   if(bl_list_len(params)==1) params = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* retval = bl_list_reverse(params);
   return retval;
}

bool has_init_integer_one = false;
static mpz_t integer_one;

bl_val_t* bl_oper_inc(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   if(!has_init_integer_one) mpz_init_set_ui(integer_one,1);
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_get(ctx,symname);
   symval->fix_int++;
//   mpz_add(symval->i_val, symval->i_val, integer_one);
   return symval;
}

bl_val_t* bl_oper_dec(bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE

//   if(!has_init_integer_one) mpz_init_set_ui(integer_one,1);
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_get(ctx,symname);
   symval->fix_int--;
   //   mpz_sub(symval->i_val, symval->i_val, integer_one);
   return symval;
}


bl_val_t* bl_oper_doc (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_get(ctx,symname);
   if(symval->docstr==NULL) {
      if(symval->type == BL_VAL_TYPE_CTX) {
         symval->docstr = bl_ctx_get(symval,bl_mk_symbol("*DOC*"));
      } else {
         return bl_mk_str("No documentation!");
      }
   }
   return bl_mk_str(symval->docstr->s_val);
}

bl_val_t* bl_oper_dir (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_eval(ctx,bl_ctx_get(ctx,symname));
   if(symval->type != BL_VAL_TYPE_CTX) return NULL;
   bl_val_t* retval  = NULL;
   int i=0;
   for(i=0; i<symval->vals_count; i++) {
       if(symval->vals[i] != NULL) {
          retval = bl_list_prepend(retval,symval->keys[i]);
       }
   }
   return retval;
}

bl_val_t* bl_oper_mksym  (bl_val_t* ctx, bl_val_t* params) { // LCOV_EXCL_LINE
   params = bl_ctx_eval(ctx,params);
   bl_val_t* first = bl_list_first(params);
   return bl_mk_symbol(first->s_val);
}
