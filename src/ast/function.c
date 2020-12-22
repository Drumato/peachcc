#include "peachcc.h"
Function *new_function(char *name, size_t length)
{
    Function *f = (Function *)calloc(1, sizeof(Function));
    f->copied_name = calloc(length, sizeof(char));
    strncpy(f->copied_name, name, length);
    f->copied_name[length] = 0;
    return f;
}