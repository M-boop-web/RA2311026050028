#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Helper ─────────────────────────────────────────────── */
static ASTNode *alloc_node(NodeType type) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    if (!n) { fprintf(stderr, "Out of memory\n"); exit(1); }
    n->type  = type;
    n->dtype = TYPE_UNKNOWN;
    return n;
}

/* ─── Constructors ───────────────────────────────────────── */
ASTNode *make_int_lit(int val) {
    ASTNode *n = alloc_node(NODE_INT_LIT);
    n->ival    = val;
    n->dtype   = TYPE_INT_T;
    return n;
}

ASTNode *make_float_lit(float val) {
    ASTNode *n = alloc_node(NODE_FLOAT_LIT);
    n->fval    = val;
    n->dtype   = TYPE_FLOAT_T;
    return n;
}

ASTNode *make_string_lit(char *val) {
    ASTNode *n = alloc_node(NODE_STRING_LIT);
    n->sval    = strdup(val);
    n->dtype   = TYPE_STRING_T;
    return n;
}

ASTNode *make_identifier(char *name) {
    ASTNode *n    = alloc_node(NODE_IDENTIFIER);
    n->ident.name = strdup(name);
    return n;
}

ASTNode *make_binop(BinOp op, ASTNode *left, ASTNode *right) {
    ASTNode *n    = alloc_node(NODE_BINOP);
    n->binop.op   = op;
    n->binop.left = left;
    n->binop.right= right;
    return n;
}

ASTNode *make_var_decl(char *name, DataType type, ASTNode *init) {
    ASTNode *n          = alloc_node(NODE_VAR_DECL);
    n->var_decl.name    = strdup(name);
    n->var_decl.var_type= type;
    n->var_decl.init_expr = init;
    return n;
}

ASTNode *make_assign(char *name, ASTNode *expr) {
    ASTNode *n     = alloc_node(NODE_ASSIGN);
    n->assign.name = strdup(name);
    n->assign.expr = expr;
    return n;
}

ASTNode *make_print(ASTNode **args, int count) {
    ASTNode *n      = alloc_node(NODE_PRINT);
    n->print.count  = count;
    n->print.args   = malloc(sizeof(ASTNode *) * count);
    memcpy(n->print.args, args, sizeof(ASTNode *) * count);
    return n;
}

ASTNode *make_program(ASTNode **stmts, int count) {
    ASTNode *n        = alloc_node(NODE_PROGRAM);
    n->program.count  = count;
    n->program.stmts  = malloc(sizeof(ASTNode *) * count);
    memcpy(n->program.stmts, stmts, sizeof(ASTNode *) * count);
    return n;
}

/* ─── Debug printer ──────────────────────────────────────── */
static const char *op_str(BinOp op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
    }
    return "?";
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program (%d stmts)\n", node->program.count);
            for (int i = 0; i < node->program.count; i++)
                print_ast(node->program.stmts[i], indent + 1);
            break;
        case NODE_VAR_DECL:
            printf("VarDecl: %s\n", node->var_decl.name);
            print_ast(node->var_decl.init_expr, indent + 1);
            break;
        case NODE_ASSIGN:
            printf("Assign: %s\n", node->assign.name);
            print_ast(node->assign.expr, indent + 1);
            break;
        case NODE_PRINT:
            printf("Print (%d args)\n", node->print.count);
            for (int i = 0; i < node->print.count; i++)
                print_ast(node->print.args[i], indent + 1);
            break;
        case NODE_BINOP:
            printf("BinOp: %s\n", op_str(node->binop.op));
            print_ast(node->binop.left,  indent + 1);
            print_ast(node->binop.right, indent + 1);
            break;
        case NODE_INT_LIT:    printf("IntLit: %d\n",  node->ival); break;
        case NODE_FLOAT_LIT:  printf("FloatLit: %f\n",node->fval); break;
        case NODE_STRING_LIT: printf("StrLit: \"%s\"\n", node->sval); break;
        case NODE_IDENTIFIER: printf("Ident: %s\n",   node->ident.name); break;
    }
}

/* ─── Memory cleanup ─────────────────────────────────────── */
void free_ast(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->program.count; i++)
                free_ast(node->program.stmts[i]);
            free(node->program.stmts);
            break;
        case NODE_VAR_DECL:
            free(node->var_decl.name);
            free_ast(node->var_decl.init_expr);
            break;
        case NODE_ASSIGN:
            free(node->assign.name);
            free_ast(node->assign.expr);
            break;
        case NODE_PRINT:
            for (int i = 0; i < node->print.count; i++)
                free_ast(node->print.args[i]);
            free(node->print.args);
            break;
        case NODE_BINOP:
            free_ast(node->binop.left);
            free_ast(node->binop.right);
            break;
        case NODE_STRING_LIT:
        case NODE_IDENTIFIER:
            free(node->sval);
            break;
        default: break;
    }
    free(node);
}
