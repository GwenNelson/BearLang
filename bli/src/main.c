#include <stdio.h>
#include <stdlib.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>

#include <readline/readline.h>

int main(int argc, char** argv) {
    bl_init();
    printf("BearLang Version 0.WHATEVER\n\n"); // TODO: add versioning
    for(;;) {
        char* input_line = readline(">>> ");
        bl_ast_node_t* ast = bl_parse_sexp(input_line);
        bl_val_t*      expr = bl_read_ast(ast);
        printf("%s\n",bl_ser_sexp(expr));
    }
}
