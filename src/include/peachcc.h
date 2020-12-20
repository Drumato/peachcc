#pragma once

#include "token.h"
// 入力されたCプログラムの中身
char *c_program_g;
// 現在のトークンを指す
// パーサ内部でしか用いられず，パーサ内部でfreeされる．
Token *cur_g;