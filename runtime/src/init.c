#include <bearlang/common.h>
#include <bearlang/sexp.h>

int bl_init() {
    GC_INIT();
    bl_init_parser();
    return 0;
}
