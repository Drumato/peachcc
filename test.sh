#!/bin/bash
make > /dev/null

assert() {
  expected="$1"
  input="$2"

  # テストケースを書き込むファイルの作成
  tmp_c_file=$(mktemp)
  echo $input > $tmp_c_file

  # コンパイル & 実行
  # 終了ステータスをもらう
  ./peachcc -i $tmp_c_file -o asm.s
  gcc -static -o tmp asm.s
  ./tmp
  actual="$?"

  if [ "$actual" == "$expected" ]; then
    echo -e "'$input' \e[32m=> $actual\e[0m"
  else
    echo "$input: $expected expected, but got $actual"
    make clean
    exit 1
  fi
}

assert 0 "return 0;"
assert 42 "return 42;"
assert 21 'return 5+20-4;'
assert 41 'return  12 + 34 - 5 ;'
assert 47 'return 5+6*7;'
assert 15 'return 5*(9-6);'
assert 4 'return (3+5)/2;'
assert 10 'return -10+20;'
assert 10 'return - -10;'
assert 10 'return - - +10;'

assert 0 'return 0==1;'
assert 1 'return 42==42;'
assert 1 'return 0!=1;'
assert 0 'return 42!=42;'

assert 1 'return 0<1;'
assert 0 'return 1<1;'
assert 0 'return 2<1;'
assert 1 'return 0<=1;'
assert 1 'return 1<=1;'
assert 0 'return 2<=1;'

assert 1 'return 1>0;'
assert 0 'return 1>1;'
assert 0 'return 1>2;'
assert 1 'return 1>=0;'
assert 1 'return 1>=1;'
assert 0 'return 1>=2;'

assert 3 '1; 2; return 3;'

assert 3 'a=3; return a;'
assert 8 'a=3; z=5; return a+z;'
assert 6 'a=b=3; return a+b;'
assert 3 'foo=3; return foo;'
assert 3 'FOO=3; return FOO;'
assert 3 '_foo=3; return _foo;'
assert 8 'foo123=3; bar=5; return foo123+bar;'

echo -e "\e[33mAll Test Passed.\e[0m"

make clean
