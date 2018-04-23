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
#include <bearlang/builtins.h>

bl_val_t n_symbol = {.type  = BL_VAL_TYPE_SYMBOL,
	             .s_val = "n"};


bl_val_t a_symbol = {.type  = BL_VAL_TYPE_SYMBOL,
	             .s_val = "a"};

bl_val_t cons_wrapped_a = {.type = BL_VAL_TYPE_CONS,
	                          .car  = &a_symbol};

bl_val_t fac1_funcargs_ptr = {.type = BL_VAL_TYPE_CONS,
	                      .car  = &n_symbol,
                              .cdr  = &cons_wrapped_a};

bl_val_t if_oper = {.type = BL_VAL_TYPE_OPER_IF};

bl_val_t eq_symbol = {.type  = BL_VAL_TYPE_OPER_NATIVE,
	              .code_ptr = &bl_oper_eq};

bl_val_t number_1 = {.type = BL_VAL_TYPE_NUMBER,
	             .i_val = 1};

bl_val_t cons_single_1 = {.type = BL_VAL_TYPE_CONS,
	                  .car  = &number_1};

bl_val_t n1_statement = {.type = BL_VAL_TYPE_CONS,
	                 .car  = &n_symbol,
			 .cdr  = &cons_single_1};

bl_val_t eq_n1_statement = {.type = BL_VAL_TYPE_CONS,
	                    .car  = &eq_symbol,
                            .cdr  = &n1_statement};



bl_val_t sym_minus = {.type  = BL_VAL_TYPE_OPER_NATIVE,
	              .code_ptr = &bl_oper_sub};

bl_val_t cons_n_minus1 = {.type = BL_VAL_TYPE_CONS,
	                  .car  = &sym_minus,
                          .cdr  = &n1_statement};

bl_val_t sym_mult = {.type = BL_VAL_TYPE_OPER_NATIVE,
	             .code_ptr = &bl_oper_mult};

bl_val_t cons_mult_na = {.type = BL_VAL_TYPE_CONS,
	                 .car  = &sym_mult,
                         .cdr  = &fac1_funcargs_ptr};

bl_val_t cons_cons_mult_na = {.type = BL_VAL_TYPE_CONS,
	                      .car  = &cons_mult_na,
			      };

bl_val_t cons_cons_n_minus1 = {.type = BL_VAL_TYPE_CONS,
	                       .car  = &cons_n_minus1,
                               .cdr  = &cons_cons_mult_na};

bl_val_t fac1;

bl_val_t cons_if_else_fac = {.type = BL_VAL_TYPE_CONS,
	                     .car  = &fac1,
                             .cdr  = &cons_cons_n_minus1};

bl_val_t cons_cons_if_else_fac = {.type = BL_VAL_TYPE_CONS,
	                          .car  = &cons_if_else_fac,
                                  .cdr  = NULL};

bl_val_t cons_if_then_a = {.type = BL_VAL_TYPE_CONS,
	                   .car  = &a_symbol,
                           .cdr  = &cons_cons_if_else_fac};

bl_val_t eq_n1_cons = {.type = BL_VAL_TYPE_CONS,
	               .car  = &eq_n1_statement,
                       .cdr  = &cons_if_then_a};

bl_val_t if_statement = {.type = BL_VAL_TYPE_CONS,
	                 .car  = &if_oper,
                         .cdr  = &eq_n1_cons};

bl_val_t fac1_funcptr = {.type = BL_VAL_TYPE_CONS,
	                 .car  = &if_statement}; 

bl_val_t eval_ctx = {.type = BL_VAL_TYPE_CTX};

bl_val_t fac1 = {.type=BL_VAL_TYPE_FUNC_BL,
                 .bl_func_ptr     = &fac1_funcptr,
                 .bl_funcargs_ptr = &fac1_funcargs_ptr,
                 .lexical_closure = &eval_ctx,
	         .sym             = NULL};

bl_val_t number_input = {.type = BL_VAL_TYPE_NUMBER,
	              .i_val = 50};

bl_val_t params_cons = {.type = BL_VAL_TYPE_CONS,
	                .car  = &number_input,
			.cdr  = &cons_single_1};

bl_val_t run_expr = {.type = BL_VAL_TYPE_CONS,
	             .car  = &fac1,
		     .cdr  = &params_cons};

int main(int argc, char** argv) {
    bl_init();

    bl_val_t* resp = bl_ctx_eval(&eval_ctx,&run_expr);
    printf("%llu", resp->i_val);

}
