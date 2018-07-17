// C foreign function interface
//
// Essentially this module lets you import any arbitrary shared object and call functions
//
// To simplify this task, bindings to dlopen/dlsym are provided
//
// Usage instructions:
// (import cffi)
// (= cputs (cffi::dlsym "puts"))
// (= my_puts (cffi::func int cputs (char*))
//
// Now my_puts can be called like any normal function, passing a single parameter of type string and returning a number
//
// dlopen can also be used:
//
// imagine somelib.so contains void somefunc() { /* do stuff */ }
//
// (= somelib (cffi::dlopen "somelib.so"))
// (= somefunc (cffi::dlsym "myfunc" somelib))
// (= my_somefunc (cffi::func void somefunc ())
//
// current types supported: void, void*, char*, int

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/sexp.h>
#include <ffi.h>
#include <dlfcn.h>

static char* doc_str = "\n"
"NAME\n"
"	cffi - C foreign function interface for BearLang\n"
"\n"
"DESCRIPTION\n"
"	This module allows you to import arbitrary shared object files using the dlopen() interface and call functions\n"
"\n"
"EXAMPLE\n"
"\n"
"	This example code shows how to import and call the standard libc function puts:\n"
"\n"
"	  (import cffi)\n"
"	  (= cputs   (cffi::dlsym \"puts\"))\n"
"	  (= my_puts (cffi::func int cputs (char*))\n"
"\n"
"	Now my_puts can be called like any normal function, passing a single parameter of type string and returning a number\n"
"\n"
"	dlopen can also be used to import an arbitrary library:\n"
"\n"
"	  (= somelib     (cffi::dlopen \"somelib.so\"))\n"
"	  (= somefunc    (cffi::dlsym  \"somefunc\" somelib))\n"
"	  (= my_somefunc (cffi::func void somefunc ())\n"
"\n"
"	Now my_somefunc can be called, in this example there are no parameters accepted by the function\n"
;

typedef struct ffi_func_t {
     ffi_type *ret_type;
     bl_val_type_t *arg_types;
     size_t arg_count;
     void* native_fn;
     ffi_cif cif;
     ffi_type **ffi_arg_types;
} ffi_func_t;


static char* dlopen_doc_str = "\n"
"	(dlopen filename)\n"
"		Opens a shared object / dynamic library file and returns a pointer to it\n"
;
bl_val_t* dlopen_bearlang(bl_val_t* ctx, bl_val_t* params) {
     params            = bl_ctx_eval(ctx,params);
     bl_val_t* libname = bl_list_first(params);
     void* retval = dlopen(libname->s_val,RTLD_LOCAL);
     return bl_mk_ptr(retval);
}

static char* dlsym_doc_str = "\n"
"	(dlsym symbol library)\n"
"		Looks up the specified symbol in the provided library, library must be a pointer returned from dlopen\n"
;

bl_val_t* dlsym_bearlang(bl_val_t* ctx, bl_val_t* params) {
     params              = bl_ctx_eval(ctx,params);
     bl_val_t* funcname  = bl_list_first(params);
     bl_val_t* libhandle = bl_list_second(params);
     void* retval = NULL;
     if(libhandle == NULL) {
        retval = dlsym(RTLD_DEFAULT, funcname->s_val);
     } else {
        retval = dlsym(libhandle, funcname->s_val);
     }
     return bl_mk_ptr(retval);
}

bl_val_t* native_oper_ret_str(bl_val_t* ctx, bl_val_t* params) {

     return bl_mk_null();
}

bl_val_t* native_oper_ret_null(bl_val_t* ctx, bl_val_t* params) {
     return bl_mk_null();
}

bl_val_t* native_oper_ret_int(bl_val_t* ctx, bl_val_t* params) {
     ffi_func_t* func_def = (ffi_func_t*)params->custom_data;
     int i=0;
     void** args = GC_MALLOC(func_def->arg_count * sizeof(void*));
     bl_val_t* param = params;
     for(i=0; i<func_def->arg_count; i++) {
         args[i] = &(param->car->s_val); // TODO - implement other stuff
         param   = params->cdr;
     }
     int rc;

     char* ret_int = GC_MALLOC_ATOMIC(10);
     snprintf(ret_int,10,"%d", rc);
     ffi_call(&(func_def->cif), func_def->native_fn, &rc, args);
     return bl_mk_integer(ret_int);
}

