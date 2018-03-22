#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

void bl_init_parser();

bl_ast_node_t* bl_parse_sexp(char* sexp);
char*          bl_ser_sexp(bl_ast_node_t* ast);

