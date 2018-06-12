// LCOV_EXCL_START
#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>
#include <bearlang/string_ops.h>
#include <bearlang/error_tools.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>
#include <bl_lexer.h>

#define TEST(desc,f) fprintf(stderr,"Testing: %s \t",desc); if(f()==0) { passed_tests++; fprintf(stderr,"PASS\n");} else { failed_tests++; fprintf(stderr,"FAIL\n");}; total_tests++;

#define ASSERT(desc,cond) if(! (cond)) { fprintf(stderr,"Assert %s failed\t",desc); return 1;}

bl_val_t* read_form(yyscan_t scanner);
bl_val_t* read_list(yyscan_t scanner);

int test_sexp_parse() {
    char* test_list = "(+ 1 2)";

    bl_val_t* sexp = bl_parse_sexp(test_list);

    ASSERT("first item is symbol", sexp->car->type           == BL_VAL_TYPE_SYMBOL)
    ASSERT("second item is int",   sexp->cdr->car->type      == BL_VAL_TYPE_NUMBER)
    ASSERT("second item is int",   sexp->cdr->cdr->car->type == BL_VAL_TYPE_NUMBER)

    sexp = bl_parse_sexp("");
    ASSERT("empty string", sexp->type == BL_VAL_TYPE_NULL)

    char* empty_str="  ";
    yyscan_t scanner;
    yylex_init(&scanner);
    yy_scan_string(empty_str,scanner);
    sexp = read_list(scanner);
    yylex_destroy(scanner);

    ASSERT("empty string with read_list", bl_list_len(sexp)==0)

    return 0;
}

int test_ser_sexp() {
    bl_val_t* sexp = bl_mk_list(3,bl_mk_symbol("+"),bl_mk_integer("1"),bl_mk_integer("2"));

    char* serialised_sexp = bl_ser_sexp(sexp);

    ASSERT("strcmp(sexp,\"(+ 1 2)\")==0", strcmp(serialised_sexp,"(+ 1 2)")==0)

    bl_val_t* empty_cons = bl_mk_val(BL_VAL_TYPE_CONS);
    empty_cons->car = NULL;
    empty_cons->cdr = NULL;
    serialised_sexp = bl_ser_sexp(empty_cons);
    ASSERT("empty cons ser_exp", strcmp(serialised_sexp,"()")==0)

    bl_val_t* invalid_cons = bl_mk_val(BL_VAL_TYPE_CONS);
    invalid_cons->car = NULL;
    invalid_cons->cdr = bl_parse_sexp("(1)");
    serialised_sexp = bl_ser_sexp(invalid_cons);

    ASSERT("invalid cons ser_exp", strcmp(serialised_sexp,"()")==0)

    bl_val_t* t = bl_mk_bool(true);
    serialised_sexp = bl_ser_sexp(t);
    ASSERT("True ser_exp", strcmp(serialised_sexp,"True")==0)

    bl_val_t* f = bl_mk_bool(false);
    serialised_sexp = bl_ser_sexp(f);
    ASSERT("False ser_exp", strcmp(serialised_sexp,"False")==0)

    ASSERT("NULL ser_sexp", strcmp(bl_ser_sexp(NULL),"")==0)
    return 0;
}

int test_first_second_rest() {
    bl_val_t* L = bl_mk_list(3,bl_mk_integer("4"),bl_mk_integer("8"),bl_mk_integer("87"));

    // first goes first
    bl_val_t* first_val = bl_list_first(L);
    ASSERT("bl_list_first() returns correct val type", first_val->type==BL_VAL_TYPE_NUMBER)
    ASSERT("bl_list_first() returns correct value",    strcmp(bl_ser_sexp(first_val),"4")==0)

    // second goes second.....
    bl_val_t* second_val = bl_list_second(L);
    ASSERT("bl_list_second() returns correct val type", second_val->type==BL_VAL_TYPE_NUMBER)
    ASSERT("bl_list_second() returns correct value",    strcmp(bl_ser_sexp(second_val),"8")==0)

    // then we do the rest, first checking that it returns a valid list
    bl_val_t* rest = bl_list_rest(L);
    ASSERT("bl_list_rest() returns a list correctly", rest->type=BL_VAL_TYPE_CONS)

    // then we check the first of the rest, which should be the same as the second
    ASSERT("bl_list_first(bl_list_rest(L)) == bl_list_second(L)", bl_list_first(rest)==second_val)

    // and finally, let's check the second of the rest, which should be the third item
    bl_val_t* third = bl_list_second(rest);
    ASSERT("bl_list_second(rest) returns correct val type", third->type==BL_VAL_TYPE_NUMBER)
    ASSERT("bl_list_second(rest) returns correct i_val",    strcmp(bl_ser_sexp(third),"87")==0)
    return 0;

}

int test_third() {
    // first construct a list with 3 items: 4, 8, 87
    bl_val_t* L = bl_mk_list(3,bl_mk_integer("4"),bl_mk_integer("8"),bl_mk_integer("87"));

    bl_val_t* third_val = bl_list_third(L);
    ASSERT("bl_list_third(L) returns correct val_type", third_val->type == BL_VAL_TYPE_NUMBER)
    ASSERT("bl_list_third(L) returns correct value",    strcmp(bl_ser_sexp(third_val),"87")==0)
    return 0;
}

int test_prepend_null() {
    bl_val_t* empty      = NULL;
    bl_val_t* first_item = bl_mk_str("foo");

    empty = bl_list_prepend(empty,first_item);

    ASSERT("bl_list_prepend returns a valid cons cell", empty->type==BL_VAL_TYPE_CONS)

    ASSERT("bl_list_prepend returns the correct val type for first item",       bl_list_first(empty)->type==BL_VAL_TYPE_STRING)
    ASSERT("bl_list_prepend returns the correct val number for the first item", strcmp(bl_list_first(empty)->s_val, "foo")==0)

    return 0;
}

int test_append_null() {
    bl_val_t* empty      = NULL;
    bl_val_t* first_item = bl_mk_str("foo");

    empty = bl_list_append(empty,first_item);

    ASSERT("bl_list_append returns a valid cons cell", empty->type==BL_VAL_TYPE_CONS)

    ASSERT("bl_list_append returns the correct val type for first item",       bl_list_first(empty)->type==BL_VAL_TYPE_STRING)
    ASSERT("bl_list_append returns the correct val number for the first item", strcmp(bl_list_first(empty)->s_val, "foo")==0)

    return 0;
}

