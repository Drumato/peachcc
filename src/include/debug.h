#pragma once

#include "ast/expr.h"
// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...);

// ASTを標準エラー出力にダンプする
// ASTの各ノードに適切なトークンが付与されているかどうかもチェックする．
void dump_ast(Expr *e, int indent);