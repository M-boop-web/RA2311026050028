#include "codegen.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Symbol table (simple) ──────────────────────────────── */
#define MAX_SYMS 512
typedef struct { char name[64]; DataType type; } Symbol;
static Symbol sym_table[MAX_SYMS];
static int    sym_count = 0;

static void sym_add(const char *name, DataType type) {
    if (sym_count >= MAX_SYMS) { fprintf(stderr, "Symbol table full\n"); exit(1); }
    strncpy(sym_table[sym_count].name, name, 63);
    sym_table[sym_count].type = type;
    sym_count++;
}

static DataType sym_lookup(const char *name) {
    for (int i = 0; i < sym_count; i++)
        if (strcmp(sym_table[i].name, name) == 0)
            return sym_table[i].type;
    return TYPE_UNKNOWN;
}

/* ─── Emit helpers ───────────────────────────────────────── */
static const char *c_type(DataType t) {
    switch (t) {
        case TYPE_INT_T:    return "int";
        case TYPE_FLOAT_T:  return "float";
        case TYPE_STRING_T: return "char*";
        default:            return "void*";
    }
}

static DataType infer_type(ASTNode *node) {
    if (!node) return TYPE_UNKNOWN;
    switch (node->type) {
        case NODE_INT_LIT:    return TYPE_INT_T;
        case NODE_FLOAT_LIT:  return TYPE_FLOAT_T;
        case NODE_STRING_LIT: return TYPE_STRING_T;
        case NODE_IDENTIFIER: return sym_lookup(node->ident.name);
        case NODE_BINOP: {
            DataType l = infer_type(node->binop.left);
            DataType r = infer_type(node->binop.right);
            if (l == TYPE_FLOAT_T || r == TYPE_FLOAT_T) return TYPE_FLOAT_T;
            return TYPE_INT_T;
        }
        default: return TYPE_UNKNOWN;
    }
}

/* ─── Expression emitter ─────────────────────────────────── */
static void emit_expr(FILE *out, ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_INT_LIT:
            fprintf(out, "%d", node->ival);
            break;
        case NODE_FLOAT_LIT:
            fprintf(out, "%ff", node->fval);
            break;
        case NODE_STRING_LIT:
            fprintf(out, "\"%s\"", node->sval);
            break;
        case NODE_IDENTIFIER:
            fprintf(out, "%s", node->ident.name);
            break;
        case NODE_BINOP: {
            char op = '+';
            switch (node->binop.op) {
                case OP_ADD: op = '+'; break;
                case OP_SUB: op = '-'; break;
                case OP_MUL: op = '*'; break;
                case OP_DIV: op = '/'; break;
            }
            fprintf(out, "(");
            emit_expr(out, node->binop.left);
            fprintf(out, " %c ", op);
            emit_expr(out, node->binop.right);
            fprintf(out, ")");
            break;
        }
        default:
            fprintf(out, "/*unknown expr*/");
            break;
    }
}

/* ─── Print format selector ──────────────────────────────── */
static const char *fmt_for(DataType t) {
    switch (t) {
        case TYPE_INT_T:    return "%d";
        case TYPE_FLOAT_T:  return "%g";
        case TYPE_STRING_T: return "%s";
        default:            return "%p";
    }
}

/* ─── Statement emitter ──────────────────────────────────── */
static void emit_stmt(FILE *out, ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_VAR_DECL: {
            DataType t = node->var_decl.var_type;
            sym_add(node->var_decl.name, t);
            fprintf(out, "    %s %s = ", c_type(t), node->var_decl.name);
            if (node->var_decl.init_expr)
                emit_expr(out, node->var_decl.init_expr);
            else {
                if (t == TYPE_INT_T)    fprintf(out, "0");
                else if (t == TYPE_FLOAT_T) fprintf(out, "0.0f");
                else fprintf(out, "\"\"");
            }
            fprintf(out, ";\n");
            break;
        }
        case NODE_ASSIGN: {
            DataType t = sym_lookup(node->assign.name);
            if (t == TYPE_UNKNOWN) {
                /* auto-infer on re-assignment */
                t = infer_type(node->assign.expr);
                sym_add(node->assign.name, t);
                fprintf(out, "    %s %s = ", c_type(t), node->assign.name);
            } else {
                fprintf(out, "    %s = ", node->assign.name);
            }
            emit_expr(out, node->assign.expr);
            fprintf(out, ";\n");
            break;
        }
        case NODE_PRINT: {
            /* Build format string */
            fprintf(out, "    printf(\"");
            for (int i = 0; i < node->print.count; i++) {
                DataType t = infer_type(node->print.args[i]);
                if (node->print.args[i]->type == NODE_STRING_LIT)
                    fprintf(out, "%%s");
                else
                    fprintf(out, "%s", fmt_for(t));
                if (i < node->print.count - 1) fprintf(out, " ");
            }
            fprintf(out, "\\n\"");
            for (int i = 0; i < node->print.count; i++) {
                fprintf(out, ", ");
                emit_expr(out, node->print.args[i]);
            }
            fprintf(out, ");\n");
            break;
        }
        default:
            fprintf(out, "    /* unknown stmt */\n");
            break;
    }
}

/* ─── Top-level codegen ──────────────────────────────────── */
void codegen(ASTNode *root, FILE *out) {
    if (!root || root->type != NODE_PROGRAM) {
        fprintf(stderr, "Codegen: expected program node\n");
        return;
    }

    fprintf(out, "/* Auto-generated by DSL Compiler */\n");
    fprintf(out, "#include <stdio.h>\n\n");
    fprintf(out, "int main(void) {\n");

    for (int i = 0; i < root->program.count; i++)
        emit_stmt(out, root->program.stmts[i]);

    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
}
