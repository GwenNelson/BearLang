#include <bearlang/common.h>
#include <bearlang/list_ops.h>
#include <bearlang/builtins.h>
#include <bearlang/sexp.h>
#include <bearlang/ctx.h>
#include <bearlang/types.h>
#include <bearlang/error_tools.h>
#include <bearlang/utils.h>

#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <dlfcn.h>
#include <gmp.h>
#include <stdbool.h>

bl_val_t* bl_oper_while(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* cond = bl_list_first(params);
   bl_val_t* body = bl_list_rest(params);
   while(bl_ctx_eval(ctx,cond)->b_val) {
      bl_val_t* retval = bl_ctx_eval(ctx,body);
      if(retval->type == BL_VAL_TYPE_ERROR) return retval;
   }
   return bl_mk_null();
}

bl_val_t* bl_oper_map(bl_val_t* ctx, bl_val_t* params) {
   params = bl_eval_cons(ctx,params,true);

   bl_val_t* func = bl_list_first(params);
   bl_val_t* L    = bl_list_second(params);

   bl_val_t* func_expr = NULL;
   bl_val_t* retval = NULL;

   while(L != NULL) {
     if(L->car != NULL) {
        func_expr = NULL;
	func_expr = bl_list_append(func_expr,func);
	func_expr = bl_list_append(func_expr,L->car);
        retval = bl_list_append(retval,bl_ctx_eval(ctx,func_expr));
     }
     L = L->cdr;
   }
   return retval;

}

bl_val_t* bl_oper_add(bl_val_t* ctx, bl_val_t* params) {

   bl_val_t* L = bl_eval_cons(ctx,params,true);
   bl_val_t* retval = bl_errif_invalid_len(L,1,BL_LONGEST_LIST);
   if(retval != NULL) return retval;

   bl_val_t* first = bl_ctx_eval(ctx,bl_list_first(params));

   if((first->type == BL_VAL_TYPE_CONS) && (bl_list_len(params)==1)) {
      params = first;
      first  = bl_list_first(params);
   }

   if(first->type == BL_VAL_TYPE_NUMBER) {
      retval = bl_mk_integer("0");
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
             mpz_add(retval->i_val, retval->i_val, x->i_val);
	   } else {
             s = bl_ser_naked_sexp(x);
	     c = strlen(s) + strlen(retval->s_val)+5;
             retval->s_val = GC_REALLOC(retval->s_val,c);
             snprintf(retval->s_val,c,"%s%s", retval->s_val,s);
	   }
	}
	L = L->cdr;
   }
   if(L->car != NULL) {
        x = bl_ctx_eval(ctx,L->car);
	   if(retval->type == BL_VAL_TYPE_NUMBER) {
             mpz_add(retval->i_val, retval->i_val, x->i_val);
	   } else {
             s = bl_ser_naked_sexp(x);
	     c = strlen(s) + strlen(retval->s_val)+5;
             retval->s_val = GC_REALLOC(retval->s_val,c);
             snprintf(retval->s_val,c,"%s%s", retval->s_val,s);

	   }
   }
   return retval;
}

bl_val_t* bl_oper_sub(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx,params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;
     
   retval = bl_mk_integer("0");

   bl_val_t* first  = bl_list_first(params);
   bl_val_t* second = bl_list_second(params);


   mpz_sub(retval->i_val, first->i_val, second->i_val);
   return retval;
}

bl_val_t* bl_oper_mult(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx, params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_mk_integer("0");

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   mpz_mul(retval->i_val, first->i_val, second->i_val);
   return retval;
}

bl_val_t* bl_oper_div(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx, params);
     	bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;

    retval = bl_mk_integer("0");
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   mpz_tdiv_q(retval->i_val, first->i_val, second->i_val);
   return retval;
}

bl_val_t* bl_oper_mod(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx,params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;

   retval = bl_mk_integer("0");

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   mpz_mod(retval->i_val, first->i_val, second->i_val);
   return retval;
}

bl_val_t* bl_oper_lt(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx,params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if(mpz_cmp(first->i_val,second->i_val)>=0) return bl_mk_bool(false);
   return bl_mk_bool(true);
}

bl_val_t* bl_oper_gt(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx,params);
   bl_val_type_t expected_types[2] = {BL_VAL_TYPE_NUMBER,BL_VAL_TYPE_NUMBER};
   bl_val_t* retval = bl_errif_invalid_fixed_args(params,expected_types,2);
   if(retval != NULL) return retval;

   retval = bl_errif_invalid_len(params,2,2);
   if(retval != NULL) return retval;

   retval = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type     = BL_VAL_TYPE_NUMBER;

   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if(mpz_cmp(first->i_val,second->i_val)<0) return bl_mk_bool(false);
   return bl_mk_bool(true);
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
   if(i->type == BL_VAL_TYPE_ERROR) return i;
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
   retval->sym = bl_mk_symbol("anonymous-lambda");
   return retval;
}

bl_val_t* bl_oper_fun(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* retval        = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
   retval->type            = BL_VAL_TYPE_FUNC_BL;
   retval->bl_funcargs_ptr = bl_list_second(params);
   retval->bl_func_ptr     = bl_list_rest(bl_list_rest(params)); // the rest of the rest is better than the rest
   retval->lexical_closure = ctx;
   retval->sym = bl_list_first(params);

   bl_ctx_set(ctx, retval->sym->s_val, retval);
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


   if(first->type == BL_VAL_TYPE_STRING && second->type == BL_VAL_TYPE_STRING) {
      if(strcmp(first->s_val, second->s_val)==0) {
         return bl_mk_bool(true);
      } else {
         return bl_mk_bool(false);
      }
   }
   if(mpz_cmp(first->i_val, second->i_val)==0) {
      return bl_mk_bool(true);
   } else {
      return bl_mk_bool(false);
   }
}

