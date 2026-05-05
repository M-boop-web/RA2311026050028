/*
 * parser_manual.c — recursive-descent parser that implements the same grammar
 * as parser.y, without requiring bison.
 *
 * Grammar (simplified):
 *   program   → stmt*
 *   stmt      → LET ID type ASSIGN expr SEMICOLON
 *             | ID ASSIGN expr SEMICOLON
 *             | PRINT LPAREN arglist RPAREN SEMICOLON
 *   arglist   → expr (COMMA expr)*
 *   expr      → term ((PLUS|MINUS) term)*
 *   term      → factor ((STAR|SLASH) factor)*
 *   factor    → INT_LIT | FLOAT_LIT | STRING_LIT | ID | LPAREN expr RPAREN
 */
#include "ast.h"
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Parser globals ─────────────────────────────────────── */
ASTNode *parse_tree = NULL;
extern int    line_num;
extern FILE  *yyin;
extern int    yylex(void);

static int    cur_tok;
static YYSTYPE cur_val;

/* forward */
static ASTNode *parse_expr(void);
static ASTNode *parse_stmt(void);

static void next_token(void) {
    cur_tok = yylex();
    cur_val = yylval;
}

static void expect(int tok, const char *ctx) {
    if (cur_tok != tok) {
        fprintf(stderr, "Parse error at line %d [%s]: expected token %d, got %d\n",
                line_num, ctx, tok, cur_tok);
        exit(1);
    }
    next_token();
}

/* ── Expressions ────────────────────────────────────────── */
static ASTNode *parse_factor(void) {
    ASTNode *node = NULL;
    switch (cur_tok) {
        case INT_LIT:
            node = make_int_lit(cur_val.ival);
            next_token();
            break;
        case FLOAT_LIT:
            node = make_float_lit(cur_val.fval);
            next_token();
            break;
        case STRING_LIT: {
            char *s = cur_val.sval;
            node = make_string_lit(s);
            free(s);
            next_token();
            break;
        }
        case IDENTIFIER: {
            char *name = cur_val.sval;
            node = make_identifier(name);
            free(name);
            next_token();
            break;
        }
        case LPAREN:
            next_token();
            node = parse_expr();
            expect(RPAREN, "closing paren");
            break;
        default:
            fprintf(stderr, "Parse error at line %d: unexpected token %d in expression\n",
                    line_num, cur_tok);
            exit(1);
    }
    return node;
}

static ASTNode *parse_term(void) {
    ASTNode *left = parse_factor();
    while (cur_tok == STAR || cur_tok == SLASH) {
        BinOp op = (cur_tok == STAR) ? OP_MUL : OP_DIV;
        next_token();
        ASTNode *right = parse_factor();
        left = make_binop(op, left, right);
    }
    return left;
}

static ASTNode *parse_expr(void) {
    ASTNode *left = parse_term();
    while (cur_tok == PLUS || cur_tok == MINUS) {
        BinOp op = (cur_tok == PLUS) ? OP_ADD : OP_SUB;
        next_token();
        ASTNode *right = parse_term();
        left = make_binop(op, left, right);
    }
    return left;
}

/* ── Statements ─────────────────────────────────────────── */
static DataType parse_type(void) {
    DataType t;
    switch (cur_tok) {
        case TYPE_INT:    t = TYPE_INT_T;    break;
        case TYPE_FLOAT:  t = TYPE_FLOAT_T;  break;
        case TYPE_STRING: t = TYPE_STRING_T; break;
        default:
            fprintf(stderr, "Parse error at line %d: expected type, got %d\n",
                    line_num, cur_tok);
            exit(1);
    }
    next_token();
    return t;
}

static ASTNode *parse_stmt(void) {
    if (cur_tok == LET) {
        next_token();
        if (cur_tok != IDENTIFIER) {
            fprintf(stderr, "Parse error line %d: expected identifier after 'let'\n", line_num);
            exit(1);
        }
        char *name = strdup(cur_val.sval);
        free(cur_val.sval);
        next_token();

        DataType t = parse_type();
        expect(ASSIGN, "let =");
        ASTNode *init = parse_expr();
        expect(SEMICOLON, "let ;");
        ASTNode *n = make_var_decl(name, t, init);
        free(name);
        return n;
    }

    if (cur_tok == IDENTIFIER) {
        char *name = strdup(cur_val.sval);
        free(cur_val.sval);
        next_token();

        if (cur_tok != ASSIGN) {
            fprintf(stderr, "Parse error line %d: expected '=' after identifier\n", line_num);
            exit(1);
        }
        next_token();
        ASTNode *expr = parse_expr();
        expect(SEMICOLON, "assign ;");
        ASTNode *n = make_assign(name, expr);
        free(name);
        return n;
    }

    if (cur_tok == PRINT) {
        next_token();
        expect(LPAREN, "print (");

        #define MAX_ARGS 64
        ASTNode *args[MAX_ARGS];
        int      count = 0;

        if (cur_tok != RPAREN) {
            args[count++] = parse_expr();
            while (cur_tok == COMMA) {
                next_token();
                if (count < MAX_ARGS)
                    args[count++] = parse_expr();
            }
        }
        expect(RPAREN, "print )");
        expect(SEMICOLON, "print ;");
        return make_print(args, count);
    }

    fprintf(stderr, "Parse error at line %d: unexpected token %d\n", line_num, cur_tok);
    exit(1);
}

/* ── Entry point ────────────────────────────────────────── */
int yyparse(void) {
    #define MAX_STMTS 1024
    ASTNode *stmts[MAX_STMTS];
    int      count = 0;

    next_token(); /* prime the pump */

    while (cur_tok != 0) { /* 0 = EOF */
        if (count < MAX_STMTS)
            stmts[count++] = parse_stmt();
    }

    parse_tree = make_program(stmts, count);
    return 0;
}
