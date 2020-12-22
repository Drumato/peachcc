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

/// identifier '(' (param_decl (',' param_decl)* )? ')'
static Function *function(TokenList *tokens)
{
    total_stack_size_in_fn_g = 0;

    Token *id = current_token(tokens);
    char *func_name = id->str;
    size_t func_name_length = id->length;
    expect(tokens, TK_IDENTIFIER);

    Function *f = new_function(func_name, func_name_length);
    Map *local_variables = new_map();
    local_variables_in_cur_fn_g = local_variables;
    f->local_variables = local_variables;

    expect(tokens, TK_LPAREN);
    Vector *params = new_vec();

    LocalVariable *lv;
    while (!try_eat(tokens, TK_RPAREN))
    {
        id = current_token(tokens);
        char *copied_name = (char *)calloc(id->length, sizeof(char));
        strncpy(copied_name, id->str, id->length);
        copied_name[id->length] = 0;
        vec_push(params, copied_name);
        expect(tokens, TK_IDENTIFIER);
        if ((lv = (LocalVariable *)map_get(local_variables_in_cur_fn_g, id->str, id->length)) == NULL)
        {
            total_stack_size_in_fn_g += 8;
            lv = new_local_var(id->str, id->length, 0);
            map_put(local_variables_in_cur_fn_g, id->str, lv);
        }

        if (try_eat(tokens, TK_RPAREN))
        {
            break;
        }
        expect(tokens, TK_COMMA);
    }

    Vector *stmts = compound_stmt(tokens);

    // とりあえずパーサでスタックに割り当ててしまう
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