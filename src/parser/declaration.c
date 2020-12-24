#include "peachcc.h"

static CType *type_specifier(TokenList *tokens);
static void pointer(CType **cty, TokenList *tokens);
static Decl *parameter_declaration(TokenList *tokens);
static Token *init_declarator_list(CType **cty, TokenList *tokens);
static Token *init_declarator(CType **cty, TokenList *tokens);
static Token *direct_declarator(CType **cty, TokenList *tokens);
static void type_suffix(CType **cty, TokenList *tokens);

// declaration-specifiers init-declarator-list? ';'
// 現状はこんな感じで良い
Decl *declaration(TokenList *tokens)
{
    Token *id;
    CType *cty;
    decl_spec(tokens, &id, &cty);
    expect(tokens, TK_SEMICOLON);

    Decl *decl = (Decl *)calloc(1, sizeof(Decl));
    decl->cty = cty;
    decl->id = id;

    return decl;
}

void decl_spec(TokenList *tokens, Token **id, CType **cty)
{
    *cty = type_specifier(tokens);

    *id = init_declarator_list(cty, tokens);
}

// '(' parameter-declaration (',' parameter-declaration)? ')'
// 今の所単なる識別子名のリスト
Vector *parameter_list(TokenList *tokens)
{
    Vector *params = new_vec();
    expect(tokens, TK_LPAREN);

    while (!try_eat(tokens, TK_RPAREN))
    {
        Decl *param = parameter_declaration(tokens);
        insert_localvar_to_fn_env(param->id, param->cty);
        char *copied_name = (char *)calloc(param->id->length, sizeof(char));
        strncpy(copied_name, param->id->str, param->id->length);
        copied_name[param->id->length] = 0;
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
static Decl *parameter_declaration(TokenList *tokens)
{
    Decl *param = (Decl *)calloc(1, sizeof(Decl));
    CType *cty = declaration_specifiers(tokens);
    Token *id = declarator(&cty, tokens);
    param->cty = cty;
    param->id = id;
    return param;
}

// type-specifier
// 後々 type-specifier declaration-specifiers? に変更
// TypeSpecifier{Type *ty; bool is_static; } みたいなのを返すような変更も必要かも
CType *declaration_specifiers(TokenList *tokens)
{
    return type_specifier(tokens);
}

// init-declarator
// 後々 init-declarator (',' init-declarator)? に変える
static Token *init_declarator_list(CType **cty, TokenList *tokens)
{
    return init_declarator(cty, tokens);
}

// declarator
// 後々 declarator '=' initializer に変える
static Token *init_declarator(CType **cty, TokenList *tokens)
{
    return declarator(cty, tokens);
}

// pointer? direct-declarator
Token *declarator(CType **cty, TokenList *tokens)
{
    if (eatable(tokens, TK_STAR))
    {
        pointer(cty, tokens);
    }

    return direct_declarator(cty, tokens);
}

// '*'*
static void pointer(CType **cty, TokenList *tokens)
{
    while (try_eat(tokens, TK_STAR))
    {
        CType *base = *cty;
        *cty = new_ptr(base);
    }
}

// identifier type-suffix?
// 実際の仕様からかなり削っているので注意
static Token *direct_declarator(CType **cty, TokenList *tokens)
{
    Token *id = expect_identifier(tokens);

    if (!eatable(tokens, TK_LBRACKET))
    {
        return id;
    }

    type_suffix(cty, tokens);
    return id;
}

// (('[' integer-literal ']')*)
// 後々もっと変更する必要あり
static void type_suffix(CType **cty, TokenList *tokens)
{
    if (!try_eat(tokens, TK_LBRACKET))
    {
        return;
    }
    int array_len = expect_integer_literal(tokens);
    expect(tokens, TK_RBRACKET);
    type_suffix(cty, tokens);
    *cty = new_array(*cty, array_len);
}

// "int"
static CType *type_specifier(TokenList *tokens)
{
    Token *cur = current_token(tokens);
    switch (cur->kind)
    {
    case TK_INT:
    {
        expect(tokens, TK_INT);
        return new_int();
    }
    default:
        error_at(cur->str, "not allowed it in type-specifier");
    }
    return NULL;
}