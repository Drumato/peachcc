#include "lexer.h"
#include "token.h"
#include <ctype.h>
#include <stdio.h>

static TokenKind char_to_operator(char op);

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

        if (*p == '+' || *p == '-')
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

        fprintf(stderr, "can't tokenize\n");
    }

    push_token(tokens, new_token(TK_EOF, p));
    return;
}

static TokenKind char_to_operator(char op)
{
    switch (op)
    {
    case '+':
        return TK_PLUS;
    case '-':
        return TK_MINUS;
    default:
        return TK_EOF;
    }
}