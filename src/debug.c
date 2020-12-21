#include "peachcc.h"

static void dump_stmt(Stmt *s, int indent);
static void dump_expr(Expr *e, int indent);

void error_at(char *loc, char *fmt, ...)
{
    assert(loc);
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - c_program_g;
    fprintf(stderr, "%s\n", c_program_g);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}
void dump_ast(Program *program)
{
    assert(program);
    for (size_t i = 0; i < program->stmts->len; i++)
    {
        Stmt *s = (Stmt *)(program->stmts->data[i]);
        dump_stmt(s, 4);
    }
}

static void dump_stmt(Stmt *s, int indent)
{
    assert(s);
    switch (s->kind)
    {
    case ST_EXPR:
        fprintf(stderr, "ExprStmt(");
        dump_expr(s->expr, indent);
        fprintf(stderr, ");\n");
        break;
    }
}

// Exprを標準エラー出力にダンプする
static void dump_expr(Expr *e, int indent)
{
    assert(e);
    switch (e->kind)
    {
    case EX_ASSIGN:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " = ");
        dump_expr(e->rhs, indent);
        break;
    case EX_ADD:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " + ");
        dump_expr(e->rhs, indent);
        break;
    case EX_SUB:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " - ");
        dump_expr(e->rhs, indent);
        break;
    case EX_MUL:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " * ");
        dump_expr(e->rhs, indent);
        break;
    case EX_DIV:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " / ");
        dump_expr(e->rhs, indent);
        break;
    case EX_LE:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " < ");
        dump_expr(e->rhs, indent);
        break;
    case EX_GE:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " > ");
        dump_expr(e->rhs, indent);
        break;
    case EX_LEEQ:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " <= ");
        dump_expr(e->rhs, indent);
        break;
    case EX_GEEQ:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " >= ");
        dump_expr(e->rhs, indent);
        break;
    case EX_EQ:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " == ");
        dump_expr(e->rhs, indent);
        break;
    case EX_NTEQ:
        dump_expr(e->lhs, indent);
        fprintf(stderr, " != ");
        dump_expr(e->rhs, indent);
        break;
    case EX_UNARY_PLUS:
        fprintf(stderr, "+ ");
        dump_expr(e->unary_op, indent);
        break;
    case EX_UNARY_MINUS:
        fprintf(stderr, "- ");
        dump_expr(e->unary_op, indent);
        break;
    case EX_INTEGER:
        fprintf(stderr, "%d", e->value);
        break;
    case EX_LOCAL_VAR:
    {
        char buf[1024];
        strncpy(buf, e->str, e->length);
        buf[e->length] = 0;
        fprintf(stderr, "%s", buf);
        break;
    }
    }
}