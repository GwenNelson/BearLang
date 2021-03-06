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

// util function to generate an error if the arguments provided don't match expected data types
// also returns an error if the number is incorrect, like bl_errif_invalid_len above - for fixed argument lengths
// the expected_types argument should be an array of bl_val_t of type BL_VAL_TYPE_TYPE
bl_val_t* bl_errif_invalid_fixed_args(bl_val_t* params, const bl_val_type_t* expected_types, uint64_t args_len);

// convert an error type into a human-readable message
// designed for interactive/realtime use
char* bl_errmsg(bl_val_t* E);

// return a symbol not found error for the specified symbol
bl_val_t* bl_err_symnotfound(char* sym);

// return a module not found error for the specified module
bl_val_t* bl_err_modnotfound(char* mod);

// return divide by zero error
bl_val_t* bl_err_divzero();

// return invalid first argument error if the first argument doesn't match, assumes rest of the args are ANY
bl_val_t* bl_errif_invalid_firstarg(bl_val_t* params, const bl_val_type_t expected_type);

// construct an error value
bl_val_t* bl_mk_err(bl_err_type_t T);

char* bl_ser_type(bl_val_type_t t);
