#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/bearjit.h>

#include <jit/jit.h>

static jit_context_t bl_jit_context;
static jit_type_t    bl_jit_sig_native_oper;
static jit_type_t    bl_jit_sig_ctx_new;
static jit_type_t    bl_jit_sig_set_params;

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
}

void* bl_jit_func(bl_val_t* f) {
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
  
      lexical_closure = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr,f->lexical_closure);
      inner_closure   = jit_insn_call_native(jitted_func,"bl_ctx_new",&bl_ctx_new,bl_jit_sig_ctx_new, &lexical_closure,1,0);

      // bind params
      if(bl_list_len(f->bl_operargs_ptr)>0) {
         jit_value_t set_params_args[3];
         set_params_args[0] = inner_closure;
         set_params_args[1] = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr, f->bl_operargs_ptr);
         set_params_args[2] = func_called_params;

         jit_insn_call_native(jitted_func,"bl_set_params", &bl_set_params, bl_jit_sig_set_params, set_params_args,3,0);
      }

      // iterate over the function body and call bl_ctx_eval on each
      jit_value_t func_ret;
      bl_val_t* L  = f->bl_func_ptr;
      eval_args[0] = inner_closure;

      for(L=f->bl_func_ptr; L!=NULL; L=L->cdr) {
          eval_args[1] = jit_value_create_nint_constant(jitted_func, jit_type_void_ptr, L->car);
          func_ret = jit_insn_call_native(jitted_func,"bl_ctx_eval",&bl_ctx_eval,bl_jit_sig_native_oper,eval_args,2,0);
      }
      jit_insn_return(jitted_func, func_ret);
      jit_function_compile(jitted_func);

      jit_context_build_end(bl_jit_context);
      return jit_function_to_closure(jitted_func);

}
