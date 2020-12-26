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
    char *output_file;
    char *input_file;
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
    TK_PLUS,      // `+`
    TK_MINUS,     // `-`
    TK_STAR,      // `*`
    TK_SLASH,     // `/`
    TK_AMPERSAND, // `&`
    TK_LPAREN,    // `(`
    TK_RPAREN,    // `)`
    TK_LBRACKET,  // `[`
    TK_RBRACKET,  // `]`
    TK_LBRACE,    // `{`
    TK_RBRACE,    // `}`
    TK_LE,        // `<`
    TK_GE,        // `>`
    TK_LEEQ,      // `<=`
    TK_GEEQ,      // `>=`
    TK_EQ,        // `==`
    TK_NTEQ,      // `!=`
    TK_INCREMENT, // `++`
    TK_DECREMENT, // `--`
    TK_LOGAND,    // `&&`
    TK_LOGOR,     // `||`

    TK_ASSIGN,          // `=`
    TK_COMMA,           // `,`
    TK_SEMICOLON,       // `;`
    TK_INTEGER_LITERAL, // 整数
    TK_STRING_LITERAL,  // 文字列
    TK_IDENTIFIER,      // 識別子
    TK_INT,             // "int"
    TK_CHAR,            // "char"
    TK_RETURN,          // "return"
    TK_IF,              // "if"
    TK_ELSE,            // "else"
    TK_FOR,             // "for"
    TK_WHILE,           // "while"
    TK_SIZEOF,          // "sizeof"
    TK_STATIC,          // "static"
    TK_EOF,             // 入力の終わり
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
    char *str;             // デバッグ用や，識別子用
    char *copied_contents; // 文字列リテラル用．
    size_t length;
};

typedef Vector TokenList;

// 整数トークンの作成
Token *new_integer_token(char *str, int value, size_t length);
// 文字列トークンの作成
Token *new_string_token(char *str, size_t length);
// 識別子トークンの作成
Token *new_identifier_token(char *str, size_t length);
// 新しいトークンを作成する
Token *new_token(TokenKind kind, char *str, size_t length);
// リスト中のposが指す現在の要素を見る
Token *current_token(TokenList *tokens);
// リスト中のposが指す現在のトークンの種類を見る
TokenKind current_tk(TokenList *tokens);
// トークンを読みすすめる
void progress(TokenList *tokens);

/// ctype.c
enum CTypeKind
{
    TY_INT,
    TY_CHAR,
    TY_PTR,
    TY_ARRAY,
};
typedef enum CTypeKind CTypeKind;
typedef struct CType CType;
struct CType
{
    CTypeKind kind;
    // 型が持つサイズ
    // C言語ではすべての型のサイズがコンパイル時に決定できる
    size_t size;
    // ポインタ型 or 配列型の場合に用いる
    // 配列型も暗黙的にポインタに変換されるため，同じメンバを使うことにする
    CType *base;
    // 配列の長さ
    // コンパイル時に決定できるはず
    int array_len;
};

CType *new_int(void);
CType *new_char(void);
CType *new_ptr(CType *base);
CType *new_array(CType *base, int array_len);

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
    EX_LOGOR,       // `lhs || rhs`
    EX_LOGAND,      // `lhs && rhs`
    EX_UNARY_PLUS,  // 単項+
    EX_UNARY_MINUS, // 単項-
    EX_UNARY_ADDR,  // 単項&
    EX_UNARY_DEREF, // 単項*
    EX_INTEGER,     // 整数リテラル
    EX_STRING,      // 文字列リテラル
    EX_CALL,        // 呼び出し式
    EX_LOCAL_VAR,   // 識別子
    EX_ASSIGN,      // 代入式
    // sizeof 演算子
    // analyze() によって変換されるので注意
    EX_UNARY_SIZEOF,
} ExprKind;

typedef struct Expr Expr;

// 式を表すASTノードの型
struct Expr
{
    char *str; // デバッグで使用
    // 変数名で使用
    // コピーされているのでそのまま出力可能
    char *copied_str;
    size_t length; // 変数名の長さ等
    ExprKind kind; // 式の型
    Expr *lhs;     // 左辺(2つのオペランドを取るノードで使用)
    Expr *rhs;     // 右辺(2つのオペランドを取るノードで使用)

    // 呼び出し式で使用
    // Expr *のリスト
    Vector *params;

    Expr *unary_op; // 単項演算で使用
    int value;      // kindがND_INTEGERの場合のみ使う
    // 式の型
    // analyze.cで型付けされる
    CType *cty;
    // 文字列リテラルで使用.
    int id;
};

Expr *new_unop(ExprKind op, Expr *child_expr, char *str);
Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, char *str);
Expr *new_integer_literal(int value, char *str);
Expr *new_string_literal(char *contents, char *str);
Expr *new_identifier(char *str, size_t length);

