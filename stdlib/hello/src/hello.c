// This is a sample of how to compile an extension module, a hello world essentially

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>

bl_val_t* hello_bearlang(bl_val_t* ctx, bl_val_t* params) {
     printf("Hello from an external C module!\n");
}

// every module must contain this symbol
// it gets passed the context the module is being imported from and must return a new context populated with the relevant symbols
bl_val_t* bl_init_mod(bl_val_t* ctx) {
     bl_val_t* my_ctx = bl_ctx_new(ctx);
     bl_ctx_set(my_ctx,"hello",bl_mk_native_oper(&hello_bearlang));
     return my_ctx;
}
