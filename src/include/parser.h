#ifndef PEACHCC_PARSER_H_
#define PEACHCC_PARSER_H_

#include "token.h"
#include <stdbool.h>

TokenKind current_tk(TokenList *tokens);
bool consume(TokenList *tokens, TokenKind k);
void expect(TokenList *tokens, TokenKind k);
int expect_integer_literal(TokenList *tokens);
bool at_eof(TokenList *tokens);
#endif