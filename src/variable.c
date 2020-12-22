#include "peachcc.h"

LocalVariable *new_local_var(char *str, size_t length, size_t stack_offset)
{
    LocalVariable *lv = (LocalVariable *)calloc(1, sizeof(LocalVariable));
    lv->str = str;
    lv->length = length;
    lv->stack_offset = stack_offset;
    return lv;
}