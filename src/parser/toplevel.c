#include "peachcc.h"

// expr
Program *parse(TokenList *tokens)
{
    // 各ASTノードがトークンへのポインタを持っているので，cur_gのfreeは最後までしてはならない．
    cur_g = calloc(1, sizeof(Token));
    cur_g = current_token(tokens);

    Program *program = new_program();
    while (!at_eof(tokens))
    {
        push_statement(program->stmts, statement(tokens));
    }
    return program;
}