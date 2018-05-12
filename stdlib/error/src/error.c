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

// (getsym E)
// returns the symbol referenced by the error as a string
bl_val_t* bl_getsym(bl_val_t* ctx, bl_val_t* params) {
     params = bl_ctx_eval(ctx,params);
     return bl_mk_str(params->err_val.symbol_name);
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("getsym"), bl_mk_native_oper(&bl_getsym));
     return my_ctx;
}
