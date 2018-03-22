#include <bearlang/common.h>
#include <bearlang/sexp.h>

#include <stdio.h>

bl_ast_node_t* bl_parse_sexp(char* sexp) {
      return (bl_ast_node_t*)GC_MALLOC(sizeof(bl_ast_node_t));
}

char* bl_ser_sexp(bl_ast_node_t* ast) {
      char* retval = (char*)GC_MALLOC(sizeof(char)*5);
      snprintf(retval, 5, "None");
      return retval;
}
