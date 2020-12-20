#include "codegen.h"
#include "debug.h"
#include <assert.h>

#include <stdio.h>

FILE *output_file_g;

static void gen_expr(Expr *expr);
static void gen_binop_expr(Expr *expr);
static void gen_unary_op_expr(Expr *expr);
static void out_newline(char *fmt, ...);

void codegen(FILE *output_file, Expr *expr)
{
    output_file_g = output_file;

    out_newline(".intel_syntax noprefix");
    out_newline(".globl main");
    out_newline("main:");

    gen_expr(expr);

    out_newline("  pop rax");
    out_newline("  ret");
}

static void gen_expr(Expr *expr)
{
    assert(expr);
    assert(expr->tok);
    switch (expr->kind)
    {
    case EX_INTEGER:
        out_newline("  push %d", expr->value);
        break;
    case EX_UNARY_PLUS:
        gen_unary_op_expr(expr);
        break;
    case EX_UNARY_MINUS:
        gen_unary_op_expr(expr);
        break;
    case EX_ADD:
    case EX_SUB:
    case EX_MUL:
    case EX_DIV:
        gen_binop_expr(expr);
        break;
    default:
        error_at(expr->tok->str, "cannot codegen from it");
        break;
    }
}

static void gen_binop_expr(Expr *expr)
{
    assert(expr->lhs);
    assert(expr->rhs);

    gen_expr(expr->lhs);
    gen_expr(expr->rhs);

    out_newline("  pop rdi");
    out_newline("  pop rax");

    switch (expr->kind)
    {
    case EX_ADD:
        out_newline("  add rax, rdi");
        break;
    case EX_SUB:
        out_newline("  sub rax, rdi");
        break;
    case EX_MUL:
        out_newline("  imul rax, rdi");
        break;
    case EX_DIV:
        out_newline("  cqo");
        out_newline("  idiv rdi");
        break;
    default:
        error_at(expr->tok->str, "It's not a binary operation");
        break;
    }

    out_newline("  push rax");
}

static void gen_unary_op_expr(Expr *expr)
{
    assert(expr->unary_op);

    gen_expr(expr->unary_op);
    out_newline("  pop rax");
    switch (expr->kind)
    {

    case EX_UNARY_PLUS: // 単項+は何もしなくてよい
        break;

    case EX_UNARY_MINUS:
        out_newline("  neg rax");
        break;
    default:
        error_at(expr->tok->str, "It's not a unary operation");
        break;
    }
    out_newline("  push rax");
}
// output_file_gに文字列を改行付きで書き込む
static void out_newline(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vfprintf(output_file_g, fmt, ap);
    fprintf(output_file_g, "\n");
}