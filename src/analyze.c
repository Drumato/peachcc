#include "peachcc.h"

// 呼び出し式のanalyzeで使用
// 関数の返り値の型を探す
static Vector *functions_g;

static void walk_fn(Function *f);

static void walk_stmt(Stmt *s);
static CType *walk_expr(Expr **e);

void analyze(Program *program)
{
    functions_g = program->functions;
    for (size_t i = 0; i < program->functions->len; i++)
    {
        Function *f = (Function *)program->functions->data[i];
        local_variables_in_cur_fn_g = f->local_variables;
        walk_fn(f);
    }
}
static void walk_fn(Function *f)
{
    // ローカル変数のスタック割当
    LocalVariable *lv;
    size_t total_stack_size = f->stack_size;
    for (size_t i = 0; i < local_variables_in_cur_fn_g->keys->len; i++)
    {
        lv = local_variables_in_cur_fn_g->vals->data[i];
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
    switch (s->kind)
    {
    case ST_EXPR:
        walk_expr(&s->expr);
        break;
    case ST_IF:
        walk_expr(&s->cond);
        walk_stmt(s->then);
        if (s->els)
        {
            walk_stmt(s->then);
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
        walk_stmt(s->then);
        break;
    case ST_WHILE:
        walk_expr(&s->cond);
        walk_stmt(s->then);
        break;
    case ST_RETURN:
        walk_expr(&s->expr);
        break;
    case ST_COMPOUND:
    {
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
    switch ((*e)->kind)
    {
    case EX_INTEGER:
        return new_ctype(TY_INT, 8);
    case EX_LOCAL_VAR:
    {
        LocalVariable *lv = map_get(local_variables_in_cur_fn_g, (*e)->str, (*e)->length);
        assert(lv);
        return lv->cty;
    }
    case EX_CALL:
    {
        for (size_t i = 0; i < (*e)->params->len; i++)
        {
            Expr *param = (*e)->params->data[i];
            walk_expr(&param);
        }

        // 関数の返り値型を得るため，関数列を走査
        for (size_t i = 0; i < functions_g->len; i++)
        {
            Function *f = functions_g->data[i];
            if (!strncmp(f->copied_name, (*e)->copied_name, (*e)->length))
            {
                return f->return_type;
            }
        }
        error_at((*e)->str, "cannot analyze %s's return type", (*e)->copied_name);
        return NULL;
    }
    case EX_UNARY_ADDR:
    {
        CType *base = walk_expr(&(*e)->unary_op);
        CType *ptr = new_ctype(TY_PTR, 8);
        ptr->ptr_to = base;
        return ptr;
    }
    case EX_UNARY_PLUS:
        return new_ctype(TY_INT, 8);
    case EX_UNARY_MINUS:
        return new_ctype(TY_INT, 8);
    case EX_UNARY_DEREF:
    {
        CType *ptr = walk_expr(&(*e)->unary_op);
        if (ptr->kind != TY_PTR)
        {
            error_at((*e)->str, "cannot dereference without pointer");
        }
        return ptr->ptr_to;
    }
    case EX_ADD:
    {
        CType *lhs_type = walk_expr(&(*e)->lhs);
        CType *rhs_type = walk_expr(&(*e)->rhs);
        if (lhs_type->kind == TY_INT && rhs_type->kind == TY_INT)
        {
            return lhs_type;
        }

        // pointer同士の足し算はinvalid
        if (lhs_type->ptr_to && rhs_type->ptr_to)
        {
            error_at((*e)->str, "invalid addition between pointer(s)");
        }

        // C言語において `+` はポインタについてオーバーロードされているので，その挙動を実現
        // ptr + integer の形に単純化する
        if (!lhs_type->ptr_to && rhs_type->ptr_to)
        {
            Expr *tmp = (*e)->lhs;
            (*e)->lhs = (*e)->rhs;
            (*e)->rhs = tmp;
        }

        // ポインタ演算
        // ptr + integerの形になっているので，右辺をtype_size倍する
        (*e)->rhs = new_binop(EX_MUL, (*e)->rhs, new_integer(lhs_type->ptr_to->size, (*e)->rhs->str), (*e)->str);
        return lhs_type;
    }
    case EX_SUB:
    {
        CType *lhs_type = walk_expr(&(*e)->lhs);
        CType *rhs_type = walk_expr(&(*e)->rhs);
        if (lhs_type->kind == TY_INT && rhs_type->kind == TY_INT)
        {
            return lhs_type;
        }

        // C言語において `-` はポインタについてオーバーロードされているので，その挙動を実現
        // ptr - integer の場合，+と同様に
        if (lhs_type->ptr_to && rhs_type->kind == TY_INT)
        {
            (*e)->rhs = new_binop(EX_MUL, (*e)->rhs, new_integer(lhs_type->ptr_to->size, (*e)->rhs->str), (*e)->str);
            return lhs_type;
        }

        // ptr - ptr の場合，2つのポインタの間にどれだけtype_size * byteがあるか，という意味論に
        if (lhs_type->ptr_to && rhs_type->ptr_to)
        {
            *e = new_binop(EX_DIV, *e, new_integer(lhs_type->ptr_to->size, (*e)->rhs->str), (*e)->str);
            return new_ctype(TY_INT, 8);
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
        CType *lhs_type = walk_expr(&(*e)->lhs);
        walk_expr(&(*e)->rhs);
        return lhs_type;
    }
    case EX_ASSIGN:
    {
        CType *lhs_type = walk_expr(&(*e)->lhs);
        CType *rhs_type = walk_expr(&(*e)->rhs);
        if (lhs_type->kind != rhs_type->kind)
        {
            error_at((*e)->str, "invalid assignments between difference types");
        }
        return lhs_type;
    }
    default:
        error_at((*e)->str, "cannot analyze from it");
        return NULL;
    }
}