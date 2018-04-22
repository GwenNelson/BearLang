#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bearlang/common.h>
#include <bearlang/types.h>

#include <bl_lexer.h>

int main(int argc, char** argv) {
    GC_INIT();
    bl_init();

    yyscan_t scanner;
    yylex_init(&scanner);



     bl_token_type_t tok;
    while((tok=yylex(scanner))) {
    	switch(tok) {
		case BL_TOKEN_STRING:
			printf("STRING(%s) ", yyget_text(scanner));
		break;
		case BL_TOKEN_LPAREN:
			printf("LEFT_PAREN ");
		break;
		case BL_TOKEN_RPAREN:
			printf("RIGHT_PAREN ");
		break;
		case BL_TOKEN_FLOAT:
			printf("BL_TOKEN_NUMBER_FLOAT(%s) ", yyget_text(scanner));
		break;
		case BL_TOKEN_INTEGER:
			printf("BL_TOKEN_NUMBER_INTEGER(%s) ", yyget_text(scanner));
		break;
		case BL_TOKEN_SYMBOL:
			printf("BL_TOKEN_SYMBOL(%s) ", yyget_text(scanner));
		break;

	}
    }
    
    
    yylex_destroy(scanner);
    return 0;

}
