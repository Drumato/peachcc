#include "debug.h"
#include "peachcc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

// ASTを標準エラー出力にダンプする
// ASTの各ノードに適切なトークンが付与されているかどうかもチェックする．
void dump_ast(Expr *e, int indent)
{
    switch (e->kind)
    {
    case EX_ADD:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " + ");
        dump_ast(e->rhs, indent);
        break;
    case EX_SUB:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " - ");
        dump_ast(e->rhs, indent);
        break;
    case EX_MUL:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " * ");
        dump_ast(e->rhs, indent);
        break;
    case EX_DIV:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " / ");
        dump_ast(e->rhs, indent);
        break;
    case EX_LE:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " < ");
        dump_ast(e->rhs, indent);
        break;
    case EX_GE:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " > ");
        dump_ast(e->rhs, indent);
        break;
    case EX_LEEQ:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " <= ");
        dump_ast(e->rhs, indent);
        break;
    case EX_GEEQ:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " >= ");
        dump_ast(e->rhs, indent);
        break;
    case EX_EQ:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " == ");
        dump_ast(e->rhs, indent);
        break;
    case EX_NTEQ:
        dump_ast(e->lhs, indent);
        fprintf(stderr, " != ");
        dump_ast(e->rhs, indent);
        break;
    case EX_UNARY_PLUS:
        fprintf(stderr, "+ ");
        dump_ast(e->unary_op, indent);
        break;
    case EX_UNARY_MINUS:
        fprintf(stderr, "- ");
        dump_ast(e->unary_op, indent);
        break;
    case EX_INTEGER:
        fprintf(stderr, "%d", e->value);
        break;
    }
}