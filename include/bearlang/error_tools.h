#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <bearlang/types.h>

// util function to see if a list is a valid length
bool bl_is_valid_len(bl_val_t* L, uint64_t min, uint64_t max);

// util function to do above, but for functions, convenience wrapper
// basically checks to ensure same number of arguments
bool bl_is_valid_funcarg_len(bl_val_t* f, bl_val_t* L);

// util function to generate an error if the list length doesn't match
// if it matches ok, returns NULL - intended for use at start of native/builtin operator C functions
bl_val_t* bl_errif_invalid_len(bl_val_t* L, uint64_t min, uint64_t max);

// convert an error type into a human-readable message
// designed for interactive/realtime use
char* bl_errmsg(bl_val_t* E);
