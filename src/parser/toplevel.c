#include "peachcc.h"

// expr
Program *parse(TokenList *tokens)
{
    // 各ASTノードがトークンへのポインタを持っているので，cur_gのfreeは最後までしてはならない．
    cur_g = calloc(1, sizeof(Token));
    cur_g = current_token(tokens);
    local_variables_g = new_map();
    total_stack_size_in_fn_g = 0;

    Program *program = new_program();
    while (!at_eof(tokens))
    {
        push_statement(program->stmts, statement(tokens));
    }
    return program;
}