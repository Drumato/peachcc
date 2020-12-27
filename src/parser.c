#include "peachcc.h"

// statement parser
static Stmt *expr_stmt(TokenList *tokens);
static Stmt *return_stmt(TokenList *tokens);
static Stmt *if_stmt(TokenList *tokens);
static Stmt *for_stmt(TokenList *tokens);
static Stmt *while_stmt(TokenList *tokens);
static Vector *block_item_list(TokenList *tokens);
static Vector *compound_stmt(TokenList *tokens);
static Stmt *statement(TokenList *tokens);

// declaration parser
static Decl *declaration(TokenList *tokens);
static DeclarationSpecifier *declaration_specifiers(TokenList *tokens);
static Token *declarator(CType **cty, TokenList *tokens);
static Vector *parameter_list(TokenList *tokens);
static DeclarationSpecifier *decl_spec(TokenList *tokens, Token **id);

static CType *type_specifier(TokenList *tokens);
static void pointer(CType **cty, TokenList *tokens);
static Decl *parameter_declaration(TokenList *tokens);
static Token *init_declarator_list(CType **cty, TokenList *tokens);
static Token *init_declarator(CType **cty, TokenList *tokens);
static Token *direct_declarator(CType **cty, TokenList *tokens);
static void type_suffix(CType **cty, TokenList *tokens);
static void storage_class_specifier(TokenList *tokens, DeclarationSpecifier **decl);

// expr parsers;
static Expr *conditional(TokenList *tokens);
static Expr *expression(TokenList *tokens);
static Expr *assign(TokenList *tokens);
static Expr *logor(TokenList *tokens);
static Expr *logand(TokenList *tokens);
static Expr *equality(TokenList *tokens);
static Expr *relation(TokenList *tokens);
static Expr *addition(TokenList *tokens);
static Expr *multiplication(TokenList *tokens);
static Expr *prefix_unary(TokenList *tokens);
static Expr *postfix_unary(TokenList *tokens);
static Expr *primary(TokenList *tokens);
static Expr *paren_expr(TokenList *tokens);
static Expr *ident_expr(TokenList *tokens, Token *ident_loc);
static Expr *call_expr(TokenList *tokens, Expr *id);

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
        error_at(cur_g->str, cur_g->line, "unexpected token");
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
        error_at(cur_g->str, cur_g->line, "expected integer literal");
    }
    int value = cur_g->value;
    progress(tokens);
    cur_g = current_token(tokens);
    return value;
}

// 次のトークンが文字列リテラルの場合，トークンを1つ読み進めてその内容を返す．
char *expect_string_literal(TokenList *tokens)
{
    if (cur_g->kind != TK_STRING_LITERAL)
    {
        fprintf(stderr, "%d\n", cur_g->kind);
        error_at(cur_g->str, cur_g->line, "expected string literal");
    }
    char *str = cur_g->copied_contents;
    progress(tokens);
    cur_g = current_token(tokens);
    return str;
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
        total_stack_size_in_fn_g = total_stack_size_in_fn_g + cty->size;
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

// "static"
bool start_storage_class(TokenList *tokens)
{
    return current_tk(tokens) == TK_STATIC;
}

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
        error_at(cur->str, cur->line, "not allowed it in type-specifier");
    }
    return NULL;
}

// "static"
static void storage_class_specifier(TokenList *tokens, DeclarationSpecifier **decl)
{
    expect(tokens, TK_STATIC);
    (*decl)->is_static = true;
}

// assign
static Expr *expression(TokenList *tokens)
{
    return assign(tokens);
}

// logor ("=" assign)?
static Expr *assign(TokenList *tokens)
{
    Expr *e = conditional(tokens);
    if (try_eat(tokens, TK_ASSIGN))
    {
        Token *loc = cur_g;
        e = new_binop(EX_ASSIGN, e, assign(tokens), loc->str, loc->line);
    }

    return e;
}

