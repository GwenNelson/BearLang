#define GC_DEBUG 1
#include <bearlang/common.h>
#include <bearlang/sexp.h>

int bl_init() {
//    GC_dont_gc=1;
    	GC_INIT();
    bl_init_parser();
    return 0;
}
