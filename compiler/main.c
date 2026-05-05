#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "codegen.h"

/* Bison/Flex API */
extern int   yyparse(void);
extern FILE *yyin;
extern ASTNode *parse_tree;

/* ─── Semantic analysis (type checking pass) ─────────────── */
static void semantic_check(ASTNode *node, int depth) {
    if (!node) return;
    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->program.count; i++)
                semantic_check(node->program.stmts[i], depth);
            break;
        case NODE_BINOP:
            semantic_check(node->binop.left,  depth+1);
            semantic_check(node->binop.right, depth+1);
            /* Warn on potential string arithmetic */
            if (node->binop.left->type == NODE_STRING_LIT ||
                node->binop.right->type == NODE_STRING_LIT) {
                fprintf(stderr, "Warning: arithmetic on string literal\n");
            }
            break;
        case NODE_VAR_DECL:
            semantic_check(node->var_decl.init_expr, depth+1);
            break;
        case NODE_ASSIGN:
            semantic_check(node->assign.expr, depth+1);
            break;
        case NODE_PRINT:
            for (int i = 0; i < node->print.count; i++)
                semantic_check(node->print.args[i], depth+1);
            break;
        default: break;
    }
}

int main(int argc, char *argv[]) {
    /* Usage: dslc <input.dsl> [-o output.c] [-ast] */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.dsl> [-o output.c] [-ast]\n", argv[0]);
        return 1;
    }

    const char *input_file  = argv[1];
    const char *output_file = "out.c";
    int         show_ast    = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-ast") == 0) show_ast = 1;
        if (strcmp(argv[i], "-o") == 0 && i+1 < argc) output_file = argv[++i];
    }

    /* Open input */
    yyin = fopen(input_file, "r");
    if (!yyin) {
        perror("Cannot open input file");
        return 1;
    }

    /* Parse */
    if (yyparse() != 0) {
        fprintf(stderr, "Parsing failed.\n");
        fclose(yyin);
        return 1;
    }
    fclose(yyin);

    if (!parse_tree) {
        fprintf(stderr, "No AST produced.\n");
        return 1;
    }

    /* Semantic check */
    semantic_check(parse_tree, 0);

    /* Dump AST if requested */
    if (show_ast) {
        fprintf(stderr, "\n── AST ──────────────────────\n");
        print_ast(parse_tree, 0);
        fprintf(stderr, "─────────────────────────────\n\n");
    }

    /* Code generation */
    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("Cannot open output file");
        return 1;
    }
    codegen(parse_tree, out);
    fclose(out);

    fprintf(stderr, "Compiled '%s' → '%s'\n", input_file, output_file);

    /* Automatic GCC compilation */
    #define GCC_PATH "D:\\C data\\Downloads\\MinGW\\bin\\gcc.exe"
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "\"%s\" %s -o out.exe", GCC_PATH, output_file);
    fprintf(stderr, "Running GCC: %s\n", cmd);
    int res = system(cmd);
    if (res == 0) {
        fprintf(stderr, "GCC Compilation Successful → 'out.exe'\n");
    } else {
        fprintf(stderr, "GCC Compilation Failed (exit code %d)\n", res);
    }

    free_ast(parse_tree);
    return 0;
}
