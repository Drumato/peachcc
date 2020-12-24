#include "peachcc.h"

static CType *new_ctype(CTypeKind k, size_t size)
{
    CType *cty = calloc(1, sizeof(CType));
    cty->kind = k;
    cty->size = size;
    return cty;
}

CType *new_int(void)
{
    return new_ctype(TY_INT, 8);
}
CType *new_ptr(CType *base)
{
    CType *ptr = new_ctype(TY_PTR, 8);
    ptr->base = base;
    return ptr;
}
CType *new_array(CType *base, int array_len)
{
    CType *array = new_ctype(TY_ARRAY, base->size * array_len);
    array->base = base;
    array->size = base->size * array_len;
    array->array_len = array_len;
    return array;
}