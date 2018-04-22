#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bearlang/common.h>
#include <bearlang/types.h>
#include <bearlang/utils.h>
#include <bearlang/list_ops.h>
#include <bearlang/sexp.h>

#include <bl_lexer.h>

bl_val_t* read_form(yyscan_t scanner);

bl_val_t* read_list(yyscan_t scanner) {
    bl_val_t* retval = bl_mk_val(BL_VAL_TYPE_CONS);
    retval->car = read_form(scanner);
    retval->cdr = NULL;

    bl_val_t* L = retval;
    bl_val_t* e = bl_mk_null();
    while(e->type != BL_VAL_TYPE_LIST_END) {
       e = read_form(scanner);
       if(e->type != BL_VAL_TYPE_LIST_END) {
          L->cdr = bl_mk_val(BL_VAL_TYPE_CONS);
	  L->cdr->car = e;
	  L->cdr->cdr = NULL;
	  L = L->cdr;
       }
    }
    return retval;
}

// TODO - make this read from strings, and then replace mpc

bl_val_t* read_form(yyscan_t scanner) {
    bl_token_type_t tok = yylex(scanner);
    if(tok == 0) return NULL;
    switch(tok) {
	case BL_TOKEN_STRING:
		return bl_mk_str(yyget_text(scanner));
	break;
	case BL_TOKEN_LPAREN:
		return read_list(scanner);
	break;
	case BL_TOKEN_RPAREN:
		return bl_mk_val(BL_VAL_TYPE_LIST_END);
	break;
	case BL_TOKEN_FLOAT:
		// TODO: implement floats properly
	//		printf("BL_TOKEN_NUMBER_FLOAT(%s) ", yyget_text(scanner));
		return bl_mk_number(0);
	break;
	case BL_TOKEN_INTEGER:
		return bl_mk_number(atoi(yyget_text(scanner)));
	break;
	case BL_TOKEN_SYMBOL:
		return bl_mk_symbol(yyget_text(scanner));
	break;
   }
 
}

int main(int argc, char** argv) {
    GC_INIT();
    bl_init();

    yyscan_t scanner;
    yylex_init(&scanner);

    bl_val_t* E=bl_mk_null();

    while((E = read_form(scanner))) {
        printf("%s\n",bl_ser_sexp(E));
    }
    yylex_destroy(scanner);
    return 0;

}
