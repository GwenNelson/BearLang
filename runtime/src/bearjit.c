#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/bearjit.h>
#include <bearlang/list_ops.h>
#include <bearlang/string_ops.h>
#include <bearlang/sexp.h>

#include <jit/jit.h>
#include <stdio.h>

static jit_context_t bl_jit_context;
static jit_type_t    bl_jit_sig_native_oper;
static jit_type_t    bl_jit_sig_ctx_new;
static jit_type_t    bl_jit_sig_set_params;
static jit_type_t    bl_jit_intptr;

void bl_init_jit() {
     bl_jit_context = jit_context_create();

     jit_type_t native_oper_params[2];
     native_oper_params[0] = jit_type_void_ptr;
     native_oper_params[1] = jit_type_void_ptr;
     
     bl_jit_sig_native_oper = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, native_oper_params, 2, 1);

     jit_type_t ctx_new_params[1];
     ctx_new_params[0] = jit_type_void_ptr;
     
     bl_jit_sig_ctx_new = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, ctx_new_params, 1, 1);

     jit_type_t bl_jit_set_params[3];
     bl_jit_set_params[0] = jit_type_void_ptr;
     bl_jit_set_params[1] = jit_type_void_ptr;
     bl_jit_set_params[2] = jit_type_void_ptr;

     bl_jit_sig_set_params = jit_type_create_signature(jit_abi_cdecl, jit_type_void, bl_jit_set_params, 3, 1);

     bl_jit_intptr = jit_type_create_pointer(jit_type_sys_int,0);
}

// precompile optimisations
// does the following:
//   1 - scan through for all symbols in the function body
//   2 - copy their bindings from the parent context into the lexical closure unless they're param symbols or quoted
//
// this means that at compile time it's possible to link directly to the symbol value for most symbols

void precompile_opt_func_syms(bl_val_t* f, bl_val_t* expr);
void precompile_opt_func_syms(bl_val_t* f, bl_val_t* expr) {
     bl_val_t* L = expr;
     if(expr->type == BL_VAL_TYPE_CONS) {
	     for(;L != NULL; L = L->cdr) {
		 precompile_opt_func_syms(f,L->car);
	     }
     } else if(expr->type == BL_VAL_TYPE_SYMBOL) {
	// first check if it's in the params
        for(L=f->bl_funcargs_ptr; L != NULL; L = L->cdr) {
	    if(L->car == expr) return; // if it's in the args, skip it
	    // otherwise, it's time to copy
	    bl_ctx_set(f->lexical_closure,expr,bl_ctx_get(f->lexical_closure,expr)); // if it's already in the lexical closure, this won't overwrite with the parent
	}
     }

}

void* bl_jit_func(bl_val_t* f) {
      precompile_opt_func_syms(f,f->bl_func_ptr);
      jit_context_build_start(bl_jit_context);
      jit_function_t jitted_func;

      jitted_func = jit_function_create(bl_jit_context,bl_jit_sig_native_oper);

      // grab ctx and params
      jit_value_t func_ctx           = jit_value_get_param(jitted_func,0);
      jit_value_t func_called_params = jit_value_get_param(jitted_func,1);

      // eval params
      jit_value_t eval_args[2];
      eval_args[0] = func_ctx;
      eval_args[1] = func_called_params;

      func_called_params = jit_insn_call_native(jitted_func,"bl_ctx_eval", &bl_ctx_eval, bl_jit_sig_native_oper,eval_args,2,0);

      // setup closures
      jit_value_t lexical_closure;
      jit_value_t inner_closure;
  
      lexical_closure = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr,(jit_nint)f->lexical_closure);
      inner_closure   = jit_insn_call_native(jitted_func,"bl_ctx_new",&bl_ctx_new,bl_jit_sig_ctx_new, &lexical_closure,1,0);

      // setup pointers for symbols
      jit_value_t lexical_closure_symvals = jit_insn_load_relative(jitted_func,lexical_closure,offsetof(bl_val_t,vals),jit_type_void_ptr);
      jit_value_t inner_closure_symvals   = jit_insn_load_relative(jitted_func,inner_closure,offsetof(bl_val_t,vals),jit_type_void_ptr);

      // copy symbol values from lexical to inner closure
      jit_value_t inner_closure_symkeys   = jit_insn_load_relative(jitted_func,inner_closure,offsetof(bl_val_t,keys),jit_type_void_ptr);
      jit_value_t lexical_closure_symkeys = jit_insn_load_relative(jitted_func,lexical_closure,offsetof(bl_val_t,keys),jit_type_void_ptr);
      jit_insn_memcpy(jitted_func,inner_closure_symkeys,lexical_closure_symkeys,jit_value_create_nint_constant(jitted_func, jit_type_int, sizeof(bl_val_t*)*f->lexical_closure->vals_count));
      jit_insn_memcpy(jitted_func,inner_closure_symvals,lexical_closure_symvals,jit_value_create_nint_constant(jitted_func, jit_type_int, sizeof(bl_val_t*)*f->lexical_closure->vals_count));

      // update number of values
      jit_insn_store_relative(jitted_func, inner_closure,offsetof(bl_val_t,vals_count),jit_value_create_nint_constant(jitted_func, jit_type_int, f->lexical_closure->vals_count));

      // bind params
      if(bl_list_len(f->bl_operargs_ptr)>0) {
         jit_value_t set_params_args[3];
         set_params_args[0] = inner_closure;
         set_params_args[1] = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr, (jit_nint)f->bl_operargs_ptr);
         set_params_args[2] = func_called_params;

