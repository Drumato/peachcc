#include "peachcc.h"

// 次のトークンが期待している種類のときには，
// トークンを1つ読み進めて真を返す．
// 読み進められなかった時は偽を返す．
bool try_eat(TokenList *tokens, TokenKind k)
{
    if (current_tk(tokens) != k)
        return false;

    progress(tokens);
    cur_g = current_token(tokens);
    return true;
}

// 次のトークンが期待している記号のときには，トークンを1つ読み進める．
// それ以外の場合にはエラーを報告する．
void expect(TokenList *tokens, TokenKind k)
{
    if (!try_eat(tokens, k))
    {
        error_at(cur_g->str, "unexpected token");
        exit(1);
    }
    cur_g = current_token(tokens);
}

// 次のトークンが整数の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
int expect_integer_literal(TokenList *tokens)
{
    if (cur_g->kind != TK_INTEGER_LITERAL)
    {
        error_at(cur_g->str, "expected integer literal");
    }
    int value = cur_g->value;
    progress(tokens);
    cur_g = current_token(tokens);
    return value;
}

// 現在見ているトークンが渡されたkと同じ種類かチェック
bool eatable(TokenList *tokens, TokenKind k)
{
    return current_tk(tokens) == k;
}

bool at_eof(TokenList *tokens)
{
    return current_tk(tokens) == TK_EOF;
}

Token *try_eat_identifier(TokenList *tokens)
{
    Token *ident_loc = current_token(tokens);
    if (!try_eat(tokens, TK_IDENTIFIER))
    {
        return NULL;
    }

    return ident_loc;
}

Token *expect_identifier(TokenList *tokens)
{
    Token *id = current_token(tokens);
    expect(tokens, TK_IDENTIFIER);
    return id;
}

// 識別子をローカル変数のマップに登録する
void insert_localvar_to_fn_env(Token *id, CType *cty)
{
    LocalVariable *lv;
    if ((lv = (LocalVariable *)map_get(local_variables_in_cur_fn_g, id->str, id->length)) == NULL)
    {
        total_stack_size_in_fn_g += cty->size;
        lv = new_local_var(id->str, id->length, cty, 0);
        lv->cty = cty;
        map_put(local_variables_in_cur_fn_g, id->str, lv);
    }
}

bool is_typename(TokenList *tokens)
{
    TokenKind cur = current_tk(tokens);
    return cur == TK_INT || cur == TK_CHAR;
}