/// ast/stmt.c

enum StmtKind
{
    ST_EXPR,     // Expression statement
    ST_RETURN,   // return statement
    ST_IF,       // if statement
    ST_FOR,      // for statement
    ST_COMPOUND, // Compound statement
    ST_WHILE,    // while statement
};

typedef enum StmtKind StmtKind;
typedef struct Stmt Stmt;

/// 文を表す
struct Stmt
{
    StmtKind kind; // Statementの種類
    Expr *expr;    // ST_EXPR, ST_RETURNで使用
    Expr *cond;    // 条件式．ST_WHILE, ST_IF, ST_FORで使用
    Expr *init;    // 初期化式．ST_FORで使用
    Expr *inc;     // ST_FORで使用
    Vector *body;  // ST_COMPOUND等で使用

    Stmt *then; // ST_IF, ST_WHILE, ST_FOR等で使用
    Stmt *els;  // ST_IF等で使用

    char *loc; // デバッグで使用
};

Stmt *new_stmt(StmtKind k, char *loc);

/// ast/function.c
struct Function
{
    // 関数名
    // コピーされているので，そのまま出力可能
    char *copied_name;
    // ローカル変数の辞書
    Map *local_variables;
    Vector *stmts;
    // 関数が持つフレームサイズ
    size_t stack_size;
    // 引数リスト
    // 現在はただ引数名を持っているに過ぎない
    // 後々型名とかを持つようになるはず
    Vector *params;
    // 返り値の型
    CType *return_type;
    bool is_static;
};
typedef struct Function Function;

Function *new_function(char *name, size_t length);

/// ast/root.c

struct TranslationUnit
{
    // 関数定義群
    Vector *functions;
    // グローバル変数の定義
    Map *global_variables;
};
typedef struct TranslationUnit TranslationUnit;
TranslationUnit *new_translation_unit(void);

/// variable.c
struct LocalVariable
{
    char *str;
    size_t length;
    CType *cty;
    size_t stack_offset;
    // コンパイラ内で使用していないので，サポートしない
    // bool is_static;
};
typedef struct LocalVariable LocalVariable;

struct GlobalVariable
{
    CType *cty;
    char *init_data;
    bool is_static;
};
typedef struct GlobalVariable GlobalVariable;

LocalVariable *new_local_var(char *str, size_t length, CType *cty, size_t stack_offset);

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
char *expect_string_literal(TokenList *tokens);

// 現在見ているトークンが渡されたkと同じ種類かチェック
bool eatable(TokenList *tokens, TokenKind k);

bool at_eof(TokenList *tokens);
bool is_typename(TokenList *tokens);
bool start_storage_class(TokenList *tokens);

Token *try_eat_identifier(TokenList *tokens);

void insert_localvar_to_fn_env(Token *id, CType *cty);
Token *expect_identifier(TokenList *tokens);

/// parser/toplevel.c
TranslationUnit *parse(TokenList *tokens);

/// parser/statement.c

Vector *compound_stmt(TokenList *tokens);
Stmt *statement(TokenList *tokens);

/// parser/declaration.c

struct Decl
{
    CType *cty;
    Token *id;
};
typedef struct Decl Decl;
struct DeclarationSpecifier
{
    bool is_static;
    CType *cty;
};
typedef struct DeclarationSpecifier DeclarationSpecifier;

Decl *declaration(TokenList *tokens);
DeclarationSpecifier *declaration_specifiers(TokenList *tokens);
Token *declarator(CType **cty, TokenList *tokens);
Vector *parameter_list(TokenList *tokens);
DeclarationSpecifier *decl_spec(TokenList *tokens, Token **id);

/// parser/expression.c

Expr *expression(TokenList *tokens);

/// analyze.c

void analyze(TranslationUnit *translation_unit);

/// codegen.c
void codegen(FILE *output_file, TranslationUnit *translation_unit);

/// debug.c

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...);

// ASTを標準エラー出力にダンプする
void dump_ast(TranslationUnit *translation_unit);
// CTypeを標準エラー出力にダンプする
void dump_ctype(CType *cty);

// 入力されたCプログラムの中身
char *c_program_g;
// 現在のトークンを指す
// パーサ内部でしか用いられず，最終的にfreeする．
Token *cur_g;
// パーサで用いる
Map *local_variables_in_cur_fn_g;
Map *global_variables_g;
int str_id_g;
// パース時にスタックオフセットを決定するために使用
// 関数をパースする毎に，0に初期化する必要がある
size_t total_stack_size_in_fn_g;

// コンパイルオプションを扱う構造体．
// main関数でコマンドラインオプションのパースが実行され，適切な値が格納されている．
CompileOption *peachcc_opt_g;
