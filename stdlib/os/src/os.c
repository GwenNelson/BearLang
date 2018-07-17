// This module provides misc OS stuff


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

static char* bl_module_name        = "os";
static char* bl_module_summary     = "Operating system interaction";
static char* bl_module_description = "The os module contains basic functions for interacting with the operating system.\n";

// (exit retval)
// exits to OS, if retval is not provided then it is set to 0
bl_val_t* bl_exit(bl_val_t* ctx, bl_val_t* params) {
     params = bl_eval_cons(ctx,params);
     int retval=0;
     if(bl_list_len(params)==1) {
        retval = bl_list_first(params)->fix_int;
     }
     exit(retval);
     return NULL; // this should never happen
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);

     bl_val_t* oper_exit = bl_mk_native_oper(&bl_exit);
     oper_exit->docstr = bl_mk_docstr("(exit retval)\n"
		                      "\texits to OS, if retval param is not provided then it is set to 0\n");
     bl_ctx_set(my_ctx,bl_mk_symbol("exit"), oper_exit);

     bl_ctx_set(my_ctx,bl_mk_symbol("*NAME*"),       bl_mk_str(bl_module_name));
     bl_ctx_set(my_ctx,bl_mk_symbol("*SUMMARY*"),    bl_mk_str(bl_module_summary));
     bl_ctx_set(my_ctx,bl_mk_symbol("*DESCRIPTION*"),bl_mk_str(bl_module_description));

     return my_ctx;
}
