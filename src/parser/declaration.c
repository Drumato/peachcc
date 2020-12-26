#include "peachcc.h"

static CType *type_specifier(TokenList *tokens);
static void pointer(CType **cty, TokenList *tokens);
static Decl *parameter_declaration(TokenList *tokens);
static Token *init_declarator_list(CType **cty, TokenList *tokens);
static Token *init_declarator(CType **cty, TokenList *tokens);
static Token *direct_declarator(CType **cty, TokenList *tokens);
static void type_suffix(CType **cty, TokenList *tokens);
static void storage_class_specifier(TokenList *tokens, DeclarationSpecifier **decl);

// declaration-specifiers init-declarator-list? ';'
// 現状はこんな感じで良い
Decl *declaration(TokenList *tokens)
{
    Token *id;
    DeclarationSpecifier *declspec = decl_spec(tokens, &id);
    expect(tokens, TK_SEMICOLON);

    Decl *decl = (Decl *)calloc(1, sizeof(Decl));
    decl->cty = declspec->cty;
    decl->id = id;

    return decl;
}

// declaration_specifiersのラッパー
// external declarationのパース時に，
// 関数定義かグローバル変数宣言かをチェックするときにのみ用いる
DeclarationSpecifier *decl_spec(TokenList *tokens, Token **id)
{
    DeclarationSpecifier *decl = declaration_specifiers(tokens);

    *id = init_declarator_list(&decl->cty, tokens);
    return decl;
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
    DeclarationSpecifier *decl_spec = declaration_specifiers(tokens);
    Token *id = declarator(&decl_spec->cty, tokens);
    param->cty = decl_spec->cty;
    param->id = id;
    return param;
}

// type-specifier (type-specifier | storage-class specifier)*
// 後々 type-specifier declaration-specifiers? に変更
DeclarationSpecifier *declaration_specifiers(TokenList *tokens)
{
    DeclarationSpecifier *decl_spec = (DeclarationSpecifier *)calloc(1, sizeof(DeclarationSpecifier));
    decl_spec->is_static = false;

    while (true)
    {
        if (is_typename(tokens))
        {
            CType *ty = type_specifier(tokens);
            decl_spec->cty = ty;
            continue;
        }
        if (start_storage_class(tokens))
        {
            storage_class_specifier(tokens, &decl_spec);
            continue;
        }
        break;
    }

    return decl_spec;
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

// "int" | "char"
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
    case TK_CHAR:
    {
        expect(tokens, TK_CHAR);
        return new_char();
    }
    default:
        error_at(cur->str, "not allowed it in type-specifier");
    }
    return NULL;
}

// "static"
static void storage_class_specifier(TokenList *tokens, DeclarationSpecifier **decl)
{
    expect(tokens, TK_STATIC);
    (*decl)->is_static = true;
}