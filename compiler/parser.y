%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Forward declarations */
extern int  yylex(void);
extern int  line_num;
void        yyerror(const char *msg);

/* Root of the AST */
ASTNode *parse_tree = NULL;

/* Dynamic statement list */
#define MAX_STMTS 1024
static ASTNode *stmt_list[MAX_STMTS];
static int      stmt_count = 0;

/* Dynamic arg list for print */
#define MAX_ARGS 64
static ASTNode *arg_list[MAX_ARGS];
static int      arg_count = 0;
%}

/* ─── Value types ────────────────────────────────────────── */
%union {
    int      ival;
    float    fval;
    char    *sval;
    ASTNode *node;
    DataType dtype;
}

/* ─── Tokens ─────────────────────────────────────────────── */
%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token <sval> STRING_LIT IDENTIFIER
%token LET PRINT SEMICOLON ASSIGN COMMA
%token TYPE_INT TYPE_FLOAT TYPE_STRING
%token LPAREN RPAREN

/* ─── Operator precedence (low → high) ───────────────────── */
%left  PLUS MINUS
%left  STAR SLASH
%token PLUS MINUS STAR SLASH

/* ─── Non-terminal types ─────────────────────────────────── */
%type <node>  stmt expr
%type <dtype> type_spec

%%

program
    : stmt_list     { parse_tree = make_program(stmt_list, stmt_count); }
    ;

stmt_list
    : /* empty */
    | stmt_list stmt    {
                            if (stmt_count < MAX_STMTS)
                                stmt_list[stmt_count++] = $2;
                        }
    ;

stmt
    : LET IDENTIFIER type_spec ASSIGN expr SEMICOLON
                        { $$ = make_var_decl($2, $3, $5); free($2); }
    | IDENTIFIER ASSIGN expr SEMICOLON
                        { $$ = make_assign($1, $3); free($1); }
    | PRINT LPAREN arg_list_expr RPAREN SEMICOLON
                        {
                            $$ = make_print(arg_list, arg_count);
                            arg_count = 0;
                        }
    ;

arg_list_expr
    : expr              { arg_list[arg_count++] = $1; }
    | arg_list_expr COMMA expr
                        { arg_list[arg_count++] = $3; }
    ;

type_spec
    : TYPE_INT      { $$ = TYPE_INT_T;    }
    | TYPE_FLOAT    { $$ = TYPE_FLOAT_T;  }
    | TYPE_STRING   { $$ = TYPE_STRING_T; }
    ;

expr
    : INT_LIT           { $$ = make_int_lit($1);    }
    | FLOAT_LIT         { $$ = make_float_lit($1);  }
    | STRING_LIT        { $$ = make_string_lit($1); free($1); }
    | IDENTIFIER        { $$ = make_identifier($1); free($1); }
    | expr PLUS  expr   { $$ = make_binop(OP_ADD, $1, $3); }
    | expr MINUS expr   { $$ = make_binop(OP_SUB, $1, $3); }
    | expr STAR  expr   { $$ = make_binop(OP_MUL, $1, $3); }
    | expr SLASH expr   { $$ = make_binop(OP_DIV, $1, $3); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *msg) {
    fprintf(stderr, "Parse error at line %d: %s\n", line_num, msg);
}
