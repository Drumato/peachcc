#pragma once
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct CompileOption
{
    const char *output_file;
    const char *input_file;
    bool debug;
};
typedef struct CompileOption CompileOption;

/// vector.c

typedef struct
{
    void **data;
    int capacity;
    int len;
    int pos;
} Vector;

Vector *new_vec(void);
void vec_push(Vector *v, void *elem);
void vec_pushi(Vector *v, int val);
void *vec_pop(Vector *v);
void *vec_last(Vector *v);
bool vec_contains(Vector *v, void *elem);
bool vec_union1(Vector *v, void *elem);

/// map.c

struct Map
{
    Vector *keys;
    Vector *vals;
};
typedef struct Map Map;

Map *new_map(void);
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key, size_t length);
// bool map_exists(Map *map, char *key, size_t length);

/// token.c

// Tokenの種類
enum TokenKind
{
    TK_PLUS,       // `+`
    TK_MINUS,      // `-`
    TK_STAR,       // `*`
    TK_SLASH,      // `/`
    TK_LPAREN,     // `(`
    TK_RPAREN,     // `)`
    TK_LE,         // `<`
    TK_GE,         // `>`
    TK_LEEQ,       // `<=`
    TK_GEEQ,       // `>=`
    TK_EQ,         // `==`
    TK_NTEQ,       // `!=`
    TK_ASSIGN,     // `=`
    TK_SEMICOLON,  // `;`
    TK_INTEGER,    // 整数
    TK_IDENTIFIER, // 識別子
    TK_EOF,        // 入力の終わり
};
typedef enum TokenKind TokenKind;

typedef struct Token Token;

// プログラムの最小単位であるトークンの定義
struct Token
{
    TokenKind kind; // 種類
    int value;      // 整数ノードのみ使用する意味値
    /** メモリ上のソースコードを指すポインタ．
     * 各Tokenがこの文字列を所有しているわけではないので注意．
     **/
    char *str; // デバッグ用や，識別子用
    size_t length;
};

typedef Vector TokenList;

// 整数トークンの作成
Token *new_integer_token(char *str, int value, size_t length);
// 識別子トークンの作成
Token *new_identifier_token(char *str, size_t length);
// 新しいトークンを作成する
Token *new_token(TokenKind kind, char *str, size_t length);
// スタックにトークンをプッシュする
void push_token(TokenList *tokens, Token *tok);
// リスト中のposが指す現在の要素を見る
Token *current_token(TokenList *tokens);
// リスト中のposが指す現在のトークンの種類を見る
TokenKind current_tk(TokenList *tokens);
// トークンを読みすすめる
void progress(TokenList *tokens);

/// ast/expr.c
typedef enum
{
    EX_ADD,         // 加算
    EX_SUB,         // 減算
    EX_MUL,         // 乗算
    EX_DIV,         // 除算
    EX_LE,          // `lhs < rhs`
    EX_GE,          // `lhs > rhs`
    EX_LEEQ,        // `lhs <= rhs`
    EX_GEEQ,        // `lhs >= rhs`
    EX_EQ,          // `lhs == rhs`
    EX_NTEQ,        // `lhs != rhs`
    EX_UNARY_PLUS,  // 単項+
    EX_UNARY_MINUS, // 単項-
    EX_INTEGER,     // 整数リテラル
    EX_LOCAL_VAR,   // 識別子
    EX_ASSIGN,      // 代入式
} ExprKind;

typedef struct Expr Expr;

// 式を表すASTノードの型
struct Expr
{
    char *str;     // 変数名やデバッグで使用
    size_t length; // 変数名の長さ等
    ExprKind kind; // 式の型
    Expr *lhs;     // 左辺(2つのオペランドを取るノードで使用)
    Expr *rhs;     // 右辺(2つのオペランドを取るノードで使用)

    Expr *unary_op; // 単項演算で使用
    int value;      // kindがND_INTEGERの場合のみ使う
};

Expr *new_unop(ExprKind op, Expr *child_expr, char *str);
Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, char *str);
Expr *new_integer(int value, char *str);
Expr *new_identifier(char *str, size_t length);

/// ast/stmt.c

enum StmtKind
{
    ST_EXPR, // Expression statement.
};

typedef enum StmtKind StmtKind;
typedef struct Stmt Stmt;

/// 文を表す
struct Stmt
{
    StmtKind kind; // Statementの種類
    Expr *expr;    // ST_EXPR等で使用

    char *loc; // デバッグで使用
};

Stmt *new_exprstmt(Expr *expr, char *loc);

/// ast/root.c

struct Program
{
    Vector *stmts;
};
typedef struct Program Program;
Program *new_program(void);

/// variable.c
struct LocalVariable
{
    char *str;
    size_t length;
    size_t stack_offset;
};
typedef struct LocalVariable LocalVariable;

LocalVariable *new_local_var(char *str, size_t length, size_t stack_offset);

/// lexer.c
void tokenize(TokenList *tokens, char *p);

/// parser/common.c

// 次のトークンが期待している種類のときには，
// トークンを1つ読み進めて真を返す．
// 読み進められなかった時は偽を返す．
bool try_eat(TokenList *tokens, TokenKind k);

// 次のトークンが期待している記号のときには，トークンを1つ読み進める．
// それ以外の場合にはエラーを報告する．
void expect(TokenList *tokens, TokenKind k);

// 次のトークンが整数の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
int expect_integer_literal(TokenList *tokens);

// 現在見ているトークンが渡されたkと同じ種類かチェック
bool eatable(TokenList *tokens, TokenKind k);

bool at_eof(TokenList *tokens);

void push_statement(Vector *stmts, Stmt *s);

Token *try_eat_identifier(TokenList *tokens);

/// parser/toplevel.c
Program *parse(TokenList *tokens);

/// parser/statement.c

// expr_stmt
Stmt *statement(TokenList *tokens);

/// parser/expression.c

Expr *expr(TokenList *tokens);

/// codegen.c
void codegen(FILE *output_file, Program *program);

/// debug.c

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...);

// ASTを標準エラー出力にダンプする
// ASTの各ノードに適切なトークンが付与されているかどうかもチェックする．
void dump_ast(Program *program);

// 入力されたCプログラムの中身
char *c_program_g;
// 現在のトークンを指す
// パーサ内部でしか用いられず，最終的にfreeする．
Token *cur_g;
// ローカル変数を保持するマップ
// 関数実装時に削除する
Map *local_variables_g;
// パース時にスタックオフセットを決定するために使用
// 関数をパースする毎に，0に初期化する必要がある
size_t total_stack_size_in_fn_g;

// コンパイルオプションを扱う構造体．
// main関数でコマンドラインオプションのパースが実行され，適切な値が格納されている．
CompileOption *peachcc_opt_g;
