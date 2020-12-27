#include "peachcc.h"

static Expr *new_expr(ExprKind k, char *str, size_t line_num);

Expr *new_conditional_expr(Expr *cond, Expr *lhs, Expr *rhs, char *str, size_t line_num)
{
    Expr *e = new_expr(EX_CONDITION, str, line_num);
    e->cond = cond;
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, char *str, size_t line_num)
{
    Expr *e = new_expr(op, str, line_num);
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

Expr *new_unop(ExprKind op, Expr *child_expr, char *str, size_t line_num)
{
    Expr *e = new_expr(op, str, line_num);
    e->unary_op = child_expr;
    return e;
}

Expr *new_integer_literal(int value, char *str, size_t line_num)
{
    Expr *e = new_expr(EX_INTEGER, str, line_num);
    e->value = value;
    return e;
}

Expr *new_string_literal(char *contents, char *str, size_t line_num)
{
    Expr *e = new_expr(EX_STRING, str, line_num);
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
    sprintf(buf, ".str%d", str_id_g++);
    map_put(global_variables_g, buf, glob_var);

    return e;
}
Expr *new_identifier(char *str, size_t length, size_t line_num)
{
    Expr *e = new_expr(EX_LOCAL_VAR, str, line_num);
    e->copied_str = (char *)calloc(length, sizeof(char));
    strncpy(e->copied_str, str, length);
    e->copied_str[length] = 0;
    e->length = length;
    return e;
}
static Expr *new_expr(ExprKind k, char *str, size_t line_num)
{
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = k;
    e->str = str;
    e->line = line_num;
    return e;
}

Function *new_function(char *name, size_t length)
{
    Function *f = (Function *)calloc(1, sizeof(Function));
    f->copied_name = calloc(length, sizeof(char));
    strncpy(f->copied_name, name, length);
    f->copied_name[length] = 0;
    return f;
}

TranslationUnit *new_translation_unit(void)
{
    TranslationUnit *translation_unit = (TranslationUnit *)calloc(1, sizeof(TranslationUnit));
    translation_unit->functions = new_vec();
    translation_unit->global_variables = new_map();
    return translation_unit;
}

Stmt *new_stmt(StmtKind k, char *loc, size_t line_num)
{
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = k;
    s->line = line_num;
    s->loc = loc;
    return s;
}