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

    CType *cty = declaration_specifiers(tokens);

    Token *fn_id = declarator(&cty, tokens);
    char *func_name = fn_id->str;
    size_t func_name_length = fn_id->length;

    Function *f = new_function(func_name, func_name_length);
    Map *local_variables = new_map();
    local_variables_in_cur_fn_g = local_variables;
    f->local_variables = local_variables;
    f->return_type = cty;

    Vector *params = parameter_list(tokens);

    Vector *stmts = compound_stmt(tokens);

    f->stmts = stmts;
    f->params = params;
    f->stack_size = total_stack_size_in_fn_g;
    return f;
}