int test_empty_ctx() {
    // first create the empty context
    bl_val_t* empty_ctx = bl_ctx_new(NULL);

    // now set a random-ish key to an interesting value
    bl_ctx_set(empty_ctx,bl_mk_symbol("TheOneForYouAndMe"), bl_mk_str("666"));


    // and now look it up and check it's the same
    bl_val_t* retval = bl_ctx_get(empty_ctx,bl_mk_symbol("TheOneForYouAndMe"));


    ASSERT("bl_ctx_get/set", (retval->type==BL_VAL_TYPE_STRING) && (strcmp(retval->s_val, "666")==0))

    bl_ctx_close(empty_ctx);
    return 0;
}

int test_child_ctx() {
    // create parent context and set something in it
    bl_val_t* parent_ctx = bl_ctx_new(NULL);
    bl_val_t* our_item   = bl_mk_str("666");
    bl_ctx_set(parent_ctx,bl_mk_symbol("TheOneForYouAndMe"), our_item);

    // create an empty child context and lookup the key in it
    bl_val_t* child_ctx = bl_ctx_new(parent_ctx);

    bl_val_t* looked_up = bl_ctx_get(child_ctx,bl_mk_symbol("TheOneForYouAndMe"));
    ASSERT("bl_ctx_get with child ctx", (looked_up->type==BL_VAL_TYPE_STRING) && (strcmp(looked_up->s_val,"666")==0))
  
    bl_ctx_close(child_ctx);
    bl_ctx_close(parent_ctx);
    return 0;
}

int test_simple_arithmetic() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* sum_str = "(+ 2 3)";

    bl_val_t* sexp = bl_parse_sexp(sum_str);

    bl_val_t* result = bl_ctx_eval(ctx,sexp);
    ASSERT("simple addition (+ 2 3)", (result->type==BL_VAL_TYPE_NUMBER) && (strcmp(bl_ser_sexp(result),"5")==0))

    result = bl_ctx_eval(ctx,bl_parse_sexp("(+ (2 3 1))"));
    ASSERT("simple addition (+ (2 3 1))", (result->type==BL_VAL_TYPE_NUMBER) && (strcmp(bl_ser_sexp(result),"6")==0))
    bl_ctx_close(ctx);
    return 0;
}


int test_nested_addition() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* sum_str = "(+ 1 1 (+ 2 1))";

    bl_val_t* pure_sexp = bl_parse_sexp(sum_str);

    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);
    ASSERT("(+ 1 1 (+ 2 1))", (result->type==BL_VAL_TYPE_NUMBER) && (strcmp(bl_ser_sexp(result),"5")==0))

    bl_ctx_close(ctx);
    return 0;
}

int test_sub_add() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* sum_str = "(- 5 (+ 1 2))";

    bl_val_t* pure_sexp = bl_parse_sexp(sum_str);

    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);
    ASSERT("(- 5 (+ 1 2))", (result->type==BL_VAL_TYPE_NUMBER) && (strcmp(bl_ser_sexp(result),"2")==0))
    bl_ctx_close(ctx);
    return 0;
}

int test_mult() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* sum_str = "(* 3 2)";

    bl_val_t* pure_sexp = bl_parse_sexp(sum_str);

    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);
    ASSERT("(* 3 2)", (result->type==BL_VAL_TYPE_NUMBER)  && (strcmp(bl_ser_sexp(result),"6")==0))

    bl_ctx_close(ctx);
    return 0;
}

int test_div() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* sum_str = "(/ 12 4)";

    bl_val_t* pure_sexp = bl_parse_sexp(sum_str);

    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);
    ASSERT("(/ 12 4)", (result->type==BL_VAL_TYPE_NUMBER) && (strcmp(bl_ser_sexp(result),"3")==0))
    
    bl_ctx_close(ctx);
    return 0;
}


int test_set_oper() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* set_str = "(= test 2)";

    bl_val_t* pure_sexp = bl_parse_sexp(set_str);

    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);

    char* sum_str = "(+ test 3)";

    pure_sexp = bl_parse_sexp(sum_str);

    result = bl_ctx_eval(ctx,pure_sexp);

    ASSERT("(= test 2) (+ test 3)", (result->type==BL_VAL_TYPE_NUMBER)  && (strcmp(bl_ser_sexp(result),"5")==0))

    bl_ctx_close(ctx);
    return 0;
}

int test_simple_func() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* defun_str = "(= test (fn (a b) (- (+ a b) 1)))";
    bl_val_t*      pure_sexp = bl_parse_sexp(defun_str);
    
    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);

    char* test_str = "(test 2 2)";
    pure_sexp = bl_parse_sexp(test_str);

    result = bl_ctx_eval(ctx,pure_sexp);

    ASSERT("Calling (= test (fn (a b) (- (+ a b) 1))) with (2 2)", (result->type==BL_VAL_TYPE_NUMBER)  && (strcmp(bl_ser_sexp(result),"3")==0))

    bl_ctx_close(ctx);
    return 0;
}


int test_multiexpr_func() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* defun_str = "(= test (fn (a b) \
		                       (= c (+ a b))\
                                       (- c 1)))";
    bl_val_t*      pure_sexp = bl_parse_sexp(defun_str);
    
    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);

    char* test_str = "(test 2 2)";
    pure_sexp = bl_parse_sexp(test_str);

    result = bl_ctx_eval(ctx,pure_sexp);
    ASSERT("Calling (= test (fn (a b) (- (+ a b) 1))) with (2 2)", (result->type==BL_VAL_TYPE_NUMBER) && (strcmp(bl_ser_sexp(result),"3")==0))

    bl_ctx_close(ctx);
    return 0;
}

int test_fun_oper() {
    bl_val_t* ctx = bl_ctx_new_std();
    char* defun_str = "(fun test (a b) \
		                       (= c (+ a b))\
                                       (- c 1))";
    bl_val_t*      pure_sexp = bl_parse_sexp(defun_str);
    
    bl_val_t* result = bl_ctx_eval(ctx,pure_sexp);

    char* test_str = "(test 2 2)";
    pure_sexp = bl_parse_sexp(test_str);

    result = bl_ctx_eval(ctx,pure_sexp);
    ASSERT("Calling (= test (fn (a b) (- (+ a b) 1))) with (2 2)", (result->type==BL_VAL_TYPE_NUMBER) && (strcmp(bl_ser_sexp(result),"3")==0))

    bl_ctx_close(ctx);
    return 0;
}

