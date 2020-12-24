#include "peachcc.h"

static void dump_fn(Function *f);
static void dump_stmt(Stmt *s, int indent);
static void dump_expr(Expr *e);

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
void dump_ctype(CType *cty)
{
    switch (cty->kind)
    {
    case TY_INT:
        fprintf(stderr, "int");
        break;
    case TY_PTR:
        dump_ctype(cty->base);
        fprintf(stderr, "*");
        break;
    case TY_ARRAY:
        dump_ctype(cty->base);
        fprintf(stderr, "[%d]", cty->array_len);
        break;
    }
}

void dump_ast(TranslationUnit *translation_unit)
{
    assert(translation_unit);
    assert(translation_unit->functions);
    assert(translation_unit->global_variables);

    for (size_t i = 0; i < translation_unit->global_variables->keys->len; i++)
    {
        char *glob_var_name = translation_unit->global_variables->keys->data[i];
        CType *glob_var_ty = translation_unit->global_variables->vals->data[i];

        fprintf(stderr, "globals[%zu] = %s: ", i, glob_var_name);
        dump_ctype(glob_var_ty);

        fprintf(stderr, "\n");
    }

    for (size_t i = 0; i < translation_unit->functions->len; i++)
    {
        Function *f = (Function *)(translation_unit->functions->data[i]);
        dump_fn(f);
    }
}

static void dump_fn(Function *f)
{
    fprintf(stderr, "%s() {\n", f->copied_name);
    for (size_t i = 0; i < f->stmts->len; i++)
    {
        Stmt *s = (Stmt *)f->stmts->data[i];
        dump_stmt(s, 4);
    }
    fprintf(stderr, ")\n");
}

static void dump_stmt(Stmt *s, int indent)
{
    assert(s);
    switch (s->kind)
    {
    case ST_WHILE:
        fprintf(stderr, "%*sWhileStmt(expr: ", indent, " ");
        dump_expr(s->cond);
        fprintf(stderr, ")\n");

        dump_stmt(s->then, indent + 4);

        break;
    case ST_IF:
        fprintf(stderr, "%*sIfStmt(expr: ", indent, " ");
        dump_expr(s->cond);
        fprintf(stderr, ")\n");

        dump_stmt(s->then, indent + 4);
        if (s->els != NULL)
        {
            fprintf(stderr, "%*sElseBlock:\n", indent, " ");
            dump_stmt(s->els, indent + 4);
        }

        break;
    case ST_FOR:
        fprintf(stderr, "%*sForStmt(", indent, " ");
        if (s->init != NULL)
        {
            fprintf(stderr, "init: ");
            dump_expr(s->init);
            fprintf(stderr, "; ");
        }
        if (s->cond != NULL)
        {
            fprintf(stderr, "cond: ");
            dump_expr(s->cond);
            fprintf(stderr, "; ");
        }
        if (s->inc != NULL)
        {
            fprintf(stderr, "inc: ");
            dump_expr(s->inc);
        }
        fprintf(stderr, ")\n");

        dump_stmt(s->then, indent + 4);

        break;
    case ST_RETURN:
        fprintf(stderr, "%*sReturnStmt(expr: ", indent, " ");
        dump_expr(s->expr);
        fprintf(stderr, ");\n");
        break;
    case ST_EXPR:
        fprintf(stderr, "%*sExprStmt(expr: ", indent, " ");
        dump_expr(s->expr);
        fprintf(stderr, ");\n");
        break;
    case ST_COMPOUND:
        fprintf(stderr, "%*sCompoundStmt(\n", indent, " ");
        for (size_t i = 0; i < s->body->len; i++)
        {
            Stmt *child = (Stmt *)s->body->data[i];
            dump_stmt(child, indent + 4);
        }
        fprintf(stderr, "%*s);\n", indent, " ");
        break;
    }
}

// Exprを標準エラー出力にダンプする
static void dump_expr(Expr *e)
{
    assert(e);
    switch (e->kind)
    {
    case EX_CALL:
    {
        fprintf(stderr, "%s(", e->copied_name);
        for (size_t i = 0; i < e->params->len; i++)
        {
            Expr *param = (Expr *)e->params->data[i];
            dump_expr(param);
            if (i != e->params->len - 1)
            {
                fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, ")");
        break;
    }
    case EX_UNARY_SIZEOF:
        fprintf(stderr, "SizeofExpr(");
        dump_expr(e->unary_op);
        fprintf(stderr, ")");
        break;
    case EX_ASSIGN:
        fprintf(stderr, "AssignExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_ADD:
        fprintf(stderr, "AddExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_SUB:
        fprintf(stderr, "SubExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_MUL:
        fprintf(stderr, "MulExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_DIV:
        fprintf(stderr, "DivExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_LE:
        fprintf(stderr, "LessThanExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_GE:
        fprintf(stderr, "GreaterThanExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_LEEQ:
        fprintf(stderr, "LessThanEqualExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_GEEQ:
        fprintf(stderr, "GreaterThanEqualExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_EQ:
        fprintf(stderr, "EqualExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_NTEQ:
        fprintf(stderr, "NotEqualExpr(");
        dump_expr(e->lhs);
        fprintf(stderr, ", ");
        dump_expr(e->rhs);
        fprintf(stderr, ")");
        break;
    case EX_UNARY_PLUS:
        fprintf(stderr, "UnaryPlusExpr(");

        dump_expr(e->unary_op);
        fprintf(stderr, ")");
        break;
    case EX_UNARY_MINUS:
        fprintf(stderr, "UnaryMinusExpr(");

        dump_expr(e->unary_op);
        fprintf(stderr, ")");
        break;
    case EX_UNARY_ADDR:
        fprintf(stderr, "UnaryAddrExpr(");
        dump_expr(e->unary_op);
        fprintf(stderr, ")");
        break;
    case EX_UNARY_DEREF:
        fprintf(stderr, "UnaryDerefExpr(");
        dump_expr(e->unary_op);
        fprintf(stderr, ")");
        break;
    case EX_INTEGER:
        fprintf(stderr, "%d", e->value);
        break;
    case EX_LOCAL_VAR:
    {
        fprintf(stderr, "%s", e->copied_name);
        break;
    }
    }
}