#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

#define DECL_BUILTIN(builtin_name,builtin_symbol,builtin_docstr) bl_val_t* bl_oper_ ## builtin_name (bl_val_t* ctx, bl_val_t* params);

#define BUILTIN_X DECL_BUILTIN
#include <bearlang/builtins.inc>
#undef BUILTIN_X


