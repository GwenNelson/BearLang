#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>

#define TEST(desc,f) fprintf(stderr,"Testing %s: \t\t ",desc); if(f()==0) { passed_tests++; fprintf(stderr,"PASS\n");} else { failed_tests++; fprintf(stderr,"FAIL\n");}; total_tests++;

#define ASSERT(desc,cond) if(! (cond)) { fprintf(stderr,"Assert %s failed\t",desc); return 1;}

int test_sexp_parse_list() {
    // this is a VERY basic test, we just want to make sure we get a correct list
    char* test_list = "(+ 1 2)";

    // first we parse to an AST
    bl_ast_node_t* ast = bl_parse_sexp(test_list);

    // now we check the AST looks correct - it should consist of an expression containing the + symbol and 2 integers (1 and 2)
    bl_ast_node_t**  children     = ast->children;
    int              child_count  = ast->child_count;

    ASSERT("child_count==3",child_count == 3)

    ASSERT("node_type==BL_VAL_TYPE_LIST",ast->node_val.type == BL_VAL_TYPE_AST_LIST)

    ASSERT("children[0] is symbol",ast->children[0]->node_val.type == BL_VAL_TYPE_SYMBOL)
    ASSERT("children[1] is int",   ast->children[1]->node_val.type == BL_VAL_TYPE_NUMBER)
    ASSERT("children[2] is int",   ast->children[2]->node_val.type == BL_VAL_TYPE_NUMBER)

    ASSERT("children[0] has value +", strcmp(ast->children[0]->node_val.s_val,"+")==0)

    ASSERT("children[1] has value 1", ast->children[1]->node_val.i_val == 1)
    ASSERT("children[2] has value 2", ast->children[2]->node_val.i_val == 2)


    return 0;
}

int test_ser_sexp() {
    // this test creates an s-expression manually then serialises it and checks for correct format

    bl_ast_node_t* ast = (bl_ast_node_t*)GC_MALLOC(sizeof(bl_ast_node_t));

    ast->child_count = 3;

    ast->children = (bl_ast_node_t**)(GC_MALLOC(sizeof(bl_ast_node_t*)*3));

    ast->node_val.type = BL_VAL_TYPE_AST_LIST;

    ast->children[0] = (bl_ast_node_t*)GC_MALLOC(sizeof(bl_ast_node_t));
    ast->children[1] = (bl_ast_node_t*)GC_MALLOC(sizeof(bl_ast_node_t));
    ast->children[2] = (bl_ast_node_t*)GC_MALLOC(sizeof(bl_ast_node_t));

    ast->children[0]->node_val.type     = BL_VAL_TYPE_SYMBOL;
    ast->children[0]->node_val.s_val    = (char*)GC_MALLOC(sizeof(char)*2);
    ast->children[0]->node_val.s_val[0] = '+';

    ast->children[1]->node_val.type     = BL_VAL_TYPE_NUMBER;
    ast->children[1]->node_val.i_val    = 1;

    ast->children[2]->node_val.type     = BL_VAL_TYPE_NUMBER;
    ast->children[2]->node_val.i_val    = 2;

    char* sexp = bl_ser_ast(ast);

    ASSERT("strcmp(sexp,\"(+ 1 2)\")==0", strcmp(sexp,"(+ 1 2)")==0)

    return 0;
}

int test_ast_pure_sexp() {
    // this test basically turns an AST into a pure expression and then checks it's all correct
    char* test_list = "(1 2 3 4 5 6)";

    // first parse into an AST
    bl_ast_node_t* ast = bl_parse_sexp(test_list);

    // now convert into a pure expression
    bl_val_t* pure_sexp = bl_read_ast(ast);

    // now check the pure expression is all correct
    bl_val_t** items = (bl_val_t**)GC_MALLOC(sizeof(bl_val_t*)*6);

    // first load each item from the list, this will look messy as hell but also double as another test of the list ops
    items[0] = bl_list_first(pure_sexp);                                                          // should be 1
    items[1] = bl_list_second(pure_sexp);                                                         // should be 2
    items[2] = bl_list_second(bl_list_rest(pure_sexp));                                           // should be 3
    items[3] = bl_list_second(bl_list_rest(bl_list_rest(pure_sexp)));                             // should be 4
    items[4] = bl_list_second(bl_list_rest(bl_list_rest(bl_list_rest(pure_sexp))));               // should be 5
    items[5] = bl_list_second(bl_list_rest(bl_list_rest(bl_list_rest(bl_list_rest(pure_sexp))))); // should be 6

    return 1;
}

int test_first_second_rest() {
    // this is a painfully trivial test

    // first construct a list with 3 items: 4, 8, 87

    // first cons cell
    bl_val_t* L = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
    L->type = BL_VAL_TYPE_CONS;

    // first item
    L->car = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
    L->car->type  = BL_VAL_TYPE_NUMBER;
    L->car->i_val = 4;

    // next cons cell
    L->cdr = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
    L->cdr->type = BL_VAL_TYPE_CONS;
    
    // second item
    L->cdr->car = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
    L->cdr->car->type  = BL_VAL_TYPE_NUMBER;
    L->cdr->car->i_val = 8;

    // next cons cell
    L->cdr->cdr = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
    L->cdr->cdr->type = BL_VAL_TYPE_CONS;

    // third item
    L->cdr->cdr->car = (bl_val_t*)GC_MALLOC(sizeof(bl_val_t));
    L->cdr->cdr->car->type  = BL_VAL_TYPE_NUMBER;
    L->cdr->cdr->car->i_val = 87;

    // finally test the fucker

    // first goes first
    bl_val_t* first_val = bl_list_first(L);
    ASSERT("bl_list_first() returns correct val type", first_val->type==BL_VAL_TYPE_NUMBER)
    ASSERT("bl_list_first() returns correct i_val",    first_val->i_val==4)

    // second goes second.....
    bl_val_t* second_val = bl_list_second(L);
    ASSERT("bl_list_second() returns correct val type", second_val->type==BL_VAL_TYPE_NUMBER)
    ASSERT("bl_list_second() returns correct i_val",    second_val->i_val==8)

    // then we do the rest, first checking that it returns a valid list
    bl_val_t* rest = bl_list_rest(L);
    ASSERT("bl_list_rest() returns a list correctly", rest->type=BL_VAL_TYPE_CONS)

    // then we check the first of the rest, which should be the same as the second
    ASSERT("bl_list_first(bl_list_rest(L)) == bl_list_second(L)", bl_list_first(rest)==second_val)

    // and finally, let's check the second of the rest, which should be the third item
    bl_val_t* third = bl_list_second(rest);
    ASSERT("bl_list_second(rest) returns correct val type", third->type==BL_VAL_TYPE_NUMBER)
    ASSERT("bl_list_second(rest) returns correct i_val",    third->i_val==87)
    return 0;

}

int main(int argc, char** argv) {
    int passed_tests = 0;
    int failed_tests = 0;
    int total_tests  = 0;

    bl_init();

    TEST("Simple s-expression parse to AST list",  test_sexp_parse_list)
    TEST("Serialise an s-expression from AST",     test_ser_sexp)
    TEST("Transform AST list into pure expression",test_ast_pure_sexp)
    TEST("List ops: first, second and rest",       test_first_second_rest)

    fprintf(stderr,"Ran %d tests, %d passed, %d failed\n", total_tests, passed_tests, failed_tests);


    if(failed_tests > 0) {
       return 1;
    } else { 
       return 0;
    }
}
