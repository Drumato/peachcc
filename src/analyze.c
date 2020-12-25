#include "peachcc.h"

// 呼び出し式のanalyzeで使用
// 関数の返り値の型を探す
static Vector *functions_g;

static void walk_fn(Function *f);

static void walk_stmt(Stmt *s);
static CType *walk_expr(Expr **e);

void analyze(TranslationUnit *translation_unit)
{
    global_variables_g = translation_unit->global_variables;
    functions_g = translation_unit->functions;

    for (size_t i = 0; i < translation_unit->functions->len; i++)
    {
        Function *f = (Function *)translation_unit->functions->data[i];
        walk_fn(f);
    }
}
static void walk_fn(Function *f)
{
    assert(f);
    assert(f->local_variables);
    assert(f->params);
    assert(f->copied_name);
    assert(f->return_type);
    assert(f->stmts);

    local_variables_in_cur_fn_g = f->local_variables;

    // ローカル変数のスタック割当
    // パース後の状態では，すべての変数のオフセットが0の状態で格納されている
    LocalVariable *lv;
    size_t total_stack_size = f->stack_size;
    for (size_t i = 0; i < local_variables_in_cur_fn_g->keys->len; i++)
    {
        lv = local_variables_in_cur_fn_g->vals->data[i];
        assert(lv);
        assert(lv->cty);
        assert(lv->str);

        lv->stack_offset = total_stack_size;
        total_stack_size -= lv->cty->size;
    }

    for (size_t i = 0; i < f->stmts->len; i++)
    {
        Stmt *s = f->stmts->data[i];
        walk_stmt(s);
    }
}
static void walk_stmt(Stmt *s)
{
    assert(s);
    switch (s->kind)
    {
    case ST_EXPR:
        assert(s->expr);
        walk_expr(&s->expr);
        break;
    case ST_IF:
        assert(s->cond);
        assert(s->then);
        walk_expr(&s->cond);
        walk_stmt(s->then);
        if (s->els)
        {
            walk_stmt(s->els);
        }
        break;
    case ST_FOR:

        if (s->init)
        {
            walk_expr(&s->init);
        }
        if (s->cond)
        {
            walk_expr(&s->cond);
        }
        if (s->inc)
        {
            walk_expr(&s->inc);
        }
        assert(s->then);
        walk_stmt(s->then);
        break;
    case ST_WHILE:
        assert(s->cond);
        assert(s->then);
        walk_expr(&s->cond);
        walk_stmt(s->then);
        break;
    case ST_RETURN:
        assert(s->expr);
        walk_expr(&s->expr);
        break;
    case ST_COMPOUND:
    {
        assert(s->body);
        for (size_t i = 0; i < s->body->len; i++)
        {
            Stmt *child = (Stmt *)s->body->data[i];
            walk_stmt(child);
        }
    }
    }
}

