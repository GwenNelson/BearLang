%{
#include <stdio.h>
#include <bearlang/types.h>
#include <bearlang/common.h>
%}

%option reentrant
%option yylineno
%option noyyalloc
%option noyyrealloc
%option noyyfree


LEFT_PAREN      "("
RIGHT_PAREN     ")"
IF_OPER		"if"
DO_OPER		"do"
WHILE_OPER	"while"
DOCSTR          \"\"\"(\\.|[^"\\])*\"\"\"
STRING          \"(\\.|[^"\\])*\"
NUMBER_INTEGER  [-]*[0-9]+ 
WHITESPACE      [ \t\n\r]
SYMBOL          [\@\_\%\:\=\*\-\/\+a-zA-Z0-9]+
QUOTED_SYMBOL   "'"[\@\_\%\:\=\*\-\/\+a-zA-Z0-9]+
COMMENT_START   \;

%%
{DOCSTR}	   { memmove(yyget_text(yyscanner),yyget_text(yyscanner)+3,yyget_leng(yyscanner)-3);
                     yyget_text(yyscanner)[yyget_leng(yyscanner)-6]='\0';  return BL_TOKEN_DOCSTRING; }
{STRING}           { memmove(yyget_text(yyscanner),yyget_text(yyscanner)+1,yyget_leng(yyscanner)-1);
                     yyget_text(yyscanner)[yyget_leng(yyscanner)-2]='\0'; return BL_TOKEN_STRING; }
{WHITESPACE}       { /* do nothing */ }
{COMMENT_START}.*$ { /* do nothing */ }
{LEFT_PAREN}       { return BL_TOKEN_LPAREN; }
{RIGHT_PAREN}      { return BL_TOKEN_RPAREN; }
{NUMBER_INTEGER}   { return BL_TOKEN_INTEGER; }
{IF_OPER}	   { return BL_TOKEN_IF; }
{DO_OPER}	   { return BL_TOKEN_DO; }
{WHILE_OPER}	   { return BL_TOKEN_WHILE; }
{QUOTED_SYMBOL}    { return BL_TOKEN_SYMBOL; }
{SYMBOL}           { return BL_TOKEN_SYMBOL; }


%%

//LCOV_EXCL_START
void* yyalloc(size_t bytes, void* yyscanner) {
      return GC_MALLOC(bytes);
}

void* yyrealloc(void* ptr, size_t bytes, void* yyscanner) {
      return GC_REALLOC(ptr, bytes);
}

void yyfree(void* ptr, void* yyscanner) {
     GC_FREE(ptr);
}
//LCOV_EXCL_STOP