// logor ('?' expression ':' conditinal)?
static Expr *conditional(TokenList *tokens)
{
    Token *loc = cur_g;
    Expr *e = logor(tokens);
    if (!try_eat(tokens, TK_QUESTION))
    {
        return e;
    }

    Expr *cond = expression(tokens);
    expect(tokens, TK_COLON);

    e = new_conditional_expr(cond, e, conditional(tokens), loc->str, loc->line);
    return e;
}

// logand ('||' logand)*
static Expr *logor(TokenList *tokens)
{
    Expr *e = logand(tokens);
    for (;;)
    {
        Token *loc = cur_g;
        if (!try_eat(tokens, TK_LOGOR))
        {
            break;
        }
        e = new_binop(EX_LOGOR, e, logand(tokens), loc->str, loc->line);
    }
    return e;
}
// equality ('||' equality)*
static Expr *logand(TokenList *tokens)
{
    Expr *e = equality(tokens);
    for (;;)
    {
        Token *loc = cur_g;
        if (!try_eat(tokens, TK_LOGAND))
        {
            break;
        }
        e = new_binop(EX_LOGAND, e, equality(tokens), loc->str, loc->line);
    }
    return e;
}

// relation ('==' relation | '!=' relation)*
static Expr *equality(TokenList *tokens)
{
    Expr *e = relation(tokens);

    for (;;)
    {
        Token *loc = cur_g;
        // `==`
        if (try_eat(tokens, TK_EQ))
        {
            e = new_binop(EX_EQ, e, relation(tokens), loc->str, loc->line);
        }
        // `!=`
        else if (try_eat(tokens, TK_NTEQ))
        {
            e = new_binop(EX_NTEQ, e, relation(tokens), loc->str, loc->line);
        }
        else
        {
            break;
        }
    }

    return e;
}

// addition ('<' addition | '<=' addition | '>' addition | '>=' addition)*
static Expr *relation(TokenList *tokens)
{
    Expr *e = addition(tokens);

    for (;;)
    {
        Token *loc = cur_g;
        // `<`
        if (try_eat(tokens, TK_LE))
        {
            e = new_binop(EX_LE, e, addition(tokens), loc->str, loc->line);
        }
        // `<=`
        else if (try_eat(tokens, TK_LEEQ))
        {
            e = new_binop(EX_LEEQ, e, addition(tokens), loc->str, loc->line);
        }
        // `>`
        else if (try_eat(tokens, TK_GE))
        {
            e = new_binop(EX_GE, e, addition(tokens), loc->str, loc->line);
        }
        // `>=`
        else if (try_eat(tokens, TK_GEEQ))
        {
            e = new_binop(EX_GEEQ, e, addition(tokens), loc->str, loc->line);
        }
        else
        {
            break;
        }
    }
    return e;
}

// multiplication ('+' multiplication | '-' multiplication)*
static Expr *addition(TokenList *tokens)
{
    Expr *e = multiplication(tokens);

    for (;;)
    {
        Token *loc = cur_g;
        // `+`
        if (try_eat(tokens, TK_PLUS))
        {
            e = new_binop(EX_ADD, e, multiplication(tokens), loc->str, loc->line);
        }
        // `-`
        else if (try_eat(tokens, TK_MINUS))
        {
            e = new_binop(EX_SUB, e, multiplication(tokens), loc->str, loc->line);
        }
        else
        {
            break;
        }
    }

    return e;
}

// prefix_unary ('*' prefix_unary | '/' prefix_unary | '%' prefix_unary)*
static Expr *multiplication(TokenList *tokens)
{
    Expr *e = prefix_unary(tokens);

    for (;;)
    {
        Token *loc = cur_g;
        // `*`
        if (try_eat(tokens, TK_STAR))
        {
            e = new_binop(EX_MUL, e, prefix_unary(tokens), loc->str, loc->line);
        }
        // `/`
        else if (try_eat(tokens, TK_SLASH))
        {
            e = new_binop(EX_DIV, e, prefix_unary(tokens), loc->str, loc->line);
        }
        // `%`
        else if (try_eat(tokens, TK_PERCENT))
        {
            e = new_binop(EX_MOD, e, prefix_unary(tokens), loc->str, loc->line);
        }
        else
        {
            break;
        }
    }

    return e;
}

