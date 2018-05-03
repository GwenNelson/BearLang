#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/sexp.h>
#include <bearlang/list_ops.h>
#include <bearlang/error_tools.h>
#include <bearlang/ctx.h>
#include <bearlang/utils.h>

#define TEST(desc,f) fprintf(stderr,"Testing: %s \t",desc); if(f()==0) { passed_tests++; fprintf(stderr,"PASS\n");} else { failed_tests++; fprintf(stderr,"FAIL\n");}; total_tests++;

#define ASSERT(desc,cond) if(! (cond)) { fprintf(stderr,"Assert %s failed\t",desc); return 1;}


int test_sexp_parse_list() {
    char* test_list = "(+ 1 2)";

    bl_val_t* sexp = bl_parse_sexp(test_list);

    ASSERT("first item is symbol", sexp->car->type           == BL_VAL_TYPE_SYMBOL)
    ASSERT("second item is int",   sexp->cdr->car->type      == BL_VAL_TYPE_NUMBER)
    ASSERT("second item is int",   sexp->cdr->cdr->car->type == BL_VAL_TYPE_NUMBER)

    return 0;
}

int test_ser_sexp() {
    bl_val_t* sexp = bl_mk_list(3,bl_mk_symbol("+"),bl_mk_integer("1"),bl_mk_integer("2"));

    char* serialised_sexp = bl_ser_sexp(sexp);

    ASSERT("strcmp(sexp,\"(+ 1 2)\")==0", strcmp(serialised_sexp,"(+ 1 2)")==0)

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

    ASSERT("(eq 2 2) is #t", is_eq_result->b_val == true)
    ASSERT("(eq 2 3) is #f", not_eq_result->b_val == false)

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

    bl_val_t* tt_result = bl_ctx_eval(ctx,bl_parse_sexp(tt_str));
    bl_val_t* ff_result = bl_ctx_eval(ctx,bl_parse_sexp(ff_str));
    bl_val_t* tf_result = bl_ctx_eval(ctx,bl_parse_sexp(tf_str));

    ASSERT("(or True True) returns True",    tt_result->b_val==true)
    ASSERT("(or False False) returns False", ff_result->b_val==false)
    ASSERT("(or True False) returns False",  tf_result->b_val==true)

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

    char* first_str   = "(first 1 2 3 4 5)";
    char* second_str  = "(second 1 2 3 4 5)";
    char* third_str   = "(third 1 2 3 4 5)";
    char* rest_str    = "(rest 1 2 3 4 5)";
    char* reverse_str = "(reverse 1 2 3 4 5)";
    char* append_str  = "(append (1 2 3 4) 5";
    char* prepend_str = "(prepend (1 2 3 4) 5";

    bl_val_t* first_result   = bl_ctx_eval(ctx,bl_parse_sexp(first_str));
    bl_val_t* second_result  = bl_ctx_eval(ctx,bl_parse_sexp(second_str));
    bl_val_t* third_result   = bl_ctx_eval(ctx,bl_parse_sexp(third_str));
    bl_val_t* rest_result    = bl_ctx_eval(ctx,bl_parse_sexp(rest_str));
    bl_val_t* reverse_result = bl_ctx_eval(ctx,bl_parse_sexp(reverse_str));
    bl_val_t* append_result  = bl_ctx_eval(ctx,bl_parse_sexp(append_str));
    bl_val_t* prepend_result = bl_ctx_eval(ctx,bl_parse_sexp(prepend_str));

    ASSERT("(first 1 2 3 4 5)  returns 1", strcmp(bl_ser_sexp(first_result),"1")==0)
    ASSERT("(second 1 2 3 4 5) returns 2", strcmp(bl_ser_sexp(second_result),"2")==0)
    ASSERT("(third 1 2 3 4 5)  returns 3", strcmp(bl_ser_sexp(third_result),"3")==0)

    ASSERT("(rest 1 2 3 4 5)  returns (2 3 4 5)", strcmp(bl_ser_sexp(rest_result),"(2 3 4 5)")==0)

    ASSERT("(reverse 1 2 3 4 5)  returns (5 4 3 2 1)", strcmp(bl_ser_sexp(reverse_result),"(5 4 3 2 1)")==0)
    ASSERT("(append (1 2 3 4) 5)  returns (1 2 3 4 5)", strcmp(bl_ser_sexp(append_result),"(1 2 3 4 5)")==0)
    ASSERT("(prepend (1 2 3 4) 5)  returns (5 1 2 3 4)", strcmp(bl_ser_sexp(prepend_result),"(5 1 2 3 4)")==0)
    bl_ctx_close(ctx);
    return 0;
}

int test_parse_string() {
    char* test_str = "\"Hello world\"";
    bl_val_t* parsed = bl_parse_sexp(test_str);
    ASSERT("String parsed OK", strstr(parsed->s_val,"Hello world"))
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
    

    bl_ctx_close(ctx);
    return 0;

}

int main(int argc, char** argv) {
    int passed_tests = 0;
    int failed_tests = 0;
    int total_tests  = 0;

    bl_init();

    TEST("Simple s-expression parse                  ", test_sexp_parse_list)
    TEST("Serialise an s-expression                  ", test_ser_sexp)
    TEST("List ops: first, second and rest           ", test_first_second_rest)
    TEST("List ops: third                            ", test_third)
    TEST("List ops: prepend to NULL                  ", test_prepend_null)
    TEST("List ops: get list length                  ", test_list_len)
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

    fprintf(stderr,"Ran %d tests, %d passed, %d failed\n", total_tests, passed_tests, failed_tests);

    if(failed_tests > 0) {
       return 1;
    } else { 
       return 0;
    }
}
