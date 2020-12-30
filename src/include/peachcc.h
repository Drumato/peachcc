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

int align_to(int n, int align);

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
    TK_PERCENT,   // `%`
    TK_AMPERSAND, // `&`
    TK_LPAREN,    // `(`
    TK_RPAREN,    // `)`
    TK_LBRACKET,  // `[`
    TK_RBRACKET,  // `]`
    TK_LBRACE,    // `{`
    TK_RBRACE,    // `}`
    TK_LE,        // `<`
    TK_GE,        // `>`
    TK_DOT,       // `.`
    TK_LEEQ,      // `<=`
    TK_GEEQ,      // `>=`
    TK_EQ,        // `==`
    TK_NTEQ,      // `!=`
    TK_INCREMENT, // `++`
    TK_DECREMENT, // `--`
    TK_LOGAND,    // `&&`
    TK_LOGOR,     // `||`
    TK_ARROW,     // `->`
    TK_ELLIPSIS,  // `...`

    TK_ASSIGN,          // `=`
    TK_COLON,           // `:`
    TK_BANG,            // `!`
    TK_QUESTION,        // `?`
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
    TK_EXTERN,          // "extern"
    TK_STRUCT,          // "struct"
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
    size_t line; // 行番号
};

typedef Vector TokenList;

// 整数トークンの作成
Token *new_integer_token(char *str, int value, size_t length, size_t line_num);
// 文字列トークンの作成
Token *new_string_token(char *str, size_t length, size_t line_num);
// 識別子トークンの作成
Token *new_identifier_token(char *str, size_t length, size_t line_num);
// 新しいトークンを作成する
Token *new_token(TokenKind kind, char *str, size_t length, size_t line_num);
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
    TY_STRUCT,
};
typedef enum CTypeKind CTypeKind;
typedef struct CType CType;

struct Member
{
    CType *cty;
    int offset;
};
typedef struct Member Member;

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

    // 構造体型に定義されたメンバ一覧
    Map *members;

    // アライメントの計算に利用
    int align;
};

CType *new_int(void);
CType *new_char(void);
CType *new_ptr(CType *base);
CType *new_array(CType *base, int array_len);
CType *new_struct(Map *members);

/// ast/expr.c
typedef enum
{
    EX_ADD,           // 加算
    EX_SUB,           // 減算
    EX_MUL,           // 乗算
    EX_DIV,           // 除算
    EX_MOD,           // 剰余算
    EX_LE,            // `lhs < rhs`
    EX_GE,            // `lhs > rhs`
    EX_LEEQ,          // `lhs <= rhs`
    EX_GEEQ,          // `lhs >= rhs`
    EX_EQ,            // `lhs == rhs`
    EX_NTEQ,          // `lhs != rhs`
    EX_LOGOR,         // `lhs || rhs`
    EX_LOGAND,        // `lhs && rhs`
    EX_UNARY_PLUS,    // 単項+
    EX_UNARY_MINUS,   // 単項-
    EX_UNARY_ADDR,    // 単項&
    EX_UNARY_DEREF,   // 単項*
    EX_INTEGER,       // 整数リテラル
    EX_STRING,        // 文字列リテラル
    EX_CALL,          // 呼び出し式
    EX_LOCAL_VAR,     // 識別子
    EX_ASSIGN,        // 代入式
    EX_CONDITION,     // 三項演算子
    EX_MEMBER_ACCESS, // メンバアクセス式
    // sizeof 演算子
    // analyze() によって変換されるので注意
    EX_UNARY_SIZEOF,
    EX_UNARY_NOT, // 否定演算子
} ExprKind;

typedef struct Expr Expr;

// 式を表すASTノードの型
typedef struct Variable Variable;
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
    Expr *cond;    // 三項演算子で使用

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
    // 行番号
    size_t line;
    // 変数のデータの格納
    // 意味解析時に格納しておくことで，コード生成時の計算を省略する
    Variable *var;
    // メンバ名
    char *copied_member;
    Member *member;
};

Expr *new_member_access(Expr *un_op, Token *member_name, char *str, size_t line_num);
Expr *new_conditional_expr(Expr *cond, Expr *lhs, Expr *rhs, char *str, size_t line_num);
Expr *new_unop(ExprKind op, Expr *child_expr, char *str, size_t line_num);
Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, char *str, size_t line_num);
Expr *new_integer_literal(int value, char *str, size_t line_num);
Expr *new_string_literal(char *contents, char *str, size_t line_num);
Expr *new_identifier(char *str, size_t length, size_t line_num);

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

typedef struct Scope Scope;
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

    char *loc;    // デバッグで使用
    size_t line;  // 行番号
    Scope *scope; // 変数スコープ
};

Stmt *new_stmt(StmtKind k, char *loc, size_t line_num);

/// ast/function.c
struct Function
{
    // 関数名
    // コピーされているので，そのまま出力可能
    char *copied_name;
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
    Scope *scope;
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
struct Variable
{
    char *str;
    size_t length;
    CType *cty;
    bool is_global;

    //  ローカル変数でのみ使用
    int stack_offset;
    // グローバル変数でのみ使用
    char *init_data;
    // グローバル変数でのみ使用
    bool is_static;
};
Variable *new_variable(char *str, size_t length, CType *cty, bool is_global);

// 変数のスコープを管理する構造体
struct Scope
{
    Scope *inner;   // より内側のスコープ
    Scope *outer;   // より外側のスコープ
    Map *variables; // 変数定義
    Map *tags;      // 構造体タグ
};
Scope *new_scope(Scope **parent);
Variable *find_var(Scope *sc, char *key, size_t length);
CType *find_tag(Scope *sc, char *tag, size_t length);

/// lexer.c
void tokenize(TokenList *tokens, char *p);

/// parser/toplevel.c
TranslationUnit *parse(TokenList *tokens);

struct Decl
{
    CType *cty;
    Token *id;
};
typedef struct Decl Decl;
struct DeclarationSpecifier
{
    bool is_static;
    bool is_extern;
    CType *cty;
};
typedef struct DeclarationSpecifier DeclarationSpecifier;

/// analyze.c

void analyze(TranslationUnit *translation_unit);

/// codegen.c
void codegen(FILE *output_file, TranslationUnit *translation_unit);

/// debug.c

// エラー箇所を報告する
void error_at(char *loc, size_t line_num, char *msg);

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
int str_id_g;
// パース時にスタックオフセットを決定するために使用
// 関数をパースする毎に，0に初期化する必要がある
size_t total_stack_size_in_fn_g;

// コンパイルオプションを扱う構造体．
// main関数でコマンドラインオプションのパースが実行され，適切な値が格納されている．
CompileOption *peachcc_opt_g;

Map *global_variables_g;