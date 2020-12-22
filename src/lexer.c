
#include "peachcc.h"
static TokenKind char_to_operator(char op);
static Token *multilength_symbol(char *ptr);
static Token *identifier(char *ptr);

void tokenize(TokenList *tokens, char *p)
{
    while (*p)
    {

        // if文の順番を必ず保持すること．
        // 順番を変更すると正しくtokenize出来ない

        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (isalpha(*p) || *p == '_')
        {
            Token *id = identifier(p);
            p += id->length;
            push_token(tokens, id);
            continue;
        }

        // 先に二文字以上の記号をtokenize可能かチェックすることで，
        // 一文字の記号のtokinizeがstrchr()で簡略化できる．
        Token *t;
        if ((t = multilength_symbol(p)) != NULL)
        {
            p += t->length;
            push_token(tokens, t);
            continue;
        }

        if (strchr("+-*/()<>;=", *p) != NULL)
        {
            TokenKind op = char_to_operator(*p);
            push_token(tokens, new_token(op, p++, 1));
            continue;
        }

        if (isdigit(*p))
        {
            char *intlit_loc = p;
            int value = strtol(p, &p, 10);
            Token *intlit = new_integer_token(intlit_loc, value, p - intlit_loc);
            push_token(tokens, intlit);
            continue;
        }

        error_at(p, "can't tokenize");
    }

    push_token(tokens, new_token(TK_EOF, p, 0));
    return;
}

// 識別子トークンのスキャン
// ('_' | [a-zA-Z]) ('_' | [a-zA-Z0-9])*
static Token *identifier(char *ptr)
{
    char *p = ptr;
    while (isalpha(*p) || *p == '_')
    {
        p++;
    }
    while (isalnum(*p) || *p == '_')
    {
        p++;
    }
    return new_identifier_token(ptr, p - ptr);
}

static Token *multilength_symbol(char *ptr)
{
    char *symbols[] = {"==", "!=", "<=", ">=", NULL};
    TokenKind kinds[] = {TK_EQ, TK_NTEQ, TK_LEEQ, TK_GEEQ};

    // 必ずsymbols[i] != NULLと比較すること．
    // kindsとsymbolsには要素数の差がある(len(symbols == len(kinds) - 1))
    for (int i = 0; symbols[i] != NULL; i++)
    {
        if (!strncmp(ptr, symbols[i], strlen(symbols[i])))
        {
            char *p = ptr;
            return new_token(kinds[i], p, 2);
        }
    }

    return NULL;
}

static TokenKind char_to_operator(char op)
{
    switch (op)
    {
    case '+':
        return TK_PLUS;
    case '-':
        return TK_MINUS;
    case '*':
        return TK_STAR;
    case '/':
        return TK_SLASH;
    case '(':
        return TK_LPAREN;
    case ')':
        return TK_RPAREN;
    case '<':
        return TK_LE;
    case '>':
        return TK_GE;
    case ';':
        return TK_SEMICOLON;
    case '=':
        return TK_ASSIGN;
    default:
        return TK_EOF;
    }
}