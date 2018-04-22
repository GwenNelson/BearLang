%{
#include <stdio.h>
#include <bearlang/types.h>
%}

%option reentrant
%option yylineno

LEFT_PAREN      "("
RIGHT_PAREN     ")"
STRING          \"(\\.|[^"\\])*\"
NUMBER_FLOAT    [0-9]+\.[0-9]+
NUMBER_INTEGER  [0-9]+ 
WHITESPACE      [ \t\n\r]
SYMBOL          [\*\-\/\+a-zA-Z0-9]+
COMMENT_START   \;

%%
{STRING}           { memmove(yyget_text(yyscanner),yyget_text(yyscanner)+1,yyget_leng(yyscanner)-1);
                     yyget_text(yyscanner)[yyget_leng(yyscanner)-2]='\0'; return BL_TOKEN_STRING; }
{WHITESPACE}       { /* do nothing */ }
{COMMENT_START}.*$ { /* do nothing */ }
{LEFT_PAREN}       { return BL_TOKEN_LPAREN; }
{RIGHT_PAREN}      { return BL_TOKEN_RPAREN; }
{NUMBER_FLOAT}     { return BL_TOKEN_FLOAT; }
{NUMBER_INTEGER}   { return BL_TOKEN_INTEGER; }
{SYMBOL}           { return BL_TOKEN_SYMBOL; }

%%