int test_list_len() {
    char* empty_list = "()";
    bl_val_t* sexp = bl_parse_sexp(empty_list);
    uint64_t empty_len = bl_list_len(sexp);
    ASSERT("length of ()==0",empty_len==0)

    char* single_item_list = "(1337)";
    sexp  = bl_parse_sexp(single_item_list);
    uint64_t single_len = bl_list_len(sexp);

    ASSERT("length of (1337)==1",single_len==1)

    char* multi_item_list = "(1337 42 666)";
    sexp  = bl_parse_sexp(multi_item_list);
    uint64_t multi_len = bl_list_len(sexp);
    ASSERT("length of (1337 42 666)==3",multi_len==3)

    ASSERT("length of NULL==0", bl_list_len(NULL)==0)
    bl_val_t* empty_cons = bl_mk_val(BL_VAL_TYPE_CONS);
    empty_cons->car = NULL;
    empty_cons->cdr = NULL;
    ASSERT("length of NULL==0", bl_list_len(empty_cons)==0)

    bl_val_t* invalid_cons = bl_mk_val(BL_VAL_TYPE_CONS);
    invalid_cons->car = NULL;
    invalid_cons->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
    ASSERT("length of invalid cons==0", bl_list_len(invalid_cons)==0)

    return 0;
}

int test_eq_oper() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* is_eq_str  = "(eq 2 2)";
    char* not_eq_str = "(eq 2 3)";

    bl_val_t* equal_exp = bl_parse_sexp(is_eq_str);
    
    bl_val_t* not_equal_exp = bl_parse_sexp(not_eq_str);

    bl_val_t* is_eq_result  = bl_ctx_eval(ctx,equal_exp);
    bl_val_t* not_eq_result = bl_ctx_eval(ctx,not_equal_exp);

    ASSERT("(eq 2 2) is #t", is_eq_result->b_val)
    ASSERT("(eq 2 3) is #f", !not_eq_result->b_val)

    is_eq_result = bl_ctx_eval(ctx,bl_parse_sexp("(eq \"foo\" \"foo\")"));
    ASSERT("(eq \"foo\" \"foo\") is #t", is_eq_result->b_val)

    not_eq_result = bl_ctx_eval(ctx,bl_parse_sexp("(eq \"foo\" \"bar\")"));
    ASSERT("(eq \"foo\" \"bar\") is #f", !not_eq_result->b_val)

    not_eq_result = bl_ctx_eval(ctx,bl_parse_sexp("(eq \"foo\" 2)"));
    ASSERT("(eq \"foo\" 2) is #f", !not_eq_result->b_val)

    bl_ctx_close(ctx);
    return 0;
}

int test_simple_if() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* true_if_str  = "(if True 666 42)";
    char* false_if_str = "(if False 1337 69)";

    bl_val_t* true_exp = bl_parse_sexp(true_if_str);
    
    bl_val_t* false_exp = bl_parse_sexp(false_if_str);

    bl_val_t* true_if_result  = bl_ctx_eval(ctx,true_exp);
    bl_val_t* false_if_result = bl_ctx_eval(ctx,false_exp);

    ASSERT("(if True 666 42) returns 666",  strcmp(bl_ser_sexp(true_if_result),"666")==0)
    ASSERT("(if False 1337 69) returns 69", strcmp(bl_ser_sexp(false_if_result),"69")==0)


    bl_ctx_close(ctx);
    return 0;
}

int test_multiexpr_if() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* true_if_str  = "(if True (do (= x 2)\
	                               (+ x 1))\
				       42)";
    char* false_if_str = "(if False 1337 69)";

    bl_val_t* true_exp = bl_parse_sexp(true_if_str);
    
    bl_val_t* false_exp = bl_parse_sexp(false_if_str);

    bl_val_t* true_if_result  = bl_ctx_eval(ctx,true_exp);
    bl_val_t* false_if_result = bl_ctx_eval(ctx,false_exp);

    ASSERT("(if True (do (= x 2) (+ x 1) 42) returns 3", strcmp(bl_ser_sexp(true_if_result),"3")==0)
    ASSERT("(if False 1337 69) returns 69",  strcmp(bl_ser_sexp(false_if_result),"69")==0)

    bl_ctx_close(ctx);
    return 0;
}

int test_and_oper() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* tt_str = "(and True True)";
    char* ff_str = "(and False False)";
    char* tf_str = "(and True False)";

    bl_val_t* tt_result = bl_ctx_eval(ctx,bl_parse_sexp(tt_str));
    bl_val_t* ff_result = bl_ctx_eval(ctx,bl_parse_sexp(ff_str));
    bl_val_t* tf_result = bl_ctx_eval(ctx,bl_parse_sexp(tf_str));

    ASSERT("(and True True) returns True",    tt_result->b_val==true)
    ASSERT("(and False False) returns False", ff_result->b_val==false)
    ASSERT("(and True False) returns False",  tf_result->b_val==false)

    bl_ctx_close(ctx);
    return 0;
}

int test_not_oper() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* t_str = "(not False)";
    char* f_str = "(not True)";

    bl_val_t* t_result = bl_ctx_eval(ctx,bl_parse_sexp(t_str));
    bl_val_t* f_result = bl_ctx_eval(ctx,bl_parse_sexp(f_str));

    ASSERT("(not False) returns True", t_result->b_val==true)
    ASSERT("(not True) returns False", f_result->b_val==false)

    bl_ctx_close(ctx);
    return 0;
}

int test_or_oper() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* tt_str = "(or True True)";
    char* ff_str = "(or False False)";
    char* tf_str = "(or True False)";
    char* ft_str = "(or False True)";

    bl_val_t* tt_result = bl_ctx_eval(ctx,bl_parse_sexp(tt_str));
    bl_val_t* ff_result = bl_ctx_eval(ctx,bl_parse_sexp(ff_str));
    bl_val_t* tf_result = bl_ctx_eval(ctx,bl_parse_sexp(tf_str));
    bl_val_t* ft_result = bl_ctx_eval(ctx,bl_parse_sexp(ft_str));

    ASSERT("(or True True) returns True",    tt_result->b_val)
    ASSERT("(or False False) returns False", !ff_result->b_val)
    ASSERT("(or True False) returns True",  tf_result->b_val)
    ASSERT("(or False True) returns True",  ft_result->b_val)

    bl_ctx_close(ctx);
    return 0;
}

