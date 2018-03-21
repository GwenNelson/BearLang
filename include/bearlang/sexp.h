#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

bl_ast_t* bl_parse_sexp(char* sexp);
char*     bl_ser_sexp(bl_ast_t* ast);

