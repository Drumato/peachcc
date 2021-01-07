
#include "peachcc.h"
static size_t line_num_g;
static TokenKind char_to_operator(char op);
static Token *multilength_symbol(char *ptr);
static Token *identifier(char *ptr);
static Token *c_keyword(char *ptr);
static Token *char_literal(char *ptr);
static Token *string_literal(char *ptr);
static int escaped_char(char *p, int *length);

void tokenize(TokenList *tokens, char *p)
{
    line_num_g = 1;
    Token *t = NULL;
    while (*p)
    {
        // if文の順番を必ず保持すること．
        // 順番を変更すると正しくtokenize出来ない

        // 空白文字をスキップ
        if (isspace(*p))
        {
            if (*p == '\n')
            {
                line_num_g++;
            }
            p++;
            continue;
        }
        if (!strncmp(p, "//", 2))
        {
            p = p + 2;
            while (*p != '\n')
                p++;
            continue;
        }
        if (!strncmp(p, "/*", 2))
        {
            char *q = strstr(p + 2, "*/");
            if (!q)
                error_at(p, line_num_g, "unclosed block comment");

            while (p != q)
            {
                // 改行数える
                if (*p == '\n')
                {
                    line_num_g++;
                }
                p++;
            }
            p = q + 2;
            continue;
        }

        if ((t = char_literal(p)) != NULL)
        {
            p = p + t->length;
            vec_push(tokens, t);
            continue;
        }
        if ((t = string_literal(p)) != NULL)
        {
            p = p + t->length;
            vec_push(tokens, t);
            continue;
        }

        // 識別子よりも先に予約語のチェック
        if ((t = c_keyword(p)) != NULL)
        {
            p = p + t->length;
            vec_push(tokens, t);
            continue;
        }

        if (isalpha(*p) || *p == '_')
        {
            Token *id = identifier(p);
            p = p + id->length;
            vec_push(tokens, id);
            continue;
        }

        // 先に二文字以上の記号をtokenize可能かチェックすることで，
        // 一文字の記号のtokinizeがstrchr()で簡略化できる．
        if ((t = multilength_symbol(p)) != NULL)
        {
            p = p + t->length;
            vec_push(tokens, t);
            continue;
        }

        if (strchr("+-*/(){}[]<>;=,%&!?:.", *p) != NULL)
        {
            TokenKind op = char_to_operator(*p);
            vec_push(tokens, new_token(op, p++, 1, line_num_g));
            continue;
        }

        if (isdigit(*p))
        {
            int value = 0;
            char *intlit_loc = p;

            if (*p == '0' && isdigit(*(p + 1)))
            {
                // 8進数
                value = strtol(p + 1, &p, 8);
            }
            else if (*p == '0' && *(p + 1) == 'b' && isdigit(*(p + 2)))
            {
                // 2進数
                value = strtol(p + 2, &p, 2);
            }
            else if (*p == '0' && *(p + 1) == 'x' && isdigit(*(p + 2)))
            {
                // 16進数
                value = strtol(p + 2, &p, 16);
            }
            else
            {
                value = strtol(p, &p, 10);
            }
            Token *intlit = new_integer_token(intlit_loc, value, p - intlit_loc, line_num_g);
            vec_push(tokens, intlit);
            continue;
        }

        error_at(p, line_num_g, "can't tokenize");
    }

    vec_push(tokens, new_token(TK_EOF, p, 0, line_num_g));
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
    return new_identifier_token(ptr, p - ptr, line_num_g);
}

static Token *multilength_symbol(char *ptr)
{
    char *symbols[] = {"==", "!=", "<=", ">=", "++", "--", "&&", "||", "...", "->", NULL};
    TokenKind kinds[] = {TK_EQ, TK_NTEQ, TK_LEEQ, TK_GEEQ, TK_INCREMENT, TK_DECREMENT, TK_LOGAND, TK_LOGOR, TK_ELLIPSIS, TK_ARROW};

    // 必ずsymbols[i] != NULLと比較すること．
    // kindsとsymbolsには要素数の差がある(len(symbols == len(kinds) - 1))
    for (int i = 0; symbols[i] != NULL; i++)
    {
        if (!strncmp(ptr, symbols[i], strlen(symbols[i])))
        {
            return new_token(kinds[i], ptr, strlen(symbols[i]), line_num_g);
        }
    }

    return NULL;
}