// ('+' | '-' | '*' | '&' | "sizeof" | "++" | "--") prefix_unary) | postfix_unary
static Expr *prefix_unary(TokenList *tokens)
{
    Expr *e;
    Token *loc = cur_g;

    if (try_eat(tokens, TK_PLUS))
        e = new_unop(EX_UNARY_PLUS, prefix_unary(tokens), loc->str, loc->line);
    else if (try_eat(tokens, TK_MINUS))
        e = new_unop(EX_UNARY_MINUS, prefix_unary(tokens), loc->str, loc->line);
    else if (try_eat(tokens, TK_STAR))
        e = new_unop(EX_UNARY_DEREF, prefix_unary(tokens), loc->str, loc->line);
    else if (try_eat(tokens, TK_AMPERSAND))
        e = new_unop(EX_UNARY_ADDR, prefix_unary(tokens), loc->str, loc->line);
    else if (try_eat(tokens, TK_SIZEOF))
        e = new_unop(EX_UNARY_SIZEOF, prefix_unary(tokens), loc->str, loc->line);
    else if (try_eat(tokens, TK_INCREMENT))
    {
        // ++x は単に x = x + 1として良い
        e = prefix_unary(tokens);
        e = new_binop(EX_ASSIGN, e, new_binop(EX_ADD, e, new_integer_literal(1, loc->str, loc->line), loc->str, loc->line), loc->str, loc->line);
    }
    else if (try_eat(tokens, TK_DECREMENT))
    {
        // --x は単に x = x - 1として良い
        e = prefix_unary(tokens);
        e = new_binop(EX_ASSIGN, e, new_binop(EX_SUB, e, new_integer_literal(1, loc->str, loc->line), loc->str, loc->line), loc->str, loc->line);
    }
    else
        e = postfix_unary(tokens);
    return e;
}

// primary (('[' expr ']')* | '++' )?
static Expr *postfix_unary(TokenList *tokens)
{
    Token *loc = current_token(tokens);
    Expr *e = primary(tokens);

    if (try_eat(tokens, TK_INCREMENT))
    {
        // i++ は (i = i + 1) - 1という式として見れる
        e = new_binop(EX_SUB, new_binop(EX_ASSIGN, e, new_binop(EX_ADD, e, new_integer_literal(1, loc->str, loc->line), loc->str, loc->line), loc->str, loc->line), new_integer_literal(1, loc->str, loc->line), loc->str, loc->line);
        return e;
    }
    if (try_eat(tokens, TK_DECREMENT))
    {
        // i++ は (i = i - 1) + 1という式として見れる
        e = new_binop(EX_ADD, new_binop(EX_ASSIGN, e, new_binop(EX_SUB, e, new_integer_literal(1, loc->str, loc->line), loc->str, loc->line), loc->str, loc->line), new_integer_literal(1, loc->str, loc->line), loc->str, loc->line);
        return e;
    }

    while (try_eat(tokens, TK_LBRACKET))
    {
        Expr *idx = expression(tokens);
        expect(tokens, TK_RBRACKET);
        // x[y] は単に *(x + y) として変換してしまう．
        e = new_unop(EX_UNARY_DEREF, new_binop(EX_ADD, e, idx, loc->str, loc->line), loc->str, loc->line);
    }

    return e;
}

