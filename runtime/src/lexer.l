%{
#include <stdio.h>
%}

LEFT_PAREN      "("
RIGHT_PAREN     ")"
STRING          \"(\\.|[^"\\])*\"
NUMBER_FLOAT    [0-9]+\.[0-9]+
NUMBER_INTEGER  [0-9]+ 
WHITESPACE      [ \t\n\r]
SYMBOL          [\*\-\/\+a-zA-Z0-9]+
COMMENT_START   \;

%%

{STRING}           { printf("STRING(%s) ", yytext); }
{WHITESPACE}       { /* do nothing */ }
{COMMENT_START}.*$ { /* do nothing */ }
{LEFT_PAREN}       { printf("LEFT_PAREN "); }
{RIGHT_PAREN}      { printf("RIGHT_PAREN "); }
{NUMBER_FLOAT}     { printf("NUMBER_FLOAT(%s) ", yytext); }
{NUMBER_INTEGER}   { printf("NUMBER_INTEGER(%s) ", yytext); }
{SYMBOL}           { printf("SYMBOL(%s) ", yytext); }

%%

