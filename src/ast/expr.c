#include "peachcc.h"

static Expr *new_expr(ExprKind k, char *str);

Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, char *str)
{
    Expr *e = new_expr(op, str);
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

Expr *new_unop(ExprKind op, Expr *child_expr, char *str)
{
    Expr *e = new_expr(op, str);
    e->unary_op = child_expr;
    return e;
}

Expr *new_integer_literal(int value, char *str)
{
    Expr *e = new_expr(EX_INTEGER, str);
    e->value = value;
    return e;
}

Expr *new_string_literal(char *contents, char *str)
{
    Expr *e = new_expr(EX_STRING, str);
    e->copied_str = (char *)calloc(strlen(contents), sizeof(char));
    strncpy(e->copied_str, contents, strlen(contents));
    e->copied_str[strlen(contents)] = 0;
    e->length = strlen(contents);
    e->id = str_id_g;

    // グローバル変数としても登録する
    GlobalVariable *glob_var = (GlobalVariable *)calloc(1, sizeof(GlobalVariable));

    // \0が挿入されるので，+1
    glob_var->cty = new_array(new_char(), strlen(contents) + 1);
    glob_var->init_data = e->copied_str;
    char *buf = calloc(20, sizeof(char));
    sprintf(buf, ".L.str%d", str_id_g++);
    map_put(global_variables_g, buf, glob_var);

    return e;
}
Expr *new_identifier(char *str, size_t length)
{
    Expr *e = new_expr(EX_LOCAL_VAR, str);
    e->copied_str = (char *)calloc(length, sizeof(char));
    strncpy(e->copied_str, str, length);
    e->copied_str[length] = 0;
    e->length = length;
    return e;
}
static Expr *new_expr(ExprKind k, char *str)
{
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = k;
    e->str = str;
    return e;
}
