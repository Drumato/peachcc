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
static void gen_load(CType *cty);
static void push_reg(char *reg);
static void pop_reg(char *reg);
static int align_to(int n, int align);

void codegen(FILE *output_file, TranslationUnit *translation_unit)
{
    label_num_g = 0;
    output_file_g = output_file;

    fprintf(output_file_g, ".intel_syntax noprefix\n");
    fprintf(output_file_g, ".file 1 \"%s\"\n", peachcc_opt_g->input_file);

    for (size_t i = 0; i < translation_unit->global_variables->keys->len; i++)
    {
        char *glob_var_name = translation_unit->global_variables->keys->data[i];
        Variable *glob_var = translation_unit->global_variables->vals->data[i];

        fprintf(output_file_g, "\n  .data\n");
        if (!glob_var->is_static)
        {
            // translation-unit外部にも公開する
            fprintf(output_file_g, "  .globl %s\n", glob_var_name);
        }
        fprintf(output_file_g, "%s:\n", glob_var_name);
        if (glob_var->init_data)
        {
            for (size_t i = 0; i < glob_var->cty->array_len; i++)
            {
                fprintf(output_file_g, "  .byte %d\n", glob_var->init_data[i]);
            }
        }
        else
        {
            fprintf(output_file_g, "  .zero %zu\n", glob_var->cty->size);
        }
    }

    fprintf(output_file_g, "\n  .text\n");
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
    if (!fn->is_static)
    {
        // translation-unit外部にも公開する
        fprintf(output_file_g, ".globl %s\n", fn->copied_name);
    }
    fprintf(output_file_g, "%s:\n", fn->copied_name);

    gen_function_prologue(fn->stack_size);

    // 引数がある分，スタックにstoreするコードを生成する
    for (size_t i = 0; i < fn->params->len; i++)
    {
        Variable *param = fn->params->data[i];
        if (param->cty->size == 1)
        {
            fprintf(output_file_g, "  mov -%zu[rbp], %s\n", param->stack_offset, param_reg8_g[i]);
        }
        else
        {
            fprintf(output_file_g, "  mov -%zu[rbp], %s\n", param->stack_offset, param_reg64_g[i]);
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
    fprintf(output_file_g, ".loc 1 %zu\n", stmt->line);
    switch (stmt->kind)
    {
    case ST_EXPR:
        gen_expr(stmt->expr);
        pop_reg("rax");
        break;
    case ST_RETURN:
        gen_expr(stmt->expr);
        pop_reg("rax");
        fprintf(output_file_g, "  jmp .L.return\n");
        break;
    case ST_WHILE:
    {
        int label = label_num_g++;
        fprintf(output_file_g, ".Lbegin%d:\n", label);
        gen_expr(stmt->cond);
        pop_reg("rax");
        fprintf(output_file_g, "  cmp rax, 0\n");
        fprintf(output_file_g, "  je .Lend%d\n", label);

        gen_stmt(stmt->then);
        fprintf(output_file_g, "  jmp .Lbegin%d\n", label);

        fprintf(output_file_g, ".Lend%d:\n", label);
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
        fprintf(output_file_g, ".Lbegin%d:\n", label);
        if (stmt->cond != NULL)
        {
            gen_expr(stmt->cond);
            pop_reg("rax");
            fprintf(output_file_g, "  cmp rax, 0\n");
            fprintf(output_file_g, "  je .Lend%d\n", label);
        }

        gen_stmt(stmt->then);
        if (stmt->inc != NULL)
        {
            gen_expr(stmt->inc);
        }
        fprintf(output_file_g, "  jmp .Lbegin%d\n", label);
        fprintf(output_file_g, ".Lend%d:\n", label);
        break;
    }
    case ST_IF:
    {
        int label = label_num_g++;

        // 条件式をコンパイルし，trueかどうかチェック
        // falseならばelseブロックに飛ぶ
        gen_expr(stmt->cond);
        pop_reg("rax");
        fprintf(output_file_g, "  cmp rax, 0\n");
        fprintf(output_file_g, "  je .Lelse%d\n", label);
        gen_stmt(stmt->then);

        fprintf(output_file_g, "  jmp .Lend%d\n", label);

        // elseの無いif文の場合，何もしないブロックが出来上がる
        fprintf(output_file_g, ".Lelse%d:\n", label);
        if (stmt->els != NULL)
        {
            gen_stmt(stmt->els);
        }

        fprintf(output_file_g, ".Lend%d:\n", label);
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
        error_at(stmt->loc, stmt->line, "cannot codegen from it");
        break;
    }
}

static void gen_expr(Expr *expr)
{
    fprintf(output_file_g, ".loc 1 %zu\n", expr->line);

    switch (expr->kind)
    {
    case EX_INTEGER:
        fprintf(output_file_g, "  push %d\n", expr->value);
        break;
    case EX_STRING:
        fprintf(output_file_g, "  push offset .str%d\n", expr->id);
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

        fprintf(output_file_g, "  push rbp\n");
        fprintf(output_file_g, "  mov rbp, rsp\n");
        fprintf(output_file_g, "  and rsp, -16\n");
        fprintf(output_file_g, "  mov rax, 0\n");
        fprintf(output_file_g, "  call %s\n", expr->copied_str);

        fprintf(output_file_g, "  mov rsp, rbp\n");
        fprintf(output_file_g, "  pop rbp\n");
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
    case EX_MOD:
    case EX_LEEQ:
    case EX_LE:
    case EX_NTEQ:
    case EX_EQ:
    case EX_GE:
    case EX_GEEQ:
    case EX_LOGOR:
    case EX_LOGAND:
        gen_binop_expr(expr);
        break;
    case EX_ASSIGN:
        // top < [変数に格納する値, 変数のアドレス] < bottom というようにコード生成
        gen_lvalue(expr->lhs);
        gen_expr(expr->rhs);

        pop_reg("rdi");
        pop_reg("rax");
        fprintf(output_file_g, "  mov [rax], rdi\n");

        // 代入式なので，値を生成する必要がある
        push_reg("rdi");
        break;
    case EX_CONDITION:
    {
        // if文のようにcmpをかけて，どちらかのブロックがスタックに値をpushしてくれれば三項演算子になる
        int label = label_num_g++;

        // 条件式をコンパイルし，trueかどうかチェック
        // falseならばelseブロックに飛ぶ
        gen_expr(expr->cond);
        pop_reg("rax");
        fprintf(output_file_g, "  cmp rax, 0\n");
        fprintf(output_file_g, "  je .Lelse%d\n", label);
        gen_expr(expr->lhs);
        fprintf(output_file_g, "  jmp .Lend%d\n", label);
        fprintf(output_file_g, ".Lelse%d:\n", label);
        gen_expr(expr->rhs);

        fprintf(output_file_g, ".Lend%d:\n", label);
        break;
    }
    default:
        error_at(expr->str, expr->line, "cannot codegen from it");
        break;
    }
}

static void gen_lvalue(Expr *expr)
{
    switch (expr->kind)
    {
    case EX_LOCAL_VAR:
    {
        if (expr->var->is_global)
        {
            fprintf(output_file_g, "  push offset %s\n", expr->copied_str);
            break;
        }

        // スタックオフセットを積む
        fprintf(output_file_g, "  lea rax, -%zu[rbp]\n", expr->var->stack_offset);
        push_reg("rax");
        break;
    }
    case EX_UNARY_DEREF:
    {
        // 変数のアドレスをスタックに積むだけでなく
        // そのアドレスに格納されたアドレスを得る操作をして良いので，gen_exprを呼ぶ
        gen_expr(expr->unary_op);
        break;
    }
    default:
        error_at(expr->str, expr->line, "not allowed this expr as a lvalue");
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
        fprintf(output_file_g, "  add rax, rdi\n");
        break;
    case EX_SUB:
        fprintf(output_file_g, "  sub rax, rdi\n");
        break;
    case EX_MUL:
        fprintf(output_file_g, "  imul rax, rdi\n");
        break;
    case EX_DIV:
        fprintf(output_file_g, "  cqo\n");
        fprintf(output_file_g, "  idiv rdi\n");
        break;
    case EX_MOD:
        fprintf(output_file_g, "  cqo\n");
        fprintf(output_file_g, "  idiv rdi\n");
        fprintf(output_file_g, "  mov rax, rdx\n");
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
    case EX_LOGOR:
        fprintf(output_file_g, "  or rax, rdi\n");
        break;
    case EX_LOGAND:
        fprintf(output_file_g, "  and rax, rdi\n");
        break;
    default:
        error_at(expr->str, expr->line, "It's not a binary operation");
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
        fprintf(output_file_g, "  neg rax\n");
        break;
    case EX_UNARY_DEREF:
        // 配列オブジェクトかもしれないので，単にデリファレンスせずgen_loadでチェックを挟む
        gen_load(expr->cty);
        break;
    default:
        error_at(expr->str, expr->line, "It's not a unary operation");
        break;
    }
    push_reg("rax");
}

// 関数プロローグの生成
static void gen_function_prologue(int stack_size)
{
    int aligned_stack_size = align_to(stack_size, 16);
    push_reg("rbp");
    fprintf(output_file_g, "  mov rbp, rsp\n");
    fprintf(output_file_g, "  sub rsp, %d\n", aligned_stack_size);
}
// 関数エピローグの生成
static void gen_function_epilogue(void)
{
    fprintf(output_file_g, ".L.return:\n");
    fprintf(output_file_g, "  mov rsp, rbp\n");
    pop_reg("rbp");
    fprintf(output_file_g, "  ret\n");
}

// raxとrdiを比較し，渡されたモードでalにフラグを立てる．
// 最終的にraxに符号拡張して返す
static void gen_compare_rax_and_rdi_by(char *mode)
{
    fprintf(output_file_g, "  cmp rax, rdi\n");
    fprintf(output_file_g, "  %s al\n", mode);
    fprintf(output_file_g, "  movzx rax, al\n");
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
        fprintf(output_file_g, "  movsx rax, BYTE PTR [rax]\n");
    }
    else
    {
        fprintf(output_file_g, "  mov rax, [rax]\n");
    }
    push_reg("rax");
}

static void push_reg(char *reg)
{
    fprintf(output_file_g, "  push %s\n", reg);
}
static void pop_reg(char *reg)
{
    fprintf(output_file_g, "  pop %s\n", reg);
}

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
static int align_to(int n, int align)
{
    return (n + align - 1) / align * align;
}