int test_xor_oper() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* tt_str = "(xor True True)";
    char* ff_str = "(xor False False)";
    char* tf_str = "(xor True False)";

    bl_val_t* tt_result = bl_ctx_eval(ctx,bl_parse_sexp(tt_str));
    bl_val_t* ff_result = bl_ctx_eval(ctx,bl_parse_sexp(ff_str));
    bl_val_t* tf_result = bl_ctx_eval(ctx,bl_parse_sexp(tf_str));

    ASSERT("(xor True True) returns True",    tt_result->b_val==false)
    ASSERT("(xor False False) returns False", ff_result->b_val==false)
    ASSERT("(xor True False) returns False",  tf_result->b_val==true)

    bl_ctx_close(ctx);
    return 0;
}

int test_list_opers() {
    bl_val_t* ctx = bl_ctx_new_std();

    char* first_str     = "(first 1 2 3 4 5)";
    char* first_str_b   = "(first (1 2 3 4 5))";
    char* second_str    = "(second 1 2 3 4 5)";
    char* second_str_b  = "(second (1 2 3 4 5))";
    char* third_str     = "(third 1 2 3 4 5)";
    char* third_str_b   = "(third (1 2 3 4 5))";
    char* rest_str      = "(rest 1 2 3 4 5)";
    char* rest_str_b    = "(rest (1 2 3 4 5))";
    char* reverse_str   = "(reverse 1 2 3 4 5)";
    char* reverse_str_b = "(reverse (1 2 3 4 5))";
    char* append_str    = "(append (1 2 3 4) 5";
    char* prepend_str   = "(prepend (1 2 3 4) 5";

    bl_val_t* first_result     = bl_ctx_eval(ctx,bl_parse_sexp(first_str));
    bl_val_t* first_result_b   = bl_ctx_eval(ctx,bl_parse_sexp(first_str_b));
    bl_val_t* second_result    = bl_ctx_eval(ctx,bl_parse_sexp(second_str));
    bl_val_t* second_result_b  = bl_ctx_eval(ctx,bl_parse_sexp(second_str_b));
    bl_val_t* third_result     = bl_ctx_eval(ctx,bl_parse_sexp(third_str));
    bl_val_t* third_result_b   = bl_ctx_eval(ctx,bl_parse_sexp(third_str_b));
    bl_val_t* rest_result      = bl_ctx_eval(ctx,bl_parse_sexp(rest_str));
    bl_val_t* rest_result_b    = bl_ctx_eval(ctx,bl_parse_sexp(rest_str_b));
    bl_val_t* reverse_result   = bl_ctx_eval(ctx,bl_parse_sexp(reverse_str));
    bl_val_t* reverse_result_b = bl_ctx_eval(ctx,bl_parse_sexp(reverse_str_b));
    bl_val_t* append_result    = bl_ctx_eval(ctx,bl_parse_sexp(append_str));
    bl_val_t* prepend_result   = bl_ctx_eval(ctx,bl_parse_sexp(prepend_str));

    ASSERT("(first 1 2 3 4 5)  returns 1", strcmp(bl_ser_sexp(first_result),"1")==0)
    ASSERT("(first (1 2 3 4 5))  returns 1", strcmp(bl_ser_sexp(first_result_b),"1")==0)
    ASSERT("(second 1 2 3 4 5) returns 2", strcmp(bl_ser_sexp(second_result),"2")==0)
    ASSERT("(second (1 2 3 4 5)) returns 2", strcmp(bl_ser_sexp(second_result_b),"2")==0)
    ASSERT("(third 1 2 3 4 5)  returns 3", strcmp(bl_ser_sexp(third_result),"3")==0)
    ASSERT("(third (1 2 3 4 5))  returns 3", strcmp(bl_ser_sexp(third_result_b),"3")==0)

    ASSERT("(rest 1 2 3 4 5)  returns (2 3 4 5)", strcmp(bl_ser_sexp(rest_result),"(2 3 4 5)")==0)
    ASSERT("(rest (1 2 3 4 5))  returns (2 3 4 5)", strcmp(bl_ser_sexp(rest_result_b),"(2 3 4 5)")==0)

    ASSERT("(reverse 1 2 3 4 5)  returns (5 4 3 2 1)", strcmp(bl_ser_sexp(reverse_result),"(5 4 3 2 1)")==0)
    ASSERT("(reverse (1 2 3 4 5))  returns (5 4 3 2 1)", strcmp(bl_ser_sexp(reverse_result_b),"(5 4 3 2 1)")==0)

    ASSERT("(append (1 2 3 4) 5)  returns (1 2 3 4 5)", strcmp(bl_ser_sexp(append_result),"(1 2 3 4 5)")==0)
    ASSERT("(prepend (1 2 3 4) 5)  returns (5 1 2 3 4)", strcmp(bl_ser_sexp(prepend_result),"(5 1 2 3 4)")==0)
    bl_ctx_close(ctx);
    return 0;
}


int test_list_ops() {
    bl_val_t* ctx = bl_ctx_new_std();

    bl_val_t* first_result    = bl_list_first(bl_parse_sexp("(1 2 3 4 5)"));
    bl_val_t* second_result   = bl_list_second(bl_parse_sexp("(1 2 3 4 5)"));
    bl_val_t* third_result    = bl_list_third(bl_parse_sexp("(1 2 3 4 5)"));
    bl_val_t* rest_result     = bl_list_rest(bl_parse_sexp("(1 2 3 4 5)"));
    bl_val_t* reverse_result  = bl_list_reverse(bl_parse_sexp("(1 2 3 4 5)"));

    ASSERT("(first 1 2 3 4 5)  returns 1", strcmp(bl_ser_sexp(first_result),"1")==0)
    ASSERT("(second 1 2 3 4 5) returns 2", strcmp(bl_ser_sexp(second_result),"2")==0)
    ASSERT("(third 1 2 3 4 5)  returns 3", strcmp(bl_ser_sexp(third_result),"3")==0)
    ASSERT("(rest 1 2 3 4 5)  returns (2 3 4 5)", strcmp(bl_ser_sexp(rest_result),"(2 3 4 5)")==0)
    ASSERT("(reverse 1 2 3 4 5)  returns (5 4 3 2 1)", strcmp(bl_ser_sexp(reverse_result),"(5 4 3 2 1)")==0)

    ASSERT("first(NULL) is NULL", bl_list_first(NULL)==NULL)
    ASSERT("second(NULL) is NULL", bl_list_second(NULL)==NULL)
    ASSERT("third(NULL) is NULL", bl_list_third(NULL)==NULL)
    bl_ctx_close(ctx);
    return 0;
}

