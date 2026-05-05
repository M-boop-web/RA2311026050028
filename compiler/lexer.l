%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>

int line_num = 1;
%}

%option noyywrap

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT         {DIGIT}+
FLOAT       {DIGIT}+"."{DIGIT}+
STRING      \"[^\"]*\"
WHITESPACE  [ \t\r]+

%%

"let"           { return LET; }
"print"         { return PRINT; }
"int"           { return TYPE_INT; }
"float"         { return TYPE_FLOAT; }
"string"        { return TYPE_STRING; }
"="             { return ASSIGN; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return STAR; }
"/"             { return SLASH; }
"("             { return LPAREN; }
")"             { return RPAREN; }
";"             { return SEMICOLON; }
","             { return COMMA; }

{FLOAT}         {
                    yylval.fval = atof(yytext);
                    return FLOAT_LIT;
                }

{INT}           {
                    yylval.ival = atoi(yytext);
                    return INT_LIT;
                }

{STRING}        {
                    /* strip surrounding quotes */
                    yylval.sval = strndup(yytext + 1, strlen(yytext) - 2);
                    return STRING_LIT;
                }

{ID}            {
                    yylval.sval = strdup(yytext);
                    return IDENTIFIER;
                }

"\n"            { line_num++; }
{WHITESPACE}    { /* skip */ }
"//"[^\n]*      { /* skip line comments */ }

.               {
                    fprintf(stderr, "Lexer error: unknown character '%s' at line %d\n", yytext, line_num);
                }

%%
