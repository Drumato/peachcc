#include "peachcc.h"

static Function *function(TokenList *tokens);
// expr
TranslationUnit *parse(TokenList *tokens)
{
    // 各ASTノードがトークンへのポインタを持っているので，cur_gのfreeは最後までしてはならない．
    cur_g = calloc(1, sizeof(Token));
    cur_g = current_token(tokens);

    TranslationUnit *translation_unit = new_translation_unit();
    Vector *fns = new_vec();
    while (!at_eof(tokens))
    {
        // 宣言の前半部分をパースして，関数宣言の始まりでありLPARENが見えるかどうか調べる
        // もし見えれば，それは関数定義．そうでなければグローバル変数，ということになる．
        int cur_pos = tokens->pos;
        Token *id;
        CType *cty;
        decl_spec(tokens, &id, &cty);
        TokenKind decl_spec_end = current_tk(tokens);
        tokens->pos = cur_pos;

        if (decl_spec_end != TK_LPAREN)
        {
            // global-variable
            Decl *global_decl = declaration(tokens);
            char *copied_name = (char *)calloc(global_decl->id->length, sizeof(char));
            strncpy(copied_name, global_decl->id->str, global_decl->id->length);
            copied_name[global_decl->id->length] = 0;
            map_put(translation_unit->global_variables, copied_name, global_decl->cty);
            continue;
        }

        vec_push(fns, function(tokens));
    }
    translation_unit->functions = fns;
    return translation_unit;
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