#include <bearlang/common.h>
#include <bearlang/sexp.h>
#include <bearlang/bearjit.h>
#include <gc.h>
#include <gmp.h>

//LCOV_EXCL_START
void* gmp_malloc(size_t size) {
      return GC_malloc(size);
}

void* gmp_realloc(void* ptr, size_t old_size, size_t new_size) {
      return GC_realloc(ptr, new_size);
}

void gmp_free(void* ptr, size_t size) {
//     GC_free(ptr);
}
//LCOV_EXCL_STOP
int bl_init() { // LCOV_EXCL_LINE
    GC_INIT();
    mp_set_memory_functions(&gmp_malloc, &gmp_realloc, &gmp_free);
    bl_init_jit();
    return 0;
}
