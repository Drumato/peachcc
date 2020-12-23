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

assert 0 "int main(){ return 0; }"
assert 42 "int main(){ return 42; }"

assert 21 'int main(){ return 5+20-4; }'
assert 41 'int main(){ return  12 + 34 - 5 ; }'
assert 47 'int main(){ return 5+6*7; }'

assert 15 'int main(){ return 5*(9-6); }'
assert 4 'int main(){ return (3+5)/2; }'

assert 10 'int main(){ return -10+20; }'
assert 10 'int main(){ return - -10; }'
assert 10 'int main(){ return - - +10; }'

assert 0 'int main(){ return 0==1; }'
assert 1 'int main(){ return 42==42; }'
assert 1 'int main(){ return 0!=1; }'
assert 0 'int main(){ return 42!=42; }'

assert 1 'int main(){ return 0<1; }'
assert 0 'int main(){ return 1<1; }'
assert 0 'int main(){ return 2<1; }'
assert 1 'int main(){ return 0<=1; }'
assert 1 'int main(){ return 1<=1; }'
assert 0 'int main(){ return 2<=1; }'

assert 1 'int main(){ return 1>0; }'
assert 0 'int main(){ return 1>1; }'
assert 0 'int main(){ return 1>2; }'
assert 1 'int main(){ return 1>=0; }'
assert 1 'int main(){ return 1>=1; }'
assert 0 'int main(){ return 1>=2; }'

assert 3 'int main(){ 1; 2; return 3; }'

assert 3 'int main(){ int a; a=3; return a; }'
assert 8 'int main(){ int a; int z; a=3; z=5; return a+z; }'
assert 6 'int main(){ int a; int b; a=b=3; return a+b; }'

assert 3 'int main(){ int foo; foo=3; return foo; }'
assert 3 'int main(){ int FOO; FOO=3; return FOO; }'
assert 3 'int main(){ int _foo; _foo=3; return _foo; }'
assert 8 'int main() { int foo123; foo123=3; int bar; bar=5; return foo123+bar; }'

assert 3 'int main(){ if (0) return 2; return 3; }'
assert 3 'int main(){ if (1-1) return 2; return 3; }'
assert 2 'int main(){ if (1) return 2; return 3; }'
assert 2 'int main(){ if (2-1) return 2; return 3; }'
assert 4 'int main(){ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 'int main(){ if (1) { 1; 2; return 3; } else { return 4; } }'

assert 55 'int main(){ int i; int j; i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'int main(){ for (;;) {return 3;} return 5; }'

assert 10 'int main(){ int i; i=0; while(i<10) { i=i+1; } return i; }'

assert 3 'int main(){ return ret3(); } int ret3() { return 3; }'
assert 5 'int main(){ return ret5(); } int ret5() { return 5; }'

assert 8 'int main(){ return add(3, 5); } int add(int a, int b) { return a + b; }'
assert 2 'int main(){ return sub(5, 3); } int sub(int a, int b) { return a - b; }'
assert 21 'int main(){ return add6(1,2,3,4,5,6); } int add6(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }'
assert 66 'int main(){ return add6(1,2,add6(3,4,5,6,7,8),9,10,11); } int add6(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }'
assert 136 'int main(){ return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); } int add6(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }'

assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 55 'int main() { return rec(0); }  int rec(int n) { if(n == 10){return n;} else{ return n + rec(n+1); } }'
assert 144 'int main () { return fibonacci(12); } int fibonacci(int n) { if(n == 0) { return 0; } else if(n == 1) { return 1; } else { return fibonacci(n-1) + fibonacci(n-2); } }'

assert 3 'int main() { int x; x=3; return *&x; }'
assert 3 'int main() { int x; int y; int z; x=3; y=&x; z=&y; return **z; }'
assert 5 'int main() { int x; int y; x=3; y=5; return *(&x+8); }'
assert 3 'int main() { int x; int y; x=3; y=5; return *(&y-8); }'
assert 5 'int main() { int x; int y; x=3; y=&x; *y=5; return x; }'
assert 7 'int main() { int x; int y; x=3; y=5; *(&x+8)=7; return y; }'

assert 7 'int main() { int x; int y; x=3; y=5; *(&y-8)=7; return x; }'

echo -e "\e[33mAll Test Passed.\e[0m"

make clean
