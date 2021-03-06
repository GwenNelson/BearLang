#include <bearlang/bearlang.h>

bl_val_t* bl_main(bl_val_t* ctx, bl_val_t* params);

int main(int argc, char** argv) {
    bl_init();
    bl_val_t* ctx = bl_ctx_new_std();
    bl_ctx_set(ctx,bl_mk_symbol("*MAINFILE*"),bl_mk_str(argv[0]));
    bl_ctx_set(ctx,bl_mk_symbol("*FILENAME*"),bl_mk_str(argv[0]));
    bl_main(ctx,NULL);
}