// paren-expr | integer-literal | string-literal
static Expr *primary(TokenList *tokens)
{
    if (eatable(tokens, TK_LPAREN))
    {
        return paren_expr(tokens);
    }

    Token *ident_loc;
    if ((ident_loc = try_eat_identifier(tokens)) != NULL)
    {
        return ident_expr(tokens, ident_loc);
    }
    Token *loc = cur_g;
    if (eatable(tokens, TK_INTEGER_LITERAL))
    {
        int value = expect_integer_literal(tokens);
        return new_integer_literal(value, loc->str, loc->line);
    }

    char *contents = expect_string_literal(tokens);
    return new_string_literal(contents, loc->str, loc->line);
}

// '(' expr ')'
static Expr *paren_expr(TokenList *tokens)
{
    expect(tokens, TK_LPAREN);
    Expr *e = expression(tokens);
    expect(tokens, TK_RPAREN);

    return e;
}

/// identifier | call_expr
static Expr *ident_expr(TokenList *tokens, Token *ident_loc)
{
    Expr *id = new_identifier(ident_loc->str, ident_loc->length, ident_loc->line);

    if (!eatable(tokens, TK_LPAREN))
    {
        // 普通の識別子
        return id;
    }

    // 呼び出し式のパース
    return call_expr(tokens, id);
}

// '(' (expression (',' expression)* )? ')'
static Expr *call_expr(TokenList *tokens, Expr *id)
{
    id->kind = EX_CALL;
    expect(tokens, TK_LPAREN);
    Vector *params = new_vec();

    while (!try_eat(tokens, TK_RPAREN))
    {
        vec_push(params, expression(tokens));
        if (try_eat(tokens, TK_RPAREN))
        {
            break;
        }
        expect(tokens, TK_COMMA);
    }
    id->params = params;

    return id;
}

Stmt *statement(TokenList *tokens)
{
    switch (cur_g->kind)
    {
    case TK_RETURN:
        return return_stmt(tokens);
    case TK_LBRACE:
    {
        Token *loc = current_token(tokens);

        Stmt *s = new_stmt(ST_COMPOUND, loc->str, loc->line);
        s->body = compound_stmt(tokens);
        return s;
    }
    case TK_IF:
        return if_stmt(tokens);
    case TK_FOR:
        return for_stmt(tokens);
    case TK_WHILE:
        return while_stmt(tokens);
    default:
        return expr_stmt(tokens);
    }
}

// "return" expression ';'
static Stmt *return_stmt(TokenList *tokens)
{
    Token *loc = cur_g;
    expect(tokens, TK_RETURN);
    Expr *e = expression(tokens);
    expect(tokens, TK_SEMICOLON);
    Stmt *s = new_stmt(ST_RETURN, loc->str, loc->line);
    s->expr = e;
    return s;
}

// "while" '(' expression ')' statement
static Stmt *while_stmt(TokenList *tokens)
{
    Token *loc = cur_g;
    expect(tokens, TK_WHILE);
    expect(tokens, TK_LPAREN);
    Expr *condition = expression(tokens);
    expect(tokens, TK_RPAREN);

    Stmt *then = statement(tokens);
    Stmt *s = new_stmt(ST_WHILE, loc->str, loc->line);
    s->cond = condition;
    s->then = then;
    return s;
}

// "if" '(' expression ')' statement ("else" statement)?
static Stmt *if_stmt(TokenList *tokens)
{
    Token *loc = cur_g;
    expect(tokens, TK_IF);
    expect(tokens, TK_LPAREN);
    Expr *condition = expression(tokens);
    expect(tokens, TK_RPAREN);

    Stmt *then = statement(tokens);
    Stmt *s = new_stmt(ST_IF, loc->str, loc->line);
    s->cond = condition;
    s->then = then;

    if (!try_eat(tokens, TK_ELSE))
    {
        return s;
    }
    s->els = statement(tokens);

    return s;
}

