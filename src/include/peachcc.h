#pragma once
#include <stdbool.h>

#include "token.h"
// 入力されたCプログラムの中身
char *c_program_g;
// 現在のトークンを指す
// パーサ内部でしか用いられず，最終的にfreeする．
Token *cur_g;

struct CompileOption
{
    const char *output_file;
    const char *input_file;
    bool debug;
};
typedef struct CompileOption CompileOption;

// コンパイルオプションを扱う構造体．
// main関数でコマンドラインオプションのパースが実行され，適切な値が格納されている．
CompileOption *peachcc_opt_g;