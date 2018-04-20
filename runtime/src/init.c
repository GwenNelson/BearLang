#include <bearlang/common.h>
#include <bearlang/sexp.h>

int bl_init() {
//    GC_INIT();
    GC_enable_incremental();
    bl_init_parser();
    return 0;
}