// "for" '(' expression? ';' expression? ';' expression?')' statement
static Stmt *for_stmt(TokenList *tokens)
{
    Token *loc = cur_g;

    expect(tokens, TK_FOR);
    expect(tokens, TK_LPAREN);
    Stmt *s = new_stmt(ST_FOR, loc->str, loc->line);
    if (!try_eat(tokens, TK_SEMICOLON))
    {
        s->init = expression(tokens);
        expect(tokens, TK_SEMICOLON);
    }
    if (!try_eat(tokens, TK_SEMICOLON))
    {
        s->cond = expression(tokens);
        expect(tokens, TK_SEMICOLON);
    }
    if (!try_eat(tokens, TK_RPAREN))
    {
        s->inc = expression(tokens);
        expect(tokens, TK_RPAREN);
    }

    s->then = statement(tokens);

    return s;
}

// '{' block_item_list '}'
Vector *compound_stmt(TokenList *tokens)
{
    expect(tokens, TK_LBRACE);

    Vector *body = block_item_list(tokens);

    expect(tokens, TK_RBRACE);

    return body;
}

// expression ';'
static Stmt *expr_stmt(TokenList *tokens)
{
    Token *loc = cur_g;
    Expr *e = expression(tokens);
    expect(tokens, TK_SEMICOLON);
    Stmt *s = new_stmt(ST_EXPR, loc->str, loc->line);
    s->expr = e;
    return s;
}

// (declaration | statement)*
static Vector *block_item_list(TokenList *tokens)
{
    Vector *body = new_vec();

    while (!eatable(tokens, TK_RBRACE))
    {
        if (is_typename(tokens))
        {
            Decl *decl = declaration(tokens);
            insert_localvar_to_fn_env(decl->id, decl->cty);
            continue;
        }

        vec_push(body, statement(tokens));
    }
    return body;
}

static Function *function(TokenList *tokens);
// expr
TranslationUnit *parse(TokenList *tokens)
{
    // 各ASTノードがトークンへのポインタを持っているので，cur_gのfreeは最後までしてはならない．
    cur_g = calloc(1, sizeof(Token));
    cur_g = current_token(tokens);

    TranslationUnit *translation_unit = new_translation_unit();
    translation_unit->global_variables = new_map();
    global_variables_g = translation_unit->global_variables;
    Vector *fns = new_vec();
    while (!at_eof(tokens))
    {
        // 宣言の前半部分をパースして，関数宣言の始まりでありLPARENが見えるかどうか調べる
        // もし見えれば，それは関数定義．そうでなければグローバル変数，ということになる．
        int cur_pos = tokens->pos;
        Token *id;
        DeclarationSpecifier *declspec = decl_spec(tokens, &id);
        TokenKind decl_spec_end = current_tk(tokens);
        tokens->pos = cur_pos;

        if (decl_spec_end != TK_LPAREN)
        {
            // global-variable
            Decl *global_decl = declaration(tokens);
            char *copied_name = (char *)calloc(global_decl->id->length, sizeof(char));
            strncpy(copied_name, global_decl->id->str, global_decl->id->length);
            copied_name[global_decl->id->length] = 0;

            GlobalVariable *glob_var = (GlobalVariable *)calloc(1, sizeof(GlobalVariable));
            glob_var->is_static = declspec->is_static;
            glob_var->cty = global_decl->cty;
            map_put(translation_unit->global_variables, copied_name, glob_var);
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

    DeclarationSpecifier *decl = declaration_specifiers(tokens);

    Token *fn_id = declarator(&decl->cty, tokens);
    char *func_name = fn_id->str;
    size_t func_name_length = fn_id->length;

    Function *f = new_function(func_name, func_name_length);
    Map *local_variables = new_map();
    local_variables_in_cur_fn_g = local_variables;
    f->local_variables = local_variables;
    f->return_type = decl->cty;
    f->is_static = decl->is_static;

    Vector *params = parameter_list(tokens);

    Vector *stmts = compound_stmt(tokens);

    f->stmts = stmts;
    f->params = params;
    f->stack_size = total_stack_size_in_fn_g;
    return f;
}