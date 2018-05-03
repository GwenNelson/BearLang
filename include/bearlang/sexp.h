#pragma once

#include <stdio.h>

#include <bearlang/types.h>
#include <bearlang/common.h>



bl_val_t* bl_parse_sexp(char* sexp);      // turn a string into an AST
bl_val_t* bl_parse_file(char* filename, FILE* fd);        // read from a file and turn it into an AST

char*          bl_ser_ast(bl_ast_node_t* ast); // turn an AST back into a string

char*          bl_ser_sexp(bl_val_t* expr); // turn a pure expression into a string
char*          bl_ser_naked_sexp(bl_val_t* expr); // same as above, but strip off the quotes in strings