bl_val_t* bl_oper_and(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->b_val) && (second->b_val)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_not(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));

   if(first->b_val) return bl_mk_bool(false);
   return bl_mk_bool(true);
}

bl_val_t* bl_oper_or(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->b_val) || (second->b_val)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_xor(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* first  = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* second = bl_ctx_eval(ctx,bl_list_second(params));

   if((first->b_val) ^ (second->b_val)) return bl_mk_bool(true);
   return bl_mk_bool(false);
}

bl_val_t* bl_oper_first(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx,params);
   if(bl_list_len(params)==1) return bl_list_first(bl_list_first(params));
   return bl_list_first(params);
}

bl_val_t* bl_oper_second(bl_val_t* ctx, bl_val_t* params) {
   params = bl_ctx_eval(ctx,params);
   if(bl_list_len(params)==1) return bl_list_second(bl_list_first(params));
   bl_val_t* retval = bl_list_second(params);
   return retval;
}

bl_val_t* bl_oper_third(bl_val_t* ctx, bl_val_t* params) {
   if(bl_list_len(params)==1) params = bl_ctx_eval(ctx,bl_list_first(params));
   bl_val_t* retval = bl_list_third(params);
   return retval;
}

bl_val_t* bl_oper_rest(bl_val_t* ctx, bl_val_t* params) {

   params = bl_ctx_eval(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params;

   if(bl_list_len(params)==1) params = bl_ctx_eval(ctx,bl_list_first(params));
   if(params->type == BL_VAL_TYPE_NULL) return bl_mk_null();
   if(bl_list_len(params)==0) return bl_mk_null();
   bl_val_t* retval = bl_list_rest(params);
   if(retval==NULL) retval = bl_mk_null();
   return retval;
}

bl_val_t* bl_oper_include(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* filename = bl_ctx_eval(ctx,bl_list_first(params));

   FILE* fd         = fopen(filename->s_val,"r");
   bl_val_t* retval = bl_eval_file(ctx, filename->s_val, fd);
   fclose(fd);
   return retval;
}

typedef bl_val_t* (*mod_init_fn)(bl_val_t*);

bl_val_t* bl_oper_import(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* module_name = bl_ctx_eval(ctx,bl_list_first(params));
   // first try to find a BearLang module
   char filename[1024];
   snprintf(filename,1024,"%s.bl",module_name->s_val);

   if(access(filename, R_OK) != -1) {
      // it exists, so let's parse it
      FILE* fd = fopen(filename,"r");
      bl_val_t* new_ctx = bl_ctx_new(ctx);
      bl_eval_file(new_ctx, filename, fd);
      fclose(fd);
      bl_ctx_set(ctx, module_name->s_val, new_ctx);
      return new_ctx;
   } else {
      snprintf(filename,1024,"%s.so",module_name->s_val);
      if(access(filename, R_OK) == -1) {
         snprintf(filename,1024,"%s.dylib", module_name->s_val);
      }
      if(access(filename, R_OK) == -1) {
         // TODO - throw error here
	 return bl_mk_null();
      }
      bl_val_t* dylib_val = bl_mk_val(BL_VAL_TYPE_CPTR);
      dylib_val->c_ptr = dlopen(filename,RTLD_LAZY);
      if(!dylib_val->c_ptr) fprintf(stderr, "dlopen error: %s\n", dlerror());
      mod_init_fn mod_init = dlsym(dylib_val->c_ptr, "bl_mod_init");
      char* err = dlerror();
      if(err) fprintf(stderr,"dlsym failed: %s\n", err);
      bl_val_t* new_ctx = mod_init(ctx);
      bl_ctx_set(ctx, module_name->s_val, new_ctx);
      return new_ctx;
   }
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

bl_val_t* bl_oper_append(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* L = bl_ctx_eval(ctx,bl_list_first(params));
   return bl_list_append(L, bl_ctx_eval(ctx,bl_list_second(params)));
}

bl_val_t* bl_oper_prepend(bl_val_t* ctx, bl_val_t* params) {
   bl_val_t* L = bl_ctx_eval(ctx,bl_list_first(params));
   return bl_list_prepend(L, bl_ctx_eval(ctx,bl_list_second(params)));
}

bl_val_t* bl_oper_reverse(bl_val_t* ctx, bl_val_t* params) {

   params = bl_ctx_eval(ctx,params);
   if(params->type == BL_VAL_TYPE_ERROR) return params;

   if(bl_list_len(params)==1) params = bl_ctx_eval(ctx,bl_list_first(params));
   if(params->type == BL_VAL_TYPE_NULL) return bl_mk_null();
   if(bl_list_len(params)==0) return bl_mk_null();
   bl_val_t* retval = bl_list_reverse(params);
   if(retval==NULL) retval = bl_mk_null();
   return retval;
}

bool has_init_integer_one = false;
static mpz_t integer_one;

bl_val_t* bl_oper_inc(bl_val_t* ctx, bl_val_t* params) {
   if(!has_init_integer_one) mpz_init_set_ui(integer_one,1);
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_get(ctx,symname->s_val);
   mpz_add(symval->i_val, symval->i_val, integer_one);
   return symval;
}

bl_val_t* bl_oper_dec(bl_val_t* ctx, bl_val_t* params) {
   if(!has_init_integer_one) mpz_init_set_ui(integer_one,1);
   bl_val_t* symname = bl_list_first(params);
   bl_val_t* symval  = bl_ctx_get(ctx,symname->s_val);
   mpz_sub(symval->i_val, symval->i_val, integer_one);
   return symval;
}
