#pragma once

#include <bearlang/types.h>
#include <bearlang/common.h>

void bl_init_parser(); // prepare the parser for use, called by bl_init()

bl_ast_node_t* bl_parse_sexp(char* sexp);      // turn a string into an AST
char*          bl_ser_ast(bl_ast_node_t* ast); // turn an AST back into a string

char*          bl_ser_sexp(bl_val_t* expr); // turn a pure expression into a string

bl_val_t* bl_read_ast(bl_ast_node_t*); // read an AST and turn it into a pure expression