bl_val_t* native_oper_ret_ptr(bl_val_t* ctx, bl_val_t* params) {
     ffi_func_t* func_def = (ffi_func_t*)params->custom_data;
     int i=0;
     void** args = GC_MALLOC(func_def->arg_count * sizeof(void*));
     bl_val_t* param = params;
     for(i=0; i<func_def->arg_count; i++) {
         args[i] = &(param->car->s_val); // TODO - implement other stuff
         param   = params->cdr;
     }
     void* ret_ptr;
     ffi_call(&(func_def->cif), func_def->native_fn, &ret_ptr, args);
     return bl_mk_ptr(ret_ptr);
}

static char* func_doc_str = "\n"
"	(func return-type function (param-type ...))\n"
"		Creates a callable BearLang function from the specified function pointer (as returned by dlsym)\n"
"		return-type must be the C type the function returns\n"
"		param-type is the type of the function's parameter at that position, optionally an empty list may be provided instead\n"
;
// this sets up the ffi_func_t struct and then returns a new native oper
bl_val_t* func_bearlang(bl_val_t* ctx, bl_val_t* params) {
     bl_val_t* retval_type = bl_list_first(params);
     bl_val_t* func        = bl_ctx_eval(ctx,bl_list_second(params));
     bl_val_t* arg_types   = bl_list_third(params);


     ffi_func_t* func_def = (ffi_func_t*)GC_MALLOC(sizeof(ffi_func_t));

     func_def->native_fn = func->c_ptr;

     bl_val_t* retval = NULL; // the oper to return



     size_t arg_len = (size_t)bl_list_len(arg_types);
     if(arg_len == 0) {
        func_def->arg_types = NULL;
        func_def->arg_count = 0;
     } else {
       func_def->arg_types = (bl_val_type_t*)GC_MALLOC_ATOMIC(sizeof(bl_val_type_t)*arg_len);
       func_def->ffi_arg_types = (ffi_type**)GC_MALLOC(sizeof(ffi_type*)*arg_len);
       func_def->arg_count = arg_len;
       bl_val_t* i = NULL;
       bl_val_t* L = arg_types;
       int n = 0;
       for(i=L; i != NULL; i=i->cdr) {
           if(L->car == bl_mk_symbol("char*")) {
              func_def->ffi_arg_types[n] = &ffi_type_pointer;
              func_def->arg_types[n] = BL_VAL_TYPE_STRING;
           } else if(L->car == bl_mk_symbol("void*")) {
              func_def->ffi_arg_types[n] = &ffi_type_pointer;
              func_def->arg_types[n] = BL_VAL_TYPE_CPTR;              
           } else if(L->car == bl_mk_symbol("int")) {
              func_def->ffi_arg_types[n] = &ffi_type_sint64;
              func_def->arg_types[n] = BL_VAL_TYPE_NUMBER;
           }
           n++;
       }

     }

     // TODO: flesh out all the types
     if(retval_type == bl_mk_symbol("int")) {
        func_def->ret_type = &ffi_type_sint;
        retval = bl_mk_native_oper(&native_oper_ret_int);
     } else if (retval_type == bl_mk_symbol("void")) {
        func_def->ret_type = &ffi_type_void;
        retval = bl_mk_native_oper(&native_oper_ret_null);
     } else if (retval_type == bl_mk_symbol("char*")) {
        func_def->ret_type = &ffi_type_pointer;
        retval = bl_mk_native_oper(&native_oper_ret_str);
     } else if (retval_type == bl_mk_symbol("void*")) {
        func_def->ret_type = &ffi_type_pointer;
        retval = bl_mk_native_oper(&native_oper_ret_ptr);
     }

     ffi_status r = ffi_prep_cif(&(func_def->cif), FFI_DEFAULT_ABI, 1, func_def->ret_type, func_def->ffi_arg_types);

     retval = bl_mk_native_oper(&native_oper_ret_int);
     retval->custom_data = (void*)func_def;
     return retval;
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("DOC"),   bl_mk_str(doc_str));

     bl_val_t* dlopen_oper = bl_mk_native_oper(&dlopen_bearlang);
     dlopen_oper->docstr   = bl_mk_str(dlopen_doc_str);
     bl_ctx_set(my_ctx,bl_mk_symbol("dlopen"),dlopen_oper);

     bl_val_t* dlsym_oper = bl_mk_native_oper(&dlsym_bearlang);
     dlsym_oper->docstr   = bl_mk_str(dlsym_doc_str);
     bl_ctx_set(my_ctx,bl_mk_symbol("dlsym"), dlsym_oper);

     bl_val_t* func_oper = bl_mk_native_oper(&func_bearlang);
     func_oper->docstr   = bl_mk_str(func_doc_str);
     bl_ctx_set(my_ctx,bl_mk_symbol("func"),  func_oper);
     return my_ctx;
}