int test_list_last() {
    bl_val_t* L = bl_mk_list(3,bl_mk_str("1"),bl_mk_str("2"),bl_mk_str("3"));
    bl_val_t* last = bl_list_last(L);
    ASSERT("correct last item", strcmp(bl_ser_naked_sexp(last),"3")==0)
    last = bl_list_last(NULL);
    ASSERT("last of NULL is NULL", last==NULL)
    return 0;
}

int test_parse_string() {
    char* test_str = "\"Hello world\"";
    bl_val_t* parsed = bl_parse_sexp(test_str);
    ASSERT("String parsed OK", strstr(parsed->s_val,"Hello world"))

    parsed = bl_parse_sexp("\"\\n\"");
    ASSERT("String parsed OK", strstr(parsed->s_val,"\n"))


    parsed = bl_parse_sexp("\"\\t\"");
    ASSERT("String parsed OK", strstr(parsed->s_val,"\t"))

    return 0;
}

int test_eval_file() {
    char* tmpfile = "/tmp/bearlang-test.bl";
    FILE* fd = fopen(tmpfile,"w");
    fprintf(fd,"True\n");
    fclose(fd);
    fd = fopen(tmpfile,"r");
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* eval_result = bl_eval_file(ctx,tmpfile,fd);
    fclose(fd);
    ASSERT("Successfully evaluated file",  eval_result->car->b_val)
    return 0;
}

int test_while_oper() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_ctx_eval(ctx,bl_parse_sexp("(= x 10)"));
    bl_ctx_eval(ctx,bl_parse_sexp("(while (gt x 0) (= x (- x 1)))"));

    bl_val_t* x_val = bl_ctx_get(ctx,bl_mk_symbol("x"));
    ASSERT("while works correctly", strcmp(bl_ser_sexp(x_val),"0")==0)

    bl_ctx_close(ctx);
    return 0;
}

int test_inc_dec() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_ctx_eval(ctx,bl_parse_sexp("(= x 10)"));
    bl_ctx_eval(ctx,bl_parse_sexp("(inc x)"));
    bl_val_t* x_val = bl_ctx_get(ctx,bl_mk_symbol("x"));
    ASSERT("inc works correctly", strcmp(bl_ser_sexp(x_val),"11")==0)

    bl_ctx_eval(ctx,bl_parse_sexp("(= y 10)"));
    bl_ctx_eval(ctx,bl_parse_sexp("(dec y)"));
    bl_val_t* y_val = bl_ctx_get(ctx,bl_mk_symbol("y"));
    ASSERT("dec works correctly", strcmp(bl_ser_sexp(y_val),"9")==0)
    bl_ctx_close(ctx);
    return 0;
}

int test_lt_gt() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* lt_true_result = bl_ctx_eval(ctx,bl_parse_sexp("(lt 2 3)"));
    bl_val_t* gt_true_result = bl_ctx_eval(ctx,bl_parse_sexp("(gt 3 2)"));
    ASSERT("(lt 2 3) returns true", lt_true_result->b_val)
    ASSERT("(gt 3 2) returns true", gt_true_result->b_val)

    bl_val_t* lt_false_result = bl_ctx_eval(ctx,bl_parse_sexp("(lt 2 2)"));
    bl_val_t* gt_false_result = bl_ctx_eval(ctx,bl_parse_sexp("(gt 3 3)"));
    ASSERT("(lt 2 2) returns false", !lt_false_result->b_val)
    ASSERT("(gt 3 3) returns false", !gt_false_result->b_val)

    bl_ctx_close(ctx);
    return 0;
}

int test_isset() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_ctx_eval(ctx,bl_parse_sexp("(= x 10)"));

    bl_val_t* set_true_result = bl_ctx_eval(ctx,bl_parse_sexp("(isset x)"));
    bl_val_t* set_false_result = bl_ctx_eval(ctx,bl_parse_sexp("(isset doesnotexist)"));
    ASSERT("(isset x) returns true", set_true_result->b_val)

    ASSERT("(isset doesnotexist) returns false", !set_false_result->b_val)

    bl_ctx_close(ctx);
    return 0;
}

int test_map() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* map_result = bl_ctx_eval(ctx,bl_parse_sexp("(map (fn (x) (+ x 1)) (1 2 3 4))"));

    ASSERT("(map (fn (x) (+ x 1)) (1 2 3 4) returns (2 3 4 5)", strcmp(bl_ser_sexp(map_result),"(2 3 4 5)")==0)

    bl_ctx_close(ctx);
    return 0;
}

int test_modulo() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* modulo_result = bl_ctx_eval(ctx,bl_parse_sexp("(% 5 3)"));
    ASSERT("(% 5 3) returns 2", strcmp(bl_ser_sexp(modulo_result),"2")==0)
    
    modulo_result = bl_ctx_eval(ctx,bl_parse_sexp("(% 15 5)"));
    ASSERT("(% 15 5) returns 0", strcmp(bl_ser_sexp(modulo_result),"0")==0)

    bl_ctx_close(ctx);
    return 0;

}

int test_add_strings() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(+ \"foo\" \"bar\")"));
    ASSERT("(+ \"foo\" \"bar\") returns \"foobar\"", strcmp(bl_ser_naked_sexp(result),"foobar")==0)
   
    result = bl_ctx_eval(ctx,bl_parse_sexp("(+ = \"foo\" 20 =)")); 
    ASSERT("(+ = \"foo\" 20 =)", strcmp(bl_ser_naked_sexp(result),"<nativeoper>foo20<nativeoper>")==0)

    bl_ctx_close(ctx);
    return 0;

}

int test_add_lists() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(+ (1) 2 3 4 (5 6))"));
    ASSERT("(+ (1) 2 3 4 (5 6)) returns (1 2 3 4 5 6)", strcmp(bl_ser_naked_sexp(result),"(1 2 3 4 5 6)")==0)

    bl_ctx_close(ctx);
    return 0;
}

int test_serexp_oper() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(serexp (1 2 3))"));
    ASSERT("(serexp (1 2 3)) returns \"(1 2 3)\"", strcmp(bl_ser_sexp(result),"\"(1 2 3)\"")==0)
    result = bl_ctx_eval(ctx,bl_parse_sexp("(serexp 1 2 3)"));
    ASSERT("(serexp 1 2 3) returns \"(1 2 3)\"", strcmp(bl_ser_sexp(result),"\"(1 2 3)\"")==0)
    bl_ctx_close(ctx);
    return 0;
}

