#include "lexer.h"
#include "debug.h"
#include "token.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

#include <string.h>
static TokenKind char_to_operator(char op);
static Token *multilength_symbol(char **ptr);

void tokenize(TokenList *tokens, char *p)
{
    arraystack_init(tokens, sizeof(Token));

    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        Token *t;
        if ((t = multilength_symbol(&p)) != NULL)
        {
            push_token(tokens, t);
            continue;
        }

        if (strchr("+-*/()<>", *p) != NULL)
        {
            TokenKind op = char_to_operator(*p);
            push_token(tokens, new_token(op, p++));
            continue;
        }

        if (isdigit(*p))
        {
            Token *intlit = new_integer_token(p, strtol(p, &p, 10));
            push_token(tokens, intlit);
            continue;
        }

        error_at(p, "can't tokenize");
    }

    push_token(tokens, new_token(TK_EOF, p));
    return;
}

static Token *multilength_symbol(char **ptr)
{
    char *symbols[] = {"==", "!=", "<=", ">=", NULL};
    TokenKind kinds[] = {TK_EQ, TK_NTEQ, TK_LEEQ, TK_GEEQ};

    for (int i = 0; symbols[i] != NULL; i++)
    {
        if (!strncmp(*ptr, symbols[i], strlen(symbols[i])))
        {
            char *p = *ptr;
            (*ptr) += 2;
            return new_token(kinds[i], p);
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
    default:
        return TK_EOF;
    }
}