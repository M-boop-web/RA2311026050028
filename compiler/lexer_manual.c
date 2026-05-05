/*
 * lexer_manual.c — hand-written tokenizer that mirrors what Flex would generate
 * from lexer.l. Used when flex is not available on the build machine.
 */
#include "ast.h"
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Custom strndup for MinGW compatibility */
static char *my_strndup(const char *s, size_t n) {
    char *res = malloc(n + 1);
    if (res) {
        strncpy(res, s, n);
        res[n] = '\0';
    }
    return res;
}

int    line_num = 1;
FILE  *yyin     = NULL;

/* ── tiny input buffer ─────────────────────────────────── */
#define BUF_SZ (1 << 20)   /* 1 MB */
static char  src_buf[BUF_SZ];
static int   src_pos = 0;
static int   src_len = 0;

static int peek(void) {
    return (src_pos < src_len) ? (unsigned char)src_buf[src_pos] : EOF;
}
static int advance(void) {
    int c = (src_pos < src_len) ? (unsigned char)src_buf[src_pos++] : EOF;
    if (c == '\n') line_num++;
    return c;
}

/* Bison expects yylval */
YYSTYPE yylval;

/* ── keyword table ─────────────────────────────────────── */
static struct { const char *kw; int tok; } keywords[] = {
    {"let",    LET},
    {"print",  PRINT},
    {"int",    TYPE_INT},
    {"float",  TYPE_FLOAT},
    {"string", TYPE_STRING},
    {NULL, 0}
};

void yylex_init_buffer(void) {
    if (!yyin) { fprintf(stderr, "yyin not set\n"); exit(1); }
    src_len = fread(src_buf, 1, BUF_SZ - 1, yyin);
    src_buf[src_len] = '\0';
    src_pos = 0;
    line_num = 1;
}

int yylex(void) {
    static int initialised = 0;
    if (!initialised) { yylex_init_buffer(); initialised = 1; }

    while (src_pos < src_len) {
        char c = peek();

        /* whitespace */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(); continue;
        }

        /* line comment */
        if (c == '/' && src_pos+1 < src_len && src_buf[src_pos+1] == '/') {
            while (peek() != '\n' && peek() != EOF) advance();
            continue;
        }

        /* single-char tokens */
        switch (c) {
            case '=': advance(); return ASSIGN;
            case '+': advance(); return PLUS;
            case '-': advance(); return MINUS;
            case '*': advance(); return STAR;
            case '/': advance(); return SLASH;
            case '(': advance(); return LPAREN;
            case ')': advance(); return RPAREN;
            case ';': advance(); return SEMICOLON;
            case ',': advance(); return COMMA;
        }

        /* string literal */
        if (c == '"') {
            advance(); /* skip opening " */
            int start = src_pos;
            while (peek() != '"' && peek() != EOF) advance();
            int len = src_pos - start;
            yylval.sval = my_strndup(src_buf + start, len);
            advance(); /* skip closing " */
            return STRING_LIT;
        }

        /* number */
        if (isdigit(c)) {
            int start = src_pos;
            while (isdigit(peek())) advance();
            int is_float = 0;
            if (peek() == '.') {
                is_float = 1;
                advance();
                while (isdigit(peek())) advance();
            }
            char tmp[64];
            int  len = src_pos - start;
            strncpy(tmp, src_buf + start, len); tmp[len] = '\0';
            if (is_float) { yylval.fval = atof(tmp); return FLOAT_LIT; }
            else          { yylval.ival = atoi(tmp); return INT_LIT;   }
        }

        /* identifier / keyword */
        if (isalpha(c) || c == '_') {
            int start = src_pos;
            while (isalnum(peek()) || peek() == '_') advance();
            int  len = src_pos - start;
            char word[64];
            strncpy(word, src_buf + start, len); word[len] = '\0';

            for (int i = 0; keywords[i].kw; i++)
                if (strcmp(word, keywords[i].kw) == 0)
                    return keywords[i].tok;

            yylval.sval = strdup(word);
            return IDENTIFIER;
        }

        fprintf(stderr, "Lexer: unknown char '%c' at line %d\n", c, line_num);
        advance();
    }
    return 0; /* EOF */
}
