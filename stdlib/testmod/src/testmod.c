/*
 * This module is used by the test suite and should probably not be used elsewhere.
 *
 *
 */
#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>

char* test_str = "TEST STRING";

bl_val_t* getcptr_bearlang(bl_val_t* ctx, bl_val_t* params) {
     return bl_mk_ptr(test_str);
}

bl_val_t* bl_mod_init(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,bl_mk_symbol("get_cptr"),bl_mk_native_oper(&getcptr_bearlang));
     return my_ctx;
}
