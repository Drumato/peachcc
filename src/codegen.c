#include "peachcc.h"

FILE *output_file_g;

static void gen_stmt(Stmt *stmt);

static void gen_function_prologue(int stack_size);
static void gen_function_epilogue(void);
static void gen_lvalue(Expr *expr);
static void gen_expr(Expr *expr);
static void gen_binop_expr(Expr *expr);
static void gen_unary_op_expr(Expr *expr);
static void gen_compare_rax_and_rdi_by(char *mode);
static void out_newline(char *fmt, ...);
static void gen_dereference_by_popping_stack(void);

void codegen(FILE *output_file, Program *program)
{
    output_file_g = output_file;

    out_newline(".intel_syntax noprefix");
    out_newline(".globl main");
    out_newline("main:");

    // 現在は決め打ちでスタックをアロケーション
    gen_function_prologue(208);

    for (size_t i = 0; i < program->stmts->len; i++)
    {
        Stmt *s = (Stmt *)program->stmts->data[i];
        gen_stmt(s);
    }
    gen_function_epilogue();
}

static void gen_stmt(Stmt *stmt)
{
    switch (stmt->kind)
    {
    case ST_EXPR:
        gen_expr(stmt->expr);
        out_newline("  pop rax");
        break;
    default:
        error_at(stmt->loc, "cannot codegen from it");
        break;
    }
}

static void gen_expr(Expr *expr)
{
    assert(expr);
    switch (expr->kind)
    {
    case EX_INTEGER:
        out_newline("  push %d", expr->value);
        break;
    case EX_LOCAL_VAR:
        // gen_lvalue でアドレスをプッシュするだけでは変数式にならない
        gen_lvalue(expr);
        gen_dereference_by_popping_stack();
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
    case EX_LEEQ:
    case EX_LE:
    case EX_NTEQ:
    case EX_EQ:
    case EX_GE:
    case EX_GEEQ:
        gen_binop_expr(expr);
        break;
    case EX_ASSIGN:
        // top < [変数に格納する値, 変数のアドレス] < bottom というようにコード生成
        gen_lvalue(expr->lhs);
        gen_expr(expr->rhs);

        out_newline("  pop rdi");
        out_newline("  pop rax");
        out_newline("  mov [rax], rdi");

        // 代入式なので，値を生成する必要がある
        out_newline("  push rdi");
        break;
    default:
        error_at(expr->str, "cannot codegen from it");
        break;
    }
}

static void gen_lvalue(Expr *expr)
{
    if (expr->kind != EX_LOCAL_VAR)
        error_at(expr->str, "It's not a variable in assignment");

    out_newline("  mov rax, rbp");

    // 変数のスタックオフセットを計算
    int offset = (expr->str[0] - 'a' + 1) * 8;
    out_newline("  sub rax, %d", offset);
    out_newline("  push rax");
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
    case EX_EQ:
        gen_compare_rax_and_rdi_by("sete");
        break;
    case EX_NTEQ:
        gen_compare_rax_and_rdi_by("setne");
        break;
    case EX_GE:
        gen_compare_rax_and_rdi_by("setg");
        break;
    case EX_GEEQ:
        gen_compare_rax_and_rdi_by("setge");
        break;
    case EX_LE:
        gen_compare_rax_and_rdi_by("setl");
        break;
    case EX_LEEQ:
        gen_compare_rax_and_rdi_by("setle");
        break;
    default:
        error_at(expr->str, "It's not a binary operation");
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
        error_at(expr->str, "It's not a unary operation");
        break;
    }
    out_newline("  push rax");
}

// 関数プロローグの生成
static void gen_function_prologue(int stack_size)
{
    out_newline("  push rbp");
    out_newline("  mov rbp, rsp");
    out_newline("  sub rsp, %d", stack_size);
}
// 関数エピローグの生成
static void gen_function_epilogue(void)
{
    out_newline("  mov rsp, rbp");
    out_newline("  pop rbp");
    out_newline("  ret");
}

// raxとrdiを比較し，渡されたモードでalにフラグを立てる．
// 最終的にraxに符号拡張して返す
static void gen_compare_rax_and_rdi_by(char *mode)
{
    out_newline("  cmp rax, rdi");
    out_newline("  %s al", mode);
    out_newline("  movzb rax, al");
}

// スタックのトップに積んであるアドレスを，
// そのアドレスが指す値に置き換える
static void gen_dereference_by_popping_stack(void)
{
    out_newline("  pop rax");
    out_newline("  mov rax, [rax]");
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