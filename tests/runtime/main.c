#include <stdio.h>
#include <stdlib.h>

#include <bearlang/sexp.h>

#define TEST(desc,f) fprintf(stderr,"Testing %s\t\t",desc); if(f()==0) { passed_tests++; fprintf(stderr,"PASS\n");} else { failed_tests++; fprintf(stderr,"FAIL\n");}; total_tests++;

int test_simple_sexp_list() {
    return 1;
}


int main(int argc, char** argv) {
    int passed_tests = 0;
    int failed_tests = 0;
    int total_tests  = 0;

    TEST("Simple s-expression parse to list",test_simple_sexp_list)

    fprintf(stderr,"Ran %d tests, %d passed, %d failed\n", total_tests, passed_tests, failed_tests);


    if(failed_tests > 0) {
       return 1;
    } else { 
       return 0;
    }
}
