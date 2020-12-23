#include "peachcc.h"

static Function *function(TokenList *tokens);
// expr
Program *parse(TokenList *tokens)
{
    // 各ASTノードがトークンへのポインタを持っているので，cur_gのfreeは最後までしてはならない．
    cur_g = calloc(1, sizeof(Token));
    cur_g = current_token(tokens);

    Program *program = new_program();
    Vector *fns = new_vec();
    while (!at_eof(tokens))
    {
        vec_push(fns, function(tokens));
    }
    program->functions = fns;
    return program;
}

/// declaration-specifiers declarator declaration_list? compound-statement
static Function *function(TokenList *tokens)
{
    total_stack_size_in_fn_g = 0;

    declaration_specifiers(tokens);

    Token *fn_id = declarator(tokens);
    char *func_name = fn_id->str;
    size_t func_name_length = fn_id->length;

    Function *f = new_function(func_name, func_name_length);
    Map *local_variables = new_map();
    local_variables_in_cur_fn_g = local_variables;
    f->local_variables = local_variables;

    Vector *params = parameter_list(tokens);

    Vector *stmts = compound_stmt(tokens);

    // とりあえずパーサでスタックに割り当ててしまう
    LocalVariable *lv;
    size_t total_stack_size = total_stack_size_in_fn_g;
    for (size_t i = 0; i < local_variables_in_cur_fn_g->keys->len; i++)
    {
        lv = local_variables_in_cur_fn_g->vals->data[i];
        lv->stack_offset = total_stack_size;
        total_stack_size -= 8;
    }

    f->stmts = stmts;
    f->params = params;
    f->stack_size = total_stack_size_in_fn_g;
    return f;
}