#include "peachcc.h"

static CType *new_ctype(CTypeKind k, size_t size, int align)
{
    CType *cty = calloc(1, sizeof(CType));
    cty->kind = k;
    cty->size = size;
    cty->align = align;
    return cty;
}

CType *new_int()
{
    return new_ctype(TY_INT, 4, 4);
}
CType *new_char()
{
    return new_ctype(TY_CHAR, 1, 1);
}
CType *new_void()
{
    return new_ctype(TY_VOID, 0, 0);
}
CType *new_ptr(CType *base)
{
    CType *ptr = new_ctype(TY_PTR, 8, 8);
    ptr->base = base;
    return ptr;
}
CType *new_array(CType *base, int array_len)
{
    CType *array = new_ctype(TY_ARRAY, base->size * array_len, base->align);
    array->base = base;
    array->size = base->size * array_len;
    array->array_len = array_len;
    return array;
}
CType *new_struct(Map *members)
{
    CType *st = new_ctype(TY_STRUCT, 0, 1);
    for (size_t i = 0; i < members->keys->len; i++)
    {
        Member *mem = members->vals->data[i];
        st->size += mem->cty->size;

        // 構造体で最も大きなサイズを持つメンバにアラインする
        if (st->align < mem->cty->align)
        {
            st->align = mem->cty->align;
        }
    }
    st->members = members;

    return st;
}

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
int align_to(int n, int align)
{
    return (n + align - 1) / align * align;
}