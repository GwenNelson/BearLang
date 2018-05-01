#include <bearlang/common.h>
#include <bearlang/sexp.h>

#include <gc.h>
#include <gmp.h>

void* gmp_malloc(size_t size) {
      return GC_MALLOC(size);
}

void* gmp_realloc(void* ptr, size_t old_size, size_t new_size) {
      return GC_REALLOC(ptr, new_size);
}

void gmp_free(void* ptr, size_t size) {
     GC_FREE(ptr);
}

int bl_init() {
    GC_init();

    mp_set_memory_functions(&gmp_malloc, &gmp_realloc, &gmp_free);
    return 0;
}
