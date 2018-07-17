// This is a sample of how to compile an extension module, a hello world essentially
//
// To use this module, do this:
//   (import hello)
//   (hello::hello)
//
#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>

static char* bl_module_name        = "hello";
static char* bl_module_summary     = "Example C code module";
static char* bl_module_description = "This module is provided as an example and template of how to implement a BearLang module in C";
static char* bl_module_example     = ""
"  (import hello)\n"
"  (hello::hello)\n"
;


char* hello_doc_str = "\n"
"	(hello)\n"
"		An example C function with no parameters, says hello and returns a string\n"
;
bl_val_t* hello_bearlang(bl_val_t* ctx, bl_val_t* params) {
     printf("Hello from an external C module!\n");
     return bl_mk_str("This is the return value!");
}

// every module must contain this symbol
// it gets passed the context the module is being imported from and must return a new context populated with the relevant symbols
bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);

     // setup the documentation
     bl_ctx_set(my_ctx,bl_mk_symbol("*NAME*"),       bl_mk_str(bl_module_name));
     bl_ctx_set(my_ctx,bl_mk_symbol("*SUMMARY*"),    bl_mk_str(bl_module_summary));
     bl_ctx_set(my_ctx,bl_mk_symbol("*DESCRIPTION*"),bl_mk_str(bl_module_description));
     bl_ctx_set(my_ctx,bl_mk_symbol("*EXAMPLE*"),    bl_mk_str(bl_module_example));


     // add our function and it's docstring
     bl_val_t* hello_oper = bl_mk_native_oper(&hello_bearlang);
     hello_oper->docstr   = bl_mk_str(hello_doc_str);
     bl_ctx_set(my_ctx,bl_mk_symbol("hello"),hello_oper);
     return my_ctx;
}
