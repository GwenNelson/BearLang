// This module provides some tools for errors

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bearlang/error_tools.h>
#include <bearlang/list_ops.h>
#include <bearlang/sexp.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static char* bl_module_name         = "error";
static char* bl_module_summary      = "Tools for handling errors";
static char* bl_module_description  = "This module provides tools for handling error types that are not already part of the builtin operators";

// (getsym E)
// returns the symbol referenced by the error as a string
static char* getsym_doc_str = "\n"
"	(getsym error)\n"
"		Returns the symbol referenced by the provided error/exception instance\n"
;
bl_val_t* bl_getsym(bl_val_t* ctx, bl_val_t* params) {
     params = bl_eval(ctx,params);
     return bl_mk_str(params->err_val.symbol_name);
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("*NAME*"),       bl_mk_str(bl_module_name));
     bl_ctx_set(my_ctx,bl_mk_symbol("*SUMMARY*"),    bl_mk_str(bl_module_summary));
     bl_ctx_set(my_ctx,bl_mk_symbol("*DESCRIPTION*"),bl_mk_str(bl_module_description));

     bl_val_t* getsym_oper = bl_mk_native_oper(&bl_getsym);
     getsym_oper->docstr   = bl_mk_str(getsym_doc_str);
     bl_ctx_set(my_ctx,bl_mk_symbol("getsym"), getsym_oper);
     return my_ctx;
}