int test_include_oper() {
    char* tmpfile = "/tmp/bearlang-test.bl";
    FILE* fd = fopen(tmpfile,"w");
    fprintf(fd,"True\n");
    fclose(fd);
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(include \"/tmp/bearlang-test.bl\")"));
    ASSERT("include successfully tested", strcmp(bl_ser_naked_sexp(result),"(True)")==0)
    return 0;
}

int test_mk_list() {
    bl_val_t* L = bl_mk_list(2,bl_mk_integer("1"),bl_mk_integer("2"));
    ASSERT("bl_mk_list with 2 items", strcmp(bl_ser_sexp(L),"(1 2)")==0)

    L = bl_mk_list(0);
    ASSERT("bl_mk_list with 0 items", strcmp(bl_ser_sexp(L),"()")==0)

    L = bl_mk_list(1,bl_mk_integer("1"),bl_mk_integer("2"));
    ASSERT("bl_mk_list with 2 items and incorrect count", strcmp(bl_ser_sexp(L),"(1)")==0)
    return 0;
}

int test_mk_sym() {
    bl_val_t* A_a = bl_mk_symbol("A");
    bl_val_t* A_b = bl_mk_symbol("A");
    ASSERT("bl_mk_symbol working", A_a==A_b);
    return 0;

}

int test_pool_alloc() {
    // basically just allocate a whole pile of values in a loop
    int i=0;
    for(i=0; i<6000000; i++) bl_mk_val(BL_VAL_TYPE_NULL);
    GC_gcollect();
    return 0;
}

int test_ctx_stress() {
    bl_val_t* new_ctx = bl_ctx_new(NULL);
    int i=0;
#ifdef DEBUG 
#define STRESS_ITER 1000
#else
#define STRESS_ITER 100000
#endif
    printf("%d iterations\n",STRESS_ITER);
    char sbuf[100];
    for(i=0; i<STRESS_ITER; i++) {
        snprintf(sbuf,100,"%d",i);
	bl_val_t* new_sym = bl_mk_symbol(sbuf);
	bl_val_t* new_int = bl_mk_integer(sbuf);
	bl_ctx_set(new_ctx,new_sym,new_int);
        
    }
    GC_gcollect();
    return 0;
}

int test_symnotfound() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(+ 1 2 bacon)"));

    ASSERT("get non-existent symbol error", result->type==BL_VAL_TYPE_ERROR)

    result = bl_ctx_eval(ctx,bl_parse_sexp("(foobar 1 2 3)"));

    ASSERT("get non-existent symbol error", result->type==BL_VAL_TYPE_ERROR)

    bl_ctx_close(ctx);
    return 0;
}

int test_eval_invalid() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("()"));

    ASSERT("eval(None) returns None", result->type==BL_VAL_TYPE_NULL)

    result = bl_ctx_eval(ctx,NULL);
    ASSERT("eval(None) returns None", result->type==BL_VAL_TYPE_NULL)

    bl_val_t* err = bl_mk_val(BL_VAL_TYPE_ERROR);
    result = bl_ctx_eval(ctx,err);
    ASSERT("eval(error) returns error", result==err)

    bl_val_t* err_list = bl_mk_list(2,err,bl_mk_str("bla"));
    result = bl_ctx_eval(ctx,err_list);
    ASSERT("eval(error) returns error", result==err)
	
    bl_ctx_close(ctx);
    return 0;
}

int test_zeroparam_func() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_ctx_eval(ctx,bl_parse_sexp("(fun myfunc () True)"));
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(myfunc)"));

    ASSERT("Zero-param function returns correct value", result->b_val)

    bl_ctx_close(ctx);
    return 0;
}

int test_eval_numberlist() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(1 2 3 )"));
    ASSERT("Return value is a list", result->type==BL_VAL_TYPE_CONS)
    ASSERT("Return value is the correct list",  strcmp(bl_ser_sexp(result),"(1 2 3)")==0)
    bl_ctx_close(ctx);
    return 0;
}

int test_while_error() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(while True foo)"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR) 
    return 0;
}

int test_simple_oper() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_ctx_eval(ctx,bl_parse_sexp("(oper test () (= x 2))"));
    bl_ctx_eval(ctx,bl_parse_sexp("(test)"));
    bl_val_t* result = bl_ctx_eval(ctx, bl_parse_sexp("x"));
    ASSERT("Return value is correct",  strcmp(bl_ser_sexp(result),"2")==0)
    return 0;
}

int test_map_error() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(map foo)"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR) 

    result = bl_ctx_eval(ctx,bl_parse_sexp("(map 1 2)"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR) 

    bl_ctx_eval(ctx,bl_parse_sexp("(fun errfunc (x) True bar)"));
    result = bl_ctx_eval(ctx,bl_parse_sexp("(map errfunc (1 2 3))"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR) 

    bl_ctx_eval(ctx,bl_parse_sexp("(fun errfunc (x) foobar True)"));
    result = bl_ctx_eval(ctx,bl_parse_sexp("(map errfunc (1 2 3))"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR) 

    bl_ctx_close(ctx);
    return 0;
}

int test_add_noargs() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(+)"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR)
    bl_ctx_close(ctx);
    return 0;
}

int test_div_toomanyargs() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(/ 1 2 3)"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR)
    bl_ctx_close(ctx);
    return 0;
}

int test_div_stringargs() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(/ \"a\" \"b\" )"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR)
    bl_ctx_close(ctx);
    return 0;
}

int test_div_onearg() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(/ 1 )"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR)
    bl_ctx_close(ctx);
    return 0;
}

int test_sub_onearg() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(- 1 )"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR)
    bl_ctx_close(ctx);
    return 0;
}

int test_mult_onearg() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(* 1 )"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR)
    bl_ctx_close(ctx);
    return 0;
}

int test_div_zero() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(/ 42 0 )"));
    ASSERT("Return value is an error", result->type==BL_VAL_TYPE_ERROR)
    bl_ctx_close(ctx);
    return 0;
}

int test_docstring_parse() {
    bl_val_t* docstr = bl_parse_sexp("\"\"\"Test\"\"\"");
    ASSERT("s_val is correct", strcmp(docstr->s_val,"Test")==0)
    return 0;
}

