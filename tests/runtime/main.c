#include <stdio.h>
#include <stdlib.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>

#define TEST(desc,f) fprintf(stderr,"Testing %s:\t",desc); if(f()==0) { passed_tests++; fprintf(stderr,"PASS\n");} else { failed_tests++; fprintf(stderr,"FAIL\n");}; total_tests++;

#define ASSERT(desc,cond) if(! (cond)) { fprintf(stderr,"Assert %s failed\t",desc); return 1;}

int test_simple_sexp_list() {
    // this is a VERY basic test, we just want to make sure we get a correct list
    char* test_list = "(+ 1 2)";

    // first we parse to an AST
    bl_ast_node_t* ast = bl_parse_sexp(test_list);

    // now we check the AST looks correct - it should consist of an expression containing the + symbol and 2 integers (1 and 2)
    bl_ast_node_t**  children     = ast->children;
    int              child_count  = ast->child_count;

    ASSERT("child_count==3",child_count == 3)

    ASSERT("node_type==BL_VAL_TYPE_LIST",ast->node_type == BL_VAL_TYPE_LIST)

    ASSERT("children[0] is symbol",ast->children[0]->node_type == BL_VAL_TYPE_SYMBOL)
    ASSERT("children[1] is int",   ast->children[1]->node_type == BL_VAL_TYPE_NUMBER)
    ASSERT("children[2] is int",   ast->children[2]->node_type == BL_VAL_TYPE_NUMBER)

    ASSERT("children[1] has value 1", ast->children[1]->node_val.i_val == 1)
    ASSERT("children[2] has value 2", ast->children[2]->node_val.i_val == 2)


    return 0;
}


int main(int argc, char** argv) {
    int passed_tests = 0;
    int failed_tests = 0;
    int total_tests  = 0;

    bl_init();

    TEST("Simple s-expression parse to list",test_simple_sexp_list)

    fprintf(stderr,"Ran %d tests, %d passed, %d failed\n", total_tests, passed_tests, failed_tests);


    if(failed_tests > 0) {
       return 1;
    } else { 
       return 0;
    }
}