//         jit_insn_call_native(jitted_func,"bl_set_params", &bl_set_params, bl_jit_sig_set_params, set_params_args,3,0);
      }

      // iterate over the function body and call bl_ctx_eval on each
      jit_value_t func_ret = jit_value_create(jitted_func, jit_type_void_ptr);
      jit_value_t tmp;
      bl_val_t* L  = f->bl_func_ptr;
      eval_args[0] = inner_closure;

      jit_label_t end_of_func = jit_label_undefined;
      bl_val_t* E;
      for(L=f->bl_func_ptr; L!=NULL; L=L->cdr) {
          E = L->car;
	  switch(E->type) {
		  case BL_VAL_TYPE_SYMBOL: // if a naked symbol is in a function body, we just look up the value
			fprintf(stderr,"NAKED SYMBOL! %s\n", bl_ser_sexp(E));
 			  jit_insn_store(jitted_func,func_ret,jit_insn_load_elem(jitted_func,inner_closure_symvals,jit_value_create_nint_constant(jitted_func, jit_type_int,E->sym_id),jit_type_void_ptr));
		  break;
		  case BL_VAL_TYPE_CONS:
			if(E->car->type == BL_VAL_TYPE_SYMBOL) {

     				// if it's the first thing in an expression inside of a function body, it must be an oper, right?
			   jit_value_t oper_sym_val = jit_insn_load_elem(jitted_func,inner_closure_symvals,jit_value_create_nint_constant(jitted_func, jit_type_int, E->car->sym_id),jit_type_void_ptr);
			   jit_value_t oper_sym_code = jit_insn_load_relative(jitted_func,oper_sym_val,offsetof(bl_val_t,code_ptr),jit_type_void_ptr);

			   eval_args[1] = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr, (jit_nint)E->cdr); 

			   tmp = jit_insn_call_indirect(jitted_func, oper_sym_code, bl_jit_sig_native_oper,eval_args,2,0);
			   jit_insn_store(jitted_func,func_ret,tmp);
			} else {
			   fprintf(stderr,"WARNING - car is not symbol in %s\n", bl_ser_sexp(E));
			}	
		  break;
		  default:
			fprintf(stderr,"WARNING - don't know how to handle %s\n", bl_ser_sexp(E));
		  break;
	  }

/*	  if(E->type == BL_VAL_TYPE_CONS) { // we try to directly generate code here instead of calling out to bl_ctx_eval
		  bl_val_t* first;


		  if(E->car->type == BL_VAL_TYPE_SYMBOL) {
		     first = bl_ctx_get(f->lexical_closure,E->car);
		  } else {
		     first = E->car;
		  }
		  switch(first->type) {
                  	case BL_VAL_TYPE_OPER_NATIVE:
			     E->cdr->invoked_sym = E->car;
			     E->cdr->custom_data = first->custom_data;
                             eval_args[1] = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr, (jit_nint)E->cdr);
		             tmp = jit_insn_call_native(jitted_func,bl_ser_sexp(E),first->code_ptr,bl_jit_sig_native_oper,eval_args,2,0);
		             jit_insn_store(jitted_func,func_ret, tmp);
			break;
  			default:
			  eval_args[1] = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr, (jit_nint)E);
		          tmp = jit_insn_call_native(jitted_func,bl_ser_sexp(E),&bl_ctx_eval,bl_jit_sig_native_oper,eval_args,2,0);
		          jit_insn_store(jitted_func,func_ret, tmp);
			break;
		  }
	  } else {
		  eval_args[1] = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr, (jit_nint)E);
	          tmp = jit_insn_call_native(jitted_func,safe_strcat("eval ",bl_ser_sexp(E)),&bl_ctx_eval,bl_jit_sig_native_oper,eval_args,2,0);
	          jit_insn_store(jitted_func,func_ret, tmp);
	  }*/


	  // check if func_ret is an error, since the error type is the first field in bl_val_t we can simply cast it
          // enums are of type int, so we can treat func_ret as a pointer to type int
	  jit_value_t ret_type   = jit_insn_load_relative(jitted_func,func_ret,0,jit_type_sys_int);
          jit_value_t ret_is_err = jit_insn_eq(jitted_func,ret_type,jit_value_create_long_constant(jitted_func, jit_type_sys_int,BL_VAL_TYPE_ERROR));
	  jit_insn_branch_if(jitted_func,ret_is_err,&end_of_func);
      }

      jit_insn_label(jitted_func, &end_of_func);
      jit_insn_return(jitted_func, func_ret);

//      jit_dump_function(stdout,jitted_func,f->sym->s_val);
      jit_function_compile(jitted_func);
//      jit_dump_function(stdout,jitted_func,f->sym->s_val);

      jit_context_build_end(bl_jit_context);
      return jit_function_to_closure(jitted_func);

}
