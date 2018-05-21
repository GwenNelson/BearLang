#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <Python.h>

bl_val_t* py_run_str(bl_val_t* ctx, bl_val_t* params) {
     params = bl_ctx_eval(ctx,params);
     bl_val_t* s = bl_list_first(params);
     PyRun_SimpleString(s->s_val);
     return bl_mk_null();
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     Py_Initialize();
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("py_run_str"),bl_mk_native_oper(&py_run_str));
     return my_ctx;
}