int test_import_testmod() {
    bl_val_t* ctx    = bl_ctx_new_std();

    bl_ctx_set(ctx,bl_mk_symbol("*MAINFILE*"), bl_mk_str(""));
    bl_ctx_set(ctx,bl_mk_symbol("*FILENAME*"), bl_mk_str(""));
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(import testmod)"));
    ASSERT("import does not return an error", result->type != BL_VAL_TYPE_ERROR)
    result = bl_ctx_eval(ctx,bl_parse_sexp("(testmod::get_cptr)"));
    ASSERT("get_cptr returns a pointer type", result->type == BL_VAL_TYPE_CPTR)
    void* ptr = result->c_ptr;
    ASSERT("ptr is not NULL", ptr != NULL)
    char* test_str = (char*)ptr;
    ASSERT("ptr as string is correct", strcmp(test_str,"TEST STRING")==0)
    bl_val_t* nested = bl_ctx_eval(ctx,bl_parse_sexp("(import testmod::nested)"));
    ASSERT("nested imported ok", nested->type == BL_VAL_TYPE_CTX)
    bl_val_t* nested_str = bl_ctx_get(ctx, bl_mk_symbol("nested::teststr"));
    ASSERT("nested string correct", strcmp(nested_str->s_val,"TEST STRING2")==0)
    bl_val_t* nested2 = bl_ctx_eval(ctx,bl_parse_sexp("(import testmod::nested::nested2)"));
    ASSERT("nested2 imported ok", nested2->type == BL_VAL_TYPE_CTX)
    bl_val_t* nested2_str = bl_ctx_get(ctx, bl_mk_symbol("nested2::teststr2"));
    ASSERT("nested2 string correct", strcmp(nested2_str->s_val,"TEST STRING3")==0)
    return 0;
}

int test_parse_builtin() {
    bl_val_t* ctx    = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(parse \"(1 2 3)\""));
    ASSERT("result is a list", result->type == BL_VAL_TYPE_CONS)
    ASSERT("result is correctly parsed", strcmp(bl_ser_sexp(result),"(1 2 3)")==0)
    return 0;
}


int test_quote_builtin() {
    bl_val_t* ctx    = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(quote print)"));
    ASSERT("result is a symbol", result->type == BL_VAL_TYPE_SYMBOL)
    ASSERT("result is correct symbol", strcmp(result->s_val,"print")==0)
    result = bl_ctx_eval(ctx,bl_parse_sexp("(quote 1 2 3)"));
    ASSERT("result is a list", result->type == BL_VAL_TYPE_CONS)
    ASSERT("result is correct list", strcmp(bl_ser_sexp(result),"(1 2 3)")==0)

    return 0;
}

int test_import_noexist() {
    bl_val_t* ctx    = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(import notexist)"));
    ASSERT("result is an error", result->type == BL_VAL_TYPE_ERROR)
    result = bl_ctx_eval(ctx,bl_parse_sexp("(import testmod)"));
    result = bl_ctx_eval(ctx,bl_parse_sexp("(import testmod::notexist)"));
    ASSERT("result is an error", result->type == BL_VAL_TYPE_ERROR)
    result = bl_ctx_eval(ctx,bl_parse_sexp("(import notexist::alsonotexist)"));
    ASSERT("result is an error", result->type == BL_VAL_TYPE_ERROR)
    return 0;
}

int test_docstr_fun() {
    bl_val_t* ctx    = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx, bl_parse_sexp("(fun test (x) \"\"\"foobar\"\"\" (+ x 1))"));
    result           = bl_ctx_eval(ctx, bl_parse_sexp("(doc test)"));
    ASSERT("result is correct docstr", strcmp(bl_ser_naked_sexp(result),"foobar")==0)
    return 0;
}

int test_invalid_dylib_import() {
    bl_val_t* ctx    = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(import testmod2)"));
    ASSERT("result is an error", result->type == BL_VAL_TYPE_ERROR)

    return 0;
}

int test_using_oper() {
    bl_val_t* ctx_a = bl_ctx_new_std();
    bl_val_t* ctx_b = bl_ctx_new_std();
    bl_val_t* val_a = bl_mk_str("foo");
    bl_val_t* val_b = bl_mk_str("bar");

    bl_ctx_set(ctx_a,bl_mk_symbol("a"), val_a);
    bl_ctx_set(ctx_a,bl_mk_symbol("b"), val_b);

    bl_ctx_set(ctx_b,bl_mk_symbol("ctx_a"),ctx_a);

    bl_ctx_eval(ctx_b, bl_parse_sexp("(using ctx_a::a)"));
    bl_val_t* result = bl_ctx_eval(ctx_b, bl_parse_sexp("a"));
    ASSERT("value a", strcmp(bl_ser_naked_sexp(result),"foo")==0)

    bl_ctx_eval(ctx_b, bl_parse_sexp("(using ctx_a::*)"));
    result = bl_ctx_eval(ctx_b, bl_parse_sexp("b"));
    ASSERT("value a", strcmp(bl_ser_naked_sexp(result),"bar")==0)

    result = bl_ctx_eval(ctx_a, bl_parse_sexp("(using doesnotexist::something)"));
    ASSERT("error on nonexistent module", result->type == BL_VAL_TYPE_ERROR)

    result = bl_ctx_eval(ctx_b, bl_parse_sexp("(using ctx_a::doesnotexist)"));
    ASSERT("error on nonexistent symbol", result->type == BL_VAL_TYPE_ERROR)
	
    return 0;
}

int test_filtered_oper() {
    bl_val_t* ctx    = bl_ctx_new_std();
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("(filtered (1 2 3 4 5 6) (2 3 4))"));
    ASSERT("filtered list is correct", strcmp(bl_ser_sexp(result),"(1 5 6)")==0)
    return 0;
}

int test_foreach_oper() {
    bl_val_t* ctx    = bl_ctx_new_std();
    bl_ctx_eval(ctx,bl_parse_sexp("(= x 0)"));
    bl_ctx_eval(ctx,bl_parse_sexp("(foreach i (1 2 3) (= x (+ x i)))"));
    bl_val_t* result = bl_ctx_eval(ctx,bl_parse_sexp("x"));
    ASSERT("foreach works", strcmp(bl_ser_sexp(result),"6")==0)
    return 0;
}

int test_try_oper() {
    bl_val_t* ctx = bl_ctx_new_std();
    bl_val_t* try_expr = bl_parse_sexp("(try (/ 2 0)"
		                       "     (catch ERR_DIVIDE_BY_ZERO \"CAUGHT DIV0\"))");
    bl_val_t* result = bl_ctx_eval(ctx,try_expr);
    ASSERT("Catch works",strcmp(bl_ser_naked_sexp(result),"CAUGHT DIV0")==0)

    try_expr = bl_parse_sexp("(try True)");
    result   = bl_ctx_eval(ctx,try_expr);
    ASSERT("Empty catch works", result->b_val==true)
    return 0;
}

