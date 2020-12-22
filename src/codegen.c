#include "peachcc.h"

static FILE *output_file_g;
static int label_num_g;
static char *arg_reg64_g[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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
static void push_reg(char *reg);
static void pop_reg(char *reg);

void codegen(FILE *output_file, Program *program)
{
    label_num_g = 0;
    output_file_g = output_file;

    out_newline(".intel_syntax noprefix");
    out_newline(".globl main");
    out_newline("main:");

    // 現在は決め打ちでスタックをアロケーション
    gen_function_prologue(total_stack_size_in_fn_g);

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
        pop_reg("rax");
        break;
    case ST_RETURN:
        gen_expr(stmt->expr);
        pop_reg("rax");
        out_newline("  jmp .L.return");
        break;
    case ST_WHILE:
    {
        int label = label_num_g++;
        out_newline(".Lbegin%d:", label);
        gen_expr(stmt->cond);
        pop_reg("rax");
        out_newline("  cmp rax, 0");
        out_newline("  je .Lend%d", label);

        gen_stmt(stmt->then);
        out_newline("  jmp .Lbegin%d", label);

        out_newline(".Lend%d:", label);
        break;
    }
    case ST_FOR:
    {
        int label = label_num_g++;
        if (stmt->init != NULL)
        {
            gen_expr(stmt->init);
        }

        // 条件式をコンパイルしてtrueかチェック
        // そうであればforループを抜ける
        out_newline(".Lbegin%d:", label);
        if (stmt->cond != NULL)
        {
            gen_expr(stmt->cond);
            pop_reg("rax");
            out_newline("  cmp rax, 0");
            out_newline("  je .Lend%d", label);
        }

        gen_stmt(stmt->then);
        if (stmt->inc != NULL)
        {
            gen_expr(stmt->inc);
        }
        out_newline("  jmp .Lbegin%d", label);
        out_newline(".Lend%d:", label);
        break;
    }
    case ST_IF:
    {
        int label = label_num_g++;

        // 条件式をコンパイルし，trueかどうかチェック
        // falseならばelseブロックに飛ぶ
        gen_expr(stmt->cond);
        pop_reg("rax");
        out_newline("  cmp rax, 0");
        out_newline("  je .Lelse%d", label);
        gen_stmt(stmt->then);

        out_newline("  jmp .Lend%d", label);

        // elseの無いif文の場合，何もしないブロックが出来上がる
        out_newline(".Lelse%d:", label);
        if (stmt->els != NULL)
        {
            gen_stmt(stmt->els);
        }

        out_newline(".Lend%d:", label);
        break;
    }

    case ST_COMPOUND:
        for (size_t i = 0; i < stmt->body->len; i++)
        {
            Stmt *child = stmt->body->data[i];
            gen_stmt(child);
        }
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
    case EX_CALL:
    {
        int nargs = 0;
        for (int i = 0; i < expr->args->len; i++)
        {
            Expr *arg = expr->args->data[i];
            gen_expr(arg);
            nargs++;
        }

        for (int i = nargs - 1; i >= 0; i--)
        {
            pop_reg(arg_reg64_g[i]);
        }

        out_newline("  call %s", expr->copied_name);
        push_reg("rax");
        break;
    }
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

        pop_reg("rdi");
        pop_reg("rax");
        out_newline("  mov [rax], rdi");

        // 代入式なので，値を生成する必要がある
        push_reg("rdi");
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
    LocalVariable *lv = map_get(local_variables_g, expr->copied_name, expr->length);
    out_newline("  sub rax, %d", lv->stack_offset);
    push_reg("rax");
}

static void gen_binop_expr(Expr *expr)
{
    assert(expr->lhs);
    assert(expr->rhs);

    gen_expr(expr->lhs);
    gen_expr(expr->rhs);

    pop_reg("rdi");
    pop_reg("rax");

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

    push_reg("rax");
}

static void gen_unary_op_expr(Expr *expr)
{
    assert(expr->unary_op);

    gen_expr(expr->unary_op);
    pop_reg("rax");
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
    push_reg("rax");
}

// 関数プロローグの生成
static void gen_function_prologue(int stack_size)
{
    push_reg("rbp");
    out_newline("  mov rbp, rsp");
    out_newline("  sub rsp, %d", stack_size);
}
// 関数エピローグの生成
static void gen_function_epilogue(void)
{
    out_newline(".L.return:");
    out_newline("  mov rsp, rbp");
    pop_reg("rbp");
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
    pop_reg("rax");
    out_newline("  mov rax, [rax]");
    push_reg("rax");
}
// output_file_gに文字列を改行付きで書き込む
static void out_newline(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vfprintf(output_file_g, fmt, ap);
    fprintf(output_file_g, "\n");
}

static void push_reg(char *reg)
{
    out_newline("  push %s", reg);
}
static void pop_reg(char *reg)
{
    out_newline("  pop %s", reg);
}