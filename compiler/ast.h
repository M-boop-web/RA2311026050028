#ifndef AST_H
#define AST_H

/* ─── Node Types ─────────────────────────────────────────── */
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_BINOP,
    NODE_INT_LIT,
    NODE_FLOAT_LIT,
    NODE_STRING_LIT,
    NODE_IDENTIFIER
} NodeType;

typedef enum {
    TYPE_INT_T,
    TYPE_FLOAT_T,
    TYPE_STRING_T,
    TYPE_UNKNOWN
} DataType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} BinOp;

/* ─── AST Node ───────────────────────────────────────────── */
typedef struct ASTNode {
    NodeType type;
    DataType dtype;

    union {
        /* Literals */
        int   ival;
        float fval;
        char *sval;

        /* Variable declaration */
        struct {
            char          *name;
            DataType       var_type;
            struct ASTNode *init_expr;
        } var_decl;

        /* Assignment */
        struct {
            char          *name;
            struct ASTNode *expr;
        } assign;

        /* Print statement */
        struct {
            struct ASTNode **args;
            int              count;
        } print;

        /* Binary operation */
        struct {
            BinOp          op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;

        /* Identifier */
        struct {
            char *name;
        } ident;

        /* Program root */
        struct {
            struct ASTNode **stmts;
            int              count;
        } program;
    };
} ASTNode;

/* ─── Constructors ───────────────────────────────────────── */
ASTNode *make_int_lit    (int val);
ASTNode *make_float_lit  (float val);
ASTNode *make_string_lit (char *val);
ASTNode *make_identifier (char *name);
ASTNode *make_binop      (BinOp op, ASTNode *left, ASTNode *right);
ASTNode *make_var_decl   (char *name, DataType type, ASTNode *init);
ASTNode *make_assign     (char *name, ASTNode *expr);
ASTNode *make_print      (ASTNode **args, int count);
ASTNode *make_program    (ASTNode **stmts, int count);

/* ─── Utils ──────────────────────────────────────────────── */
void free_ast (ASTNode *node);
void print_ast(ASTNode *node, int indent);

#endif /* AST_H */
