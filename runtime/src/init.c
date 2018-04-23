#include <bearlang/common.h>
#include <bearlang/sexp.h>

int bl_init() {
    GC_time_limit=1;
    GC_full_freq=90;
    GC_free_space_divisor=2;
    GC_init();
    return 0;
}
