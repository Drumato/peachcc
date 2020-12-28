#include "peachcc.h"

Variable *new_variable(char *str, size_t length, CType *cty, bool is_global)
{
    Variable *v = (Variable *)calloc(1, sizeof(Variable));
    v->str = str;
    v->is_global = is_global;
    v->length = length;
    v->cty = cty;
    return v;
}

Scope *new_scope(Scope **parent)
{
    Scope *scope = (Scope *)calloc(1, sizeof(Scope));
    scope->variables = new_map();
    if (parent != NULL)
    {
        (*parent)->inner = scope;
        scope->outer = *parent;
    }

    return scope;
}

// 内側のスコープから外側に向かって探索する
Variable *find_var(Scope *sc, char *key, size_t length)
{
    for (Scope *s = sc; s != NULL; s = s->outer)
    {
        Variable *v = map_get(s->variables, key, length);
        if (v != NULL)
        {
            return v;
        }
    }

    return NULL;
}