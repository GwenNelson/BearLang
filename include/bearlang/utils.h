#pragma once

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <stdbool.h>
#include <stdio.h>

bl_val_t* bl_mk_val(bl_val_type_t type);
bl_val_t* bl_mk_symbol(char* sym);
bl_val_t* bl_mk_number(uint64_t n);
bl_val_t* bl_mk_str(char* s);
bl_val_t* bl_mk_native_oper(void* func_ptr);
bl_val_t* bl_mk_bool(bool b);

bl_val_t* bl_eval_file(bl_val_t* ctx, char* filename, FILE* fd);
