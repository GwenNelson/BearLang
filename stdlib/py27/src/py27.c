#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <Python.h>

PyObject *m;

bl_val_t* py_get(bl_val_t* ctx, bl_val_t* sym) {
     char* sym_name  = sym->s_val;
     PyObject* pyobj = PyObject_GetAttrString(m,sym_name);
     if(pyobj==NULL) return NULL;
     return bl_mk_ptr(pyobj);
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     Py_Initialize();
     m = PyImport_AddModule("__main__");
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     my_ctx->ctx_get  = &py_get;
     return my_ctx;
}
