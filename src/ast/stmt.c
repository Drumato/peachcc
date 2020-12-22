#include "peachcc.h"

Stmt *new_stmt(StmtKind k, char *loc)
{
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = k;
    s->loc = loc;
    return s;
}