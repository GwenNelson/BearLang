#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/sexp.h>
#include <Python.h>

bl_val_t* py_invoke_callable(bl_val_t* ctx, bl_val_t* params) {
     void* custom_data  = params->custom_data;
     PyObject* callable = (PyObject*)custom_data;

     PyObject* args     = PyTuple_New(bl_list_len(params));

     bl_val_t* L = params;
     PyObject* v=NULL;
     int i=0;
     for(; L != NULL; L=L->cdr) {
         switch(L->car->type) {
            case BL_VAL_TYPE_NUMBER:
		 v = PyInt_FromString(bl_ser_sexp(L->car), NULL,10);
	    break;
	    case BL_VAL_TYPE_STRING:
	         v = PyString_FromString(L->car->s_val);
	    break;
	    default:
                 v = PyString_FromString(bl_ser_sexp(L->car));
	    break;
         }
	     PyTuple_SetItem(args,i,v);

         i++;
     }

     PyObject* retval   = PyEval_CallObject(callable,args);
     return bl_mk_null();
}

bl_val_t* py_get(bl_val_t* ctx, bl_val_t* sym) {
     PyGILState_STATE gstate;
     gstate = PyGILState_Ensure();

   	PyObject* m = (PyObject*)ctx->custom_ctx_data;
     char* sym_name  = sym->s_val;
     PyObject* pyobj = PyObject_GetAttrString(m,sym_name);
     if(pyobj==NULL) { 
        PyErr_Print();
PyGILState_Release(gstate);
	return NULL;
     }
     if(PyString_Check(pyobj)) {
        char* py_str = PyString_AsString(pyobj);
PyGILState_Release(gstate);
	return bl_mk_str(py_str);
     } 
     if(PyCallable_Check(pyobj)) {
        bl_val_t* retval = bl_mk_native_oper(&py_invoke_callable);
	retval->custom_data = (void*)pyobj;
PyGILState_Release(gstate);
	return retval;
     }
PyGILState_Release(gstate);
     return bl_mk_ptr(pyobj);
}

bl_val_t* py_ctx(PyObject* m) {
     bl_val_t* retval = bl_ctx_new(NULL);
     retval->custom_ctx_data = (void*)m;
     retval->ctx_get         = &py_get;
     return retval;
}

bl_val_t* py_import(bl_val_t* ctx, bl_val_t* params) {
     bl_val_t* first = bl_list_first(params);
     char* py_name = first->s_val;
     PyObject* mod = PyImport_Import(PyString_FromString(py_name));
     if (!mod) {
        PyErr_Print();
     }
     return py_ctx(mod);
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     Py_Initialize();
     PyObject* sysPath = PySys_GetObject((char*)"path");

     PyList_Append(sysPath, PyString_FromString("."));
     bl_val_t* my_ctx       = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx, bl_mk_symbol("import"), bl_mk_native_oper(&py_import));
     return my_ctx;
}
