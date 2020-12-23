#include "peachcc.h"

CType *new_ctype(CTypeKind k, size_t size)
{
    CType *cty = calloc(1, sizeof(CType));
    cty->kind = k;
    cty->size = size;
    return cty;
}
