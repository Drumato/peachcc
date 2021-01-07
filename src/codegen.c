#include "peachcc.h"

static FILE *output_file_g;
static int label_num_g;
static char *param_reg8_g[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *param_reg16_g[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *param_reg32_g[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *param_reg64_g[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static Function *cur_fn_g;

static void gen_fn(Function *fn);
static void gen_stmt(Stmt *stmt);

static void gen_function_prologue(int stack_size);
static void gen_function_epilogue(char *func_name);
static void gen_lvalue(Expr *expr);
static void gen_expr(Expr *expr);
static void gen_binop_expr(Expr *expr);
static void gen_unary_op_expr(Expr *expr);
static void gen_compare_rax_and_rdi_by(char *mode);
static void gen_load(CType *cty);
static void push_reg(char *reg);
static void pop_reg(char *reg);
static void store_parameters_to_stack(size_t reg_num, int stack_offset, size_t size);
static void store(CType *cty);

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
        store_parameters_to_stack(i, param->stack_offset, param->cty->size);
    }

    for (size_t i = 0; i < fn->stmts->len; i++)
    {
        Stmt *s = (Stmt *)fn->stmts->data[i];
        gen_stmt(s);
    }

    gen_function_epilogue(fn->copied_name);
}

static void gen_stmt(Stmt *stmt)
{
    fprintf(output_file_g, ".loc 1 %zu\n", stmt->line);
    switch (stmt->kind)
    {
    case ST_LABEL:
        fprintf(output_file_g, "  # ST_LABEL\n");
        fprintf(output_file_g, "  %s:\n", stmt->label);
        gen_stmt(stmt->then);
        break;
    case ST_GOTO:
        fprintf(output_file_g, "  # ST_GOTO\n");
        fprintf(output_file_g, "  jmp %s\n", stmt->label);
        break;
    case ST_EXPR:
        fprintf(output_file_g, "  # ST_EXPR\n");
        gen_expr(stmt->expr);
        break;
    case ST_RETURN:
        fprintf(output_file_g, "  # ST_RETURN\n");
        gen_expr(stmt->expr);
        fprintf(output_file_g, "  jmp .L.return.%s\n", cur_fn_g->copied_name);
        break;
    case ST_WHILE:
    {
        fprintf(output_file_g, "  # ST_WHILE\n");
        int label = label_num_g++;
        fprintf(output_file_g, ".Lbegin%d:\n", label);
        gen_expr(stmt->cond);
        fprintf(output_file_g, "  cmp rax, 0\n");
        fprintf(output_file_g, "  je .Lend%d\n", label);

        gen_stmt(stmt->then);
        fprintf(output_file_g, "  jmp .Lbegin%d\n", label);

        fprintf(output_file_g, ".Lend%d:\n", label);
        break;
    }
    case ST_FOR:
    {
        fprintf(output_file_g, "  # ST_FOR\n");
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
        fprintf(output_file_g, "  # ST_IF\n");
        int label = label_num_g++;

        // 条件式をコンパイルし，trueかどうかチェック
        // falseならばelseブロックに飛ぶ
        gen_expr(stmt->cond);
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
        fprintf(output_file_g, "  # ST_COMPOUND\n");
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

// 式のコンパイル
// 計算結果はraxに格納されるので注意
static void gen_expr(Expr *expr)
{
    // fprintf(output_file_g, ".loc 1 %zu\n", expr->line);

    switch (expr->kind)
    {
    case EX_INTEGER:
        fprintf(output_file_g, "  # EX_INTEGER\n");
        fprintf(output_file_g, "  mov rax, %d\n", expr->value);
        break;
    case EX_STRING:
        fprintf(output_file_g, "  # EX_STRING\n");
        fprintf(output_file_g, "  lea rax, .str%d[rip]\n", expr->id);
        break;
    case EX_LOCAL_VAR:
        fprintf(output_file_g, "  # EX_LOCAL_VAR\n");
        // gen_lvalue でアドレスをプッシュするだけでは変数式にならない
        gen_lvalue(expr);
        gen_load(expr->cty);
        break;
    case EX_MEMBER_ACCESS:
    {
        fprintf(output_file_g, "  # EX_MEMBER_ACCESS\n");
        gen_lvalue(expr);
        gen_load(expr->cty);
        break;
    }
    case EX_CALL:
    {
        fprintf(output_file_g, "  # EX_CALL\n");
        int nparams = 0;
        for (int i = 0; i < expr->params->len; i++)
        {
            Expr *arg = expr->params->data[i];
            gen_expr(arg);
            // gen_exprはraxに値をロードするだけなので，pushも実行する
            push_reg("rax");
            nparams++;
        }

        for (int i = nparams - 1; i >= 0; i--)
        {
            pop_reg(param_reg64_g[i]);
        }

        // fprintf(output_file_g, "  push rbp\n");
        // fprintf(output_file_g, "  mov rbp, rsp\n");
        // fprintf(output_file_g, "  and rsp, -16\n");
        fprintf(output_file_g, "  mov rax, 0\n");
        fprintf(output_file_g, "  call %s\n", expr->copied_str);

        // fprintf(output_file_g, "  mov rsp, rbp\n");
        // fprintf(output_file_g, "  pop rbp\n");
        break;
    }
    case EX_UNARY_ADDR:
        fprintf(output_file_g, "  # EX_UNARY_ADDR\n");
        // このノードだけgen_unary_op_expr()に突っ込めないので注意
        // 単にアドレスを生成して返すだけなので．
        gen_lvalue(expr->unary_op);
        break;
    case EX_UNARY_PLUS:
    case EX_UNARY_MINUS:
    case EX_UNARY_DEREF:
    case EX_UNARY_NOT:
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
        fprintf(output_file_g, "  # EX_ASSIGN\n");
        // 格納するアドレスをスタックに保持しておき，
        // 代入する式の値 -> rax; store先 -> rdiの状態を作る．
        // これで，代入式全体の結果がraxに格納されていることと同じになる
        gen_lvalue(expr->lhs);
        push_reg("rax");
        gen_expr(expr->rhs);
        store(expr->cty);
        break;
    case EX_CONDITION:
    {
        fprintf(output_file_g, "  # EX_CONDITION\n");
        // if文のようにcmpをかけて，どちらかのブロックがスタックに値をpushしてくれれば三項演算子になる
        int label = label_num_g++;

        // 条件式をコンパイルし，trueかどうかチェック
        // falseならばelseブロックに飛ぶ
        gen_expr(expr->cond);
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

// 左辺値としてのコンパイル
// 結果のアドレスはraxに格納される
static void gen_lvalue(Expr *expr)
{
    switch (expr->kind)
    {
    case EX_LOCAL_VAR:
    {
        if (expr->var->is_global)
        {
            fprintf(output_file_g, "  lea rax, %s[rip]\n", expr->copied_str);
        }
        else
        {
            fprintf(output_file_g, "  lea rax, %d[rbp]\n", expr->var->stack_offset);
        }
        return;
    }
    case EX_UNARY_DEREF:
    {
        // 変数のアドレスをraxにロードするだけでなく
        // そのアドレスに格納されたアドレスを得る操作をして良いので，gen_exprを呼ぶ
        gen_expr(expr->unary_op);
        return;
    }
    case EX_MEMBER_ACCESS:
    {
        gen_lvalue(expr->unary_op);
        fprintf(output_file_g, "  add rax, %d\n", expr->member->offset);
        return;
    }
    default:
        error_at(expr->str, expr->line, "not allowed this expr as a lvalue");
    }
}
static void gen_binop_expr(Expr *expr)
{
    assert(expr->lhs);
    assert(expr->rhs);

    // rhsのコンパイル結果(rax)をスタックに保持しておくことで，
    // lhs -> rax; rhs->rdiを実現
    gen_expr(expr->rhs);
    push_reg("rax");
    gen_expr(expr->lhs);
    pop_reg("rdi");

    switch (expr->kind)
    {
    case EX_ADD:
        fprintf(output_file_g, "  # EX_ADD\n");
        fprintf(output_file_g, "  add rax, rdi\n");
        break;
    case EX_SUB:
        fprintf(output_file_g, "  # EX_SUB\n");
        fprintf(output_file_g, "  sub rax, rdi\n");
        break;
    case EX_MUL:
        fprintf(output_file_g, "  # EX_MUL\n");
        fprintf(output_file_g, "  imul rax, rdi\n");
        break;
    case EX_DIV:
        fprintf(output_file_g, "  # EX_DIV\n");
        fprintf(output_file_g, "  cqo\n");
        fprintf(output_file_g, "  idiv rdi\n");
        break;
    case EX_MOD:
        fprintf(output_file_g, "  # EX_MOD\n");
        fprintf(output_file_g, "  cqo\n");
        fprintf(output_file_g, "  idiv rdi\n");
        fprintf(output_file_g, "  mov rax, rdx\n");
        break;
    case EX_EQ:
        fprintf(output_file_g, "  # EX_EQ\n");
        gen_compare_rax_and_rdi_by("sete");
        break;
    case EX_NTEQ:
        fprintf(output_file_g, "  # EX_NTEQ\n");
        gen_compare_rax_and_rdi_by("setne");
        break;
    case EX_GE:
        fprintf(output_file_g, "  # EX_GE\n");
        gen_compare_rax_and_rdi_by("setg");
        break;
    case EX_GEEQ:
        fprintf(output_file_g, "  # EX_GEEQ\n");
        gen_compare_rax_and_rdi_by("setge");
        break;
    case EX_LE:
        fprintf(output_file_g, "  # EX_LE\n");
        gen_compare_rax_and_rdi_by("setl");
        break;
    case EX_LEEQ:
        fprintf(output_file_g, "  # EX_LEEQ\n");
        gen_compare_rax_and_rdi_by("setle");
        break;
    case EX_LOGOR:
        fprintf(output_file_g, "  # EX_LOGOR\n");
        fprintf(output_file_g, "  or rax, rdi\n");
        break;
    case EX_LOGAND:
        fprintf(output_file_g, "  # EX_LOGAND\n");
        fprintf(output_file_g, "  and rax, rdi\n");
        break;
    default:
        error_at(expr->str, expr->line, "It's not a binary operation");
        break;
    }
}

static void gen_unary_op_expr(Expr *expr)
{
    assert(expr->unary_op);

    gen_expr(expr->unary_op);
    switch (expr->kind)
    {

    case EX_UNARY_PLUS: // 単項+は何もしなくてよい
        fprintf(output_file_g, "  # EX_UNARY_PLUS\n");
        break;
    case EX_UNARY_MINUS:
        fprintf(output_file_g, "  # EX_UNARY_MINUS\n");
        fprintf(output_file_g, "  neg rax\n");
        break;
    case EX_UNARY_DEREF:
        fprintf(output_file_g, "  # EX_UNARY_DEREF\n");
        // 配列オブジェクトかもしれないので，単にデリファレンスせずgen_loadでチェックを挟む
        gen_load(expr->cty);
        break;
    case EX_UNARY_NOT:
        fprintf(output_file_g, "  # EX_UNARY_NOT\n");
        fprintf(output_file_g, "  cmp rax, 0\n");
        fprintf(output_file_g, "  sete al\n");
        fprintf(output_file_g, "  movzx rax, al\n");
        break;
    default:
        error_at(expr->str, expr->line, "It's not a unary operation");
        break;
    }
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
static void gen_function_epilogue(char *func_name)
{
    fprintf(output_file_g, ".L.return.%s:\n", func_name);
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

    if (cty->size == 1)
    {
        fprintf(output_file_g, "  movsx rax, BYTE PTR [rax]\n");
    }
    else if (cty->size == 2)
    {
        fprintf(output_file_g, "  movswq rax, WORD PTR [rax]\n");
    }
    else if (cty->size == 4)
    {
        fprintf(output_file_g, "  movsxd rax, DWORD PTR [rax]\n");
    }
    else
    {
        fprintf(output_file_g, "  mov rax, [rax]\n");
    }
}

static void push_reg(char *reg)
{
    fprintf(output_file_g, "  push %s\n", reg);
}
static void pop_reg(char *reg)
{
    fprintf(output_file_g, "  pop %s\n", reg);
}

static void store_parameters_to_stack(size_t reg_num, int stack_offset, size_t size)
{
    char *reg;
    switch (size)
    {
    case 1:
        reg = param_reg8_g[reg_num];
        break;
    case 2:
        reg = param_reg16_g[reg_num];

        break;
    case 4:
        reg = param_reg32_g[reg_num];
        break;
    case 8:
        reg = param_reg64_g[reg_num];
        break;
    default:
        fprintf(stderr, "not allowed such a type-size in store-param semantics");
        exit(1);
        break;
    }

    fprintf(output_file_g, "  mov %d[rbp], %s\n", stack_offset, reg);
}

static void store(CType *cty)
{
    pop_reg("rdi");
    char *src_reg;

    switch (cty->size)
    {
    case 1:
        src_reg = "al";
        break;
    case 2:
        src_reg = "ax";
        break;
    case 4:
        src_reg = "eax";
        break;
    case 8:
        src_reg = "rax";
        break;
    default:
        fprintf(stderr, "not allowed such a type-size in store semantics");
        exit(1);
        break;
    }

    fprintf(output_file_g, "  mov [rdi], %s\n", src_reg);
}