// 識別子のスキャン
static Token *c_keyword(char *ptr)
{
    char *keywords[] = {
        "return", "if", "else", "for", "while",
        "int", "sizeof", "char", "static", "extern",
        "struct", "void", "long", "short", NULL};
    TokenKind kinds[] = {
        TK_RETURN, TK_IF, TK_ELSE, TK_FOR, TK_WHILE,
        TK_INT, TK_SIZEOF, TK_CHAR, TK_STATIC, TK_EXTERN,
        TK_STRUCT, TK_VOID, TK_LONG, TK_SHORT};
    // 必ずkeywords[i] != NULLと比較すること．
    // kindsとkeywordsには要素数の差がある(len(keywords == len(kinds) - 1))

    for (size_t i = 0; keywords[i] != NULL; i++)
    {
        size_t keyword_len = strlen(keywords[i]);
        bool starts_with_keyword = !strncmp(ptr, keywords[i], keyword_len);
        bool is_not_an_identifier = !isalnum(ptr[keyword_len]) && ptr[keyword_len] != '_';
        if (starts_with_keyword && is_not_an_identifier)
        {
            return new_token(kinds[i], ptr, keyword_len, line_num_g);
        }
    }

    return NULL;
}
// 文字リテラルのtokenize
static Token *char_literal(char *ptr)
{
    if (*ptr != '\'')
    {
        return NULL;
    }
    char *p = ptr + 1;
    int value = 0;
    if (*p == '\\')
    {
        int length = 0;
        value = escaped_char(p + 1, &length);
        p = p + length;
    }
    else
    {
        value = *p++;
    }
    char *end = strchr(p, '\'');
    if (end == NULL)
    {
        error_at(p, line_num_g, "unclosed char literal found");
        return NULL;
    }
    end++;

    Token *char_lit = new_integer_token(ptr, value, end - ptr, line_num_g);
    return char_lit;
}
// 文字列リテラルのtokenize
static Token *string_literal(char *ptr)
{
    if (*ptr != '"')
    {
        return NULL;
    }
    char *p = ptr + 1;

    int cur_pos = 0;
    char *buf = (char *)calloc(1024, sizeof(char));
    while (*p != '"')
    {
        if (*p == '\\')
        {
            int length;
            buf[cur_pos++] = escaped_char(p + 1, &length);
            p = p + length;
            continue;
        }
        buf[cur_pos++] = *p;
        p++;
    }
    p++;

    Token *str_lit = new_string_token(ptr, p - ptr, line_num_g);
    str_lit->copied_contents = buf;

    return str_lit;
}

static TokenKind char_to_operator(char op)
{
    switch (op)
    {
    case '.':
        return TK_DOT;
    case '+':
        return TK_PLUS;
    case '-':
        return TK_MINUS;
    case '*':
        return TK_STAR;
    case '/':
        return TK_SLASH;
    case '%':
        return TK_PERCENT;
    case '&':
        return TK_AMPERSAND;
    case '(':
        return TK_LPAREN;
    case ')':
        return TK_RPAREN;
    case '[':
        return TK_LBRACKET;
    case ']':
        return TK_RBRACKET;
    case '{':
        return TK_LBRACE;
    case '}':
        return TK_RBRACE;
    case '<':
        return TK_LE;
    case '>':
        return TK_GE;
    case ';':
        return TK_SEMICOLON;
    case ',':
        return TK_COMMA;
    case '=':
        return TK_ASSIGN;
    case '!':
        return TK_BANG;
    case '?':
        return TK_QUESTION;
    case ':':
        return TK_COLON;
    default:
        return TK_EOF;
    }
}

// エスケープシーケンスを読む
static int escaped_char(char *p, int *length)
{
    *length = 2;
    // エスケープシーケンスを返してもいいらしいけど，あえて整数をそのまま返す
    switch (*p)
    {
    case 'a':
        return 0x07;
    case 'b':
        return 0x08;
    case 't':
        return 0x09;
    case 'n':
        return 0x0a;
    case 'v':
        return 0x0b;
    case 'f':
        return 0x0c;
    case 'r':
        return 0x0d;
    // [GNU] \e for the ASCII escape character is a GNU C extension.
    case 'e':
        return 0x1b;
    case 'x':
    {
        char *hex = p + 1;
        int value = strtol(hex, &hex, 16);
        *length = hex - p;
        return value;
    }
    default:
    {
        if ('0' <= *p && *p <= '7')
        {
            char *start = p;
            int value = *p++ - '0';
            if ('0' <= *p && *p <= '7')
            {
                value = (value << 3) + (*p++ - '0');
                if ('0' <= *p && *p <= '7')
                    value = (value << 3) + (*p++ - '0');
            }
            *length = p - start;
            return value;
        }
        return *p;
    }
    }
}