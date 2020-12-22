#include "peachcc.h"

static Stmt *new_stmt(StmtKind k, char *loc);

Stmt *new_exprstmt(Expr *expr, char *loc)
{
    Stmt *s = new_stmt(ST_EXPR, loc);
    s->expr = expr;
    return s;
}
Stmt *new_returnstmt(Expr *expr, char *loc)
{
    Stmt *s = new_stmt(ST_RETURN, loc);
    s->expr = expr;
    return s;
}

static Stmt *new_stmt(StmtKind k, char *loc)
{
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = k;
    s->loc = loc;
    return s;
}