#include "peachcc.h"

static void type_specifier(TokenList *tokens);
static Token *parameter_declaration(TokenList *tokens);
static Token *init_declarator_list(TokenList *tokens);
static Token *init_declarator(TokenList *tokens);
static Token *direct_declarator(TokenList *tokens);

// declaration-specifiers init-declarator-list? ';'
// 現状はこんな感じで良い
Token *declaration(TokenList *tokens)
{
    type_specifier(tokens);

    Token *id = init_declarator_list(tokens);
    expect(tokens, TK_SEMICOLON);

    return id;
}

// '(' parameter-declaration (',' parameter-declaration)? ')'
// 今の所単なる識別子名のリスト
Vector *parameter_list(TokenList *tokens)
{
    Vector *params = new_vec();
    expect(tokens, TK_LPAREN);

    while (!try_eat(tokens, TK_RPAREN))
    {
        Token *id = parameter_declaration(tokens);
        insert_localvar_to_fn_env(id);
        char *copied_name = (char *)calloc(id->length, sizeof(char));
        strncpy(copied_name, id->str, id->length);
        copied_name[id->length] = 0;
        vec_push(params, copied_name);

        if (try_eat(tokens, TK_RPAREN))
        {
            break;
        }
        expect(tokens, TK_COMMA);
    }

    return params;
}

// declaration-specifiers declarator
static Token *parameter_declaration(TokenList *tokens)
{
    declaration_specifiers(tokens);
    return declarator(tokens);
}

// type-specifier
// 後々 type-specifier declaration-specifiers? に変更
// TypeSpecifier{Type *ty; bool is_static; } みたいなのを返すような変更も必要かも
void declaration_specifiers(TokenList *tokens)
{
    type_specifier(tokens);
}

// init-declarator
// 後々 init-declarator (',' init-declarator)? に変える
static Token *init_declarator_list(TokenList *tokens)
{
    return init_declarator(tokens);
}

// declarator
// 後々 declarator '=' initializer に変える
static Token *init_declarator(TokenList *tokens)
{
    return declarator(tokens);
}

// direct-declarator
// 後々 pointer? direct-declarator に変える
// 返り値の型も変わる
Token *declarator(TokenList *tokens)
{
    return direct_declarator(tokens);
}

// identifier
// 実際の仕様からかなり削っているので注意
// 返り値の型も変更する必要あり
static Token *direct_declarator(TokenList *tokens)
{
    Token *id = expect_identifier(tokens);
    return id;
}

// "int"
static void type_specifier(TokenList *tokens)
{
    expect(tokens, TK_INT);
}