int test_safe_strcat() {
    char* a = "foo";
    char* b = "bar";
    char* result = safe_strcat(a,b);
    ASSERT("correct result", strcmp(result,"foobar")==0)
    return 0;
}

int test_split_str() {
    bl_val_t* result = split_str("meow::foo::bar::1::2","::");
    ASSERT("correct split result", strcmp(bl_ser_sexp(result),"(\"meow\" \"foo\" \"bar\" \"1\" \"2\")")==0)
    result = split_str("a::b::c::","::");
    ASSERT("correct split result", strcmp(bl_ser_sexp(result),"(\"a\" \"b\" \"c\")")==0)
    result = split_str("::a::b::c::","::");
    ASSERT("correct split result", strcmp(bl_ser_sexp(result),"(\"\" \"a\" \"b\" \"c\")")==0)
    result = split_str("a::b::c:d","::");
    ASSERT("correct split result", strcmp(bl_ser_sexp(result),"(\"a\" \"b\" \"c:d\")")==0)
    result = split_str("a",":::");
    ASSERT("correct split result", strcmp(bl_ser_sexp(result),"(\"a\")")==0)
    return 0;
}

int test_join_str() {
    bl_val_t* L = bl_mk_list(3,bl_mk_str("a"),bl_mk_str("b"),bl_mk_str("c"));
    char* result = join_str(L,"::");
    ASSERT("Correctly joined string", strcmp(result,"a::b::c")==0)
    return 0;
}

int main(int argc, char** argv) {
    int passed_tests = 0;
    int failed_tests = 0;
    int total_tests  = 0;

    bl_init();

    TEST("Simple s-expression parse                  ", test_sexp_parse)
    TEST("Serialise an s-expression                  ", test_ser_sexp)
    TEST("List ops: first, second and rest           ", test_first_second_rest)
    TEST("List ops: third                            ", test_third)
    TEST("List ops: prepend to NULL                  ", test_prepend_null)
    TEST("List ops: append to NULL                   ", test_append_null)
    TEST("List ops: get list length                  ", test_list_len)
    TEST("List ops: first,second,third,rest          ", test_list_ops)
    TEST("List ops: last                             ", test_list_last)
    TEST("Create an empty context and get/set        ", test_empty_ctx)
    TEST("Create a child context and lookup in parent", test_child_ctx)
    TEST("Simple arithmetic (addition)               ", test_simple_arithmetic)
    TEST("Nested addition                            ", test_nested_addition)
    TEST("Addition and subtraction in one expression ", test_sub_add)
    TEST("Multiplication                             ", test_mult)
    TEST("Division                                   ", test_div)
    TEST("Set operator                               ", test_set_oper)
    TEST("Simple function                            ", test_simple_func)
    TEST("Multi-expression function                  ", test_multiexpr_func)
    TEST("Equality operator                          ", test_eq_oper)
    TEST("fun operator                               ", test_fun_oper)
    TEST("simple if statement                        ", test_simple_if)
    TEST("multi-expression if statement (do oper)    ", test_multiexpr_if)
    TEST("and operator                               ", test_and_oper)
    TEST("not operator                               ", test_not_oper)
    TEST("or operator                                ", test_or_oper)
    TEST("xor operator                               ", test_xor_oper)
    TEST("list operators                             ", test_list_opers)
    TEST("parse a string                             ", test_parse_string)
    TEST("evaluate file                              ", test_eval_file)
    TEST("while oper                                 ", test_while_oper)
    TEST("inc and dec opers                          ", test_inc_dec)
    TEST("lt and gt opers                            ", test_lt_gt)
    TEST("isset oper                                 ", test_isset)
    TEST("map oper                                   ", test_map)
    TEST("modulo oper (%)                            ", test_modulo)
    TEST("add strings                                ", test_add_strings)
    TEST("add lists                                  ", test_add_lists)
    TEST("serexp oper                                ", test_serexp_oper)
    TEST("include oper                               ", test_include_oper)
    TEST("bl_mk_list                                 ", test_mk_list)
    TEST("bl_mk_symbol                               ", test_mk_sym)
    TEST("pool allocation                            ", test_pool_alloc)
    TEST("ctx stress test                            ", test_ctx_stress)
    TEST("symbol not found error                     ", test_symnotfound)
    TEST("eval invalid expression                    ", test_eval_invalid)
    TEST("0-params function                          ", test_zeroparam_func)
    TEST("eval list of numbers                       ", test_eval_numberlist)
    TEST("error inside while oper                    ", test_while_error)
    TEST("simple custom oper                         ", test_simple_oper)
    TEST("error inside map oper                      ", test_map_error)
    TEST("add oper with no arguments                 ", test_add_noargs)
    TEST("div oper with too many arguments           ", test_div_toomanyargs)
    TEST("div oper with string arguments             ", test_div_stringargs)
    TEST("div oper with only 1 argument              ", test_div_onearg)
    TEST("sub oper with only 1 argument              ", test_sub_onearg)
    TEST("mult oper with only 1 argument             ", test_mult_onearg)
    TEST("divide by zero                             ", test_div_zero)
    TEST("simple docstring parsing                   ", test_docstring_parse)
    TEST("import test module                         ", test_import_testmod)
    TEST("parse builtin oper                         ", test_parse_builtin)
    TEST("quote builtin oper                         ", test_quote_builtin)
    TEST("import non-existent module                 ", test_import_noexist)
    TEST("docstring in functions                     ", test_docstr_fun)
    TEST("import invalid dynamic library module      ", test_invalid_dylib_import)
    TEST("using oper                                 ", test_using_oper)
    TEST("filtered oper                              ", test_filtered_oper)
    TEST("foreach oper                               ", test_foreach_oper)
    TEST("try oper                                   ", test_try_oper)
    TEST("safe_strcat                                ", test_safe_strcat)
    TEST("split_str                                  ", test_split_str)
    TEST("join_str                                   ", test_join_str)

    fprintf(stderr,"Ran %d tests, %d passed, %d failed\n", total_tests, passed_tests, failed_tests);

    if(failed_tests > 0) {
       return 1;
    } else { 
       return 0;
    }
}

// LCOV_EXCL_STOP