static CType *walk_expr(Expr **e)
{
    assert(*e);

    switch ((*e)->kind)
    {
    case EX_INTEGER:
        (*e)->cty = new_int();
        return (*e)->cty;
    case EX_STRING:
    {
        // グローバルマップに登録されているものを使う
        char *buf = (char *)calloc(20, sizeof(char));
        sprintf(buf, ".L.str%d", (*e)->id);
        GlobalVariable *glob_var = map_get(global_variables_g, buf, strlen(buf));
        assert(glob_var);
        (*e)->cty = glob_var->cty;
        return glob_var->cty;
    }
    case EX_LOCAL_VAR:
    {
        LocalVariable *lv = map_get(local_variables_in_cur_fn_g, (*e)->str, (*e)->length);
        if (lv != NULL)
        {
            (*e)->cty = lv->cty;
            return lv->cty;
        }

        // グローバル変数のマップからも探す
        GlobalVariable *glob_var = map_get(global_variables_g, (*e)->str, (*e)->length);
        assert(glob_var);
        (*e)->cty = glob_var->cty;
        return glob_var->cty;
    }
    case EX_UNARY_SIZEOF:
    {
        CType *cty = walk_expr(&(*e)->unary_op);
        *e = new_integer_literal(cty->size, (*e)->str);
        (*e)->cty = new_int();
        return (*e)->cty;
    }
    case EX_CALL:
    {
        assert((*e)->params);
        assert((*e)->copied_str);
        for (size_t i = 0; i < (*e)->params->len; i++)
        {
            Expr *param = (*e)->params->data[i];
            CType *param_ty = walk_expr(&param);
            param->cty = param_ty;
        }

        // 関数の返り値型を得るため，関数列を走査
        for (size_t i = 0; i < functions_g->len; i++)
        {
            Function *f = functions_g->data[i];
            if (!strncmp(f->copied_name, (*e)->copied_str, (*e)->length))
            {
                (*e)->cty = f->return_type;
                return f->return_type;
            }
        }
        // error_at((*e)->str, "cannot analyze %s's return type", (*e)->copied_str);
        return NULL;
    }
    case EX_UNARY_ADDR:
    {
        assert((*e)->unary_op);
        CType *base = walk_expr(&(*e)->unary_op);
        CType *ptr = new_ptr(base);
        (*e)->cty = ptr;
        return ptr;
    }
    case EX_UNARY_PLUS:
        assert((*e)->unary_op);
        (*e)->cty = new_int();
        return (*e)->cty;
    case EX_UNARY_MINUS:
        assert((*e)->unary_op);
        (*e)->cty = new_int();
        return (*e)->cty;
    case EX_UNARY_DEREF:
    {
        assert((*e)->unary_op);
        CType *ptr = walk_expr(&(*e)->unary_op);
        (*e)->cty = ptr->base;
        return ptr->base;
    }
    case EX_ADD:
    {
        assert((*e)->lhs);
        assert((*e)->rhs);
        CType *lhs_type = walk_expr(&(*e)->lhs);
        CType *rhs_type = walk_expr(&(*e)->rhs);
        (*e)->cty = lhs_type;
        if (lhs_type->kind == TY_INT && rhs_type->kind == TY_INT)
        {
            return lhs_type;
        }

        // pointer同士の足し算はinvalid
        if (lhs_type->base && rhs_type->base)
        {
            error_at((*e)->str, "invalid addition between pointer(s)");
        }

        // C言語において `+` はポインタについてオーバーロードされているので，その挙動を実現
        // ptr + integer の形に単純化する
        if (!lhs_type->base && rhs_type->base)
        {
            Expr *tmp = (*e)->lhs;
            (*e)->lhs = (*e)->rhs;
            (*e)->rhs = tmp;
        }

        // ポインタ演算
        // ptr + integerの形になっているので，右辺をtype_size倍する
        (*e)->rhs = new_binop(EX_MUL, (*e)->rhs, new_integer_literal((*e)->lhs->cty->base->size, (*e)->rhs->str), (*e)->str);

        return (*e)->lhs->cty;
    }
    case EX_SUB:
    {
        assert((*e)->lhs);
        assert((*e)->rhs);
        CType *lhs_type = walk_expr(&(*e)->lhs);
        CType *rhs_type = walk_expr(&(*e)->rhs);
        (*e)->cty = lhs_type;
        if ((lhs_type->kind == TY_INT && rhs_type->kind == TY_INT) || (lhs_type->kind == TY_CHAR && rhs_type->kind == TY_CHAR))
        {
            return lhs_type;
        }

        // C言語において `-` はポインタについてオーバーロードされているので，その挙動を実現
        // ptr - integer の場合，+と同様に
        if (lhs_type->base && rhs_type->kind == TY_INT)
        {
            (*e)->rhs = new_binop(EX_MUL, (*e)->rhs, new_integer_literal(lhs_type->base->size, (*e)->rhs->str), (*e)->str);
            return lhs_type;
        }

        // ptr - ptr の場合，2つのポインタの間にどれだけtype_size * byteがあるか，という意味論に
        if (lhs_type->base && rhs_type->base)
        {
            *e = new_binop(EX_DIV, *e, new_integer_literal(lhs_type->base->size, (*e)->rhs->str), (*e)->str);
            return new_int();
        }

        error_at((*e)->str, "invalid subtraction between these types");
        return NULL;
    }
    case EX_MUL:
    case EX_DIV:
    case EX_LEEQ:
    case EX_LE:
    case EX_NTEQ:
    case EX_EQ:
    case EX_GE:
    case EX_GEEQ:
    {
        assert((*e)->lhs);
        assert((*e)->rhs);

        CType *lhs_type = walk_expr(&(*e)->lhs);
        walk_expr(&(*e)->rhs);
        (*e)->cty = lhs_type;
        return lhs_type;
    }
    case EX_ASSIGN:
    {
        assert((*e)->lhs);
        assert((*e)->rhs);

        CType *lhs_type = walk_expr(&(*e)->lhs);
        if (lhs_type->kind == TY_ARRAY)
        {
            error_at((*e)->str, "array type is not an lvalue");
        }
        walk_expr(&(*e)->rhs);
        (*e)->cty = lhs_type;
        return lhs_type;
    }
    default:
        error_at((*e)->str, "cannot analyze from it");
        return NULL;
    }
}