#include "peachcc.h"

static FILE *output_file_g;
static int label_num_g;
static char *param_reg8_g[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *param_reg64_g[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static Function *cur_fn_g;

static void gen_fn(Function *fn);
static void gen_stmt(Stmt *stmt);

static void gen_function_prologue(int stack_size);
static void gen_function_epilogue(void);
static void gen_lvalue(Expr *expr);
static void gen_expr(Expr *expr);
static void gen_binop_expr(Expr *expr);
static void gen_unary_op_expr(Expr *expr);
static void gen_compare_rax_and_rdi_by(char *mode);
static void out_newline(char *fmt, ...);
static void gen_load(CType *cty);
static void push_reg(char *reg);
static void pop_reg(char *reg);
static int align_to(int n, int align);

void codegen(FILE *output_file, TranslationUnit *translation_unit)
{
    label_num_g = 0;
    output_file_g = output_file;
    global_variables_g = translation_unit->global_variables;

    out_newline(".intel_syntax noprefix");

    for (size_t i = 0; i < global_variables_g->keys->len; i++)
    {
        char *glob_var_name = global_variables_g->keys->data[i];
        GlobalVariable *glob_var = global_variables_g->vals->data[i];

        out_newline("\n  .data");
        out_newline("  .globl %s", glob_var_name);
        out_newline("%s:", glob_var_name);
        if (glob_var->init_data)
        {
            for (size_t i = 0; i < glob_var->cty->array_len; i++)
            {
                out_newline("  .byte %d", glob_var->init_data[i]);
            }
            // out_newline("  .byte 0", glob_var->init_data[i]);
        }
        else
        {
            out_newline("  .zero %d", glob_var->cty->size);
        }
    }

    out_newline("\n  .text");
    for (size_t i = 0; i < translation_unit->functions->len; i++)
    {
        Function *f = (Function *)translation_unit->functions->data[i];
        gen_fn(f);
    }

    gen_function_epilogue();
}

static void gen_fn(Function *fn)
{
    cur_fn_g = fn;
    out_newline(".globl %s", fn->copied_name);
    out_newline("%s:", fn->copied_name);

    gen_function_prologue(fn->stack_size);

    // 引数がある分，スタックにstoreするコードを生成する
    for (size_t i = 0; i < fn->params->len; i++)
    {
        char *param_name = fn->params->data[i];
        LocalVariable *lv = (LocalVariable *)map_get(cur_fn_g->local_variables, param_name, strlen(param_name));
        if (lv->cty->size == 1)
        {
            out_newline("  mov -%zu[rbp], %s", lv->stack_offset, param_reg8_g[i]);
        }
        else
        {
            out_newline("  mov -%zu[rbp], %s", lv->stack_offset, param_reg64_g[i]);
        }
    }

    for (size_t i = 0; i < fn->stmts->len; i++)
    {
        Stmt *s = (Stmt *)fn->stmts->data[i];
        gen_stmt(s);
    }
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
    case EX_STRING:
        out_newline("  push offset .str%d", expr->id);
        break;
    case EX_LOCAL_VAR:
        // gen_lvalue でアドレスをプッシュするだけでは変数式にならない
        gen_lvalue(expr);
        gen_load(expr->cty);
        break;
    case EX_CALL:
    {
        int nparams = 0;
        for (int i = 0; i < expr->params->len; i++)
        {
            Expr *arg = expr->params->data[i];
            gen_expr(arg);
            nparams++;
        }

        for (int i = nparams - 1; i >= 0; i--)
        {
            pop_reg(param_reg64_g[i]);
        }

        out_newline("	push rbp");
        out_newline("	mov rbp, rsp");
        out_newline("	and rsp, -16");
        out_newline("  call %s", expr->copied_str);

        out_newline("	mov rsp, rbp");
        out_newline("	pop rbp");
        push_reg("rax");
        break;
    }
    case EX_UNARY_ADDR:
        // このノードだけgen_unary_op_expr()に突っ込めないので注意
        // 単にアドレスを生成して返すだけなので．
        gen_lvalue(expr->unary_op);
        break;
    case EX_UNARY_PLUS:
    case EX_UNARY_MINUS:
    case EX_UNARY_DEREF:
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
    switch (expr->kind)
    {
    case EX_LOCAL_VAR:
    {

        // 変数のスタックオフセットを計算
        LocalVariable *lv = map_get(cur_fn_g->local_variables, expr->copied_str, expr->length);
        if (lv != NULL)
        {
            out_newline("  lea rax, -%d[rbp]", lv->stack_offset);
            push_reg("rax");
            break;
        }

        // グローバル変数とする
        // analyze.cの解析により，変数が存在しなければ既にエラーが出ているはずなので，
        // この時点では存在すると決め打ってコード生成して良い
        out_newline("  push offset %s", expr->copied_str);
        break;
    }
    case EX_UNARY_DEREF:
    {
        // 変数のアドレスをスタックに積むだけでなく
        // そのアドレスに格納されたアドレスを得る操作をして良い
        gen_expr(expr->unary_op);
        break;
    }
    default:
        error_at(expr->str, "not allowed this expr as a lvalue");
    }
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
    switch (expr->kind)
    {

    case EX_UNARY_PLUS: // 単項+は何もしなくてよい
        pop_reg("rax");
        break;

    case EX_UNARY_MINUS:
        pop_reg("rax");
        out_newline("  neg rax");
        break;
    case EX_UNARY_DEREF:
        // 配列オブジェクトかもしれないので，単にデリファレンスせずgen_loadでチェックを挟む
        gen_load(expr->cty);
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
    int aligned_stack_size = align_to(stack_size, 16);
    push_reg("rbp");
    out_newline("  mov rbp, rsp");
    out_newline("  sub rsp, %d", aligned_stack_size);
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
    out_newline("  movzx rax, al");
}

// raxに格納されたアドレスの中身を取り出す
// 配列の場合，レジスタに配列全体をロードできないので何もしない
// これにより，レジスタには単に配列オブジェクトの先頭アドレスが格納される．
// C言語のセマンティクスに合致したコード生成が可能．
static void gen_load(CType *cty)
{
    if (cty->kind == TY_ARRAY)
    {
        return;
    }
    pop_reg("rax");
    if (cty->size == 1)
    {
        out_newline("  movsx rax, BYTE PTR [rax]");
    }
    else
    {
        out_newline("  mov rax, [rax]");
    }
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

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
static int align_to(int n, int align)
{
    return (n + align - 1) / align * align;
}