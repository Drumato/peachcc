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

assert 0 "main(){ return 0; }"
assert 42 "main(){ return 42; }"

assert 21 'main(){ return 5+20-4; }'
assert 41 'main(){ return  12 + 34 - 5 ; }'
assert 47 'main(){ return 5+6*7; }'

assert 15 'main(){ return 5*(9-6); }'
assert 4 'main(){ return (3+5)/2; }'

assert 10 'main(){ return -10+20; }'
assert 10 'main(){ return - -10; }'
assert 10 'main(){ return - - +10; }'

assert 0 'main(){ return 0==1; }'
assert 1 'main(){ return 42==42; }'
assert 1 'main(){ return 0!=1; }'
assert 0 'main(){ return 42!=42; }'

assert 1 'main(){ return 0<1; }'
assert 0 'main(){ return 1<1; }'
assert 0 'main(){ return 2<1; }'
assert 1 'main(){ return 0<=1; }'
assert 1 'main(){ return 1<=1; }'
assert 0 'main(){ return 2<=1; }'

assert 1 'main(){ return 1>0; }'
assert 0 'main(){ return 1>1; }'
assert 0 'main(){ return 1>2; }'
assert 1 'main(){ return 1>=0; }'
assert 1 'main(){ return 1>=1; }'
assert 0 'main(){ return 1>=2; }'

assert 3 'main(){ 1; 2; return 3; }'

assert 3 'main(){ a=3; return a; }'
assert 8 'main(){ a=3; z=5; return a+z; }'
assert 6 'main(){ a=b=3; return a+b; }'

assert 3 'main(){ foo=3; return foo; }'
assert 3 'main(){ FOO=3; return FOO; }'
assert 3 'main(){ _foo=3; return _foo; }'
assert 8 'main() { foo123=3; bar=5; return foo123+bar; }'

assert 3 'main(){ if (0) return 2; return 3; }'
assert 3 'main(){ if (1-1) return 2; return 3; }'
assert 2 'main(){ if (1) return 2; return 3; }'
assert 2 'main(){ if (2-1) return 2; return 3; }'
assert 4 'main(){ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 'main(){ if (1) { 1; 2; return 3; } else { return 4; } }'

assert 55 'main(){ i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'main(){ for (;;) {return 3;} return 5; }'

assert 10 'main(){ i=0; while(i<10) { i=i+1; } return i; }'

assert 3 'main(){ return ret3(); } ret3() { return 3; }'
assert 5 'main(){ return ret5(); } ret5() { return 5; }'

assert 8 'main(){ return add(3, 5); } add(a, b) { return a + b; }'
assert 2 'main(){ return sub(5, 3); } sub(a, b) { return a - b; }'
assert 21 'main(){ return add6(1,2,3,4,5,6); } add6(a, b, c, d, e, f) { return a+b+c+d+e+f; }'
assert 66 'main(){ return add6(1,2,add6(3,4,5,6,7,8),9,10,11); } add6(a, b, c, d, e, f) { return a+b+c+d+e+f; }'
assert 136 'main(){ return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); } add6(a, b, c, d, e, f) { return a+b+c+d+e+f; }'

assert 32 'main() { return ret32(); }  ret32() { return 32; }'
assert 55 'main() { return rec(0); }  rec(n) { if(n == 10){return n;} else{ return n + rec(n+1); } }'
assert 144 'main () { return fibonacci(12); } fibonacci(n) { if(n == 0) { return 0; } else if(n == 1) { return 1; } else { return fibonacci(n-1) + fibonacci(n-2); } }'

assert 3 'main() { x=3; return *&x; }'
assert 3 'main() { x=3; y=&x; z=&y; return **z; }'
assert 5 'main() { x=3; y=5; return *(&x+8); }'
assert 3 'main() { x=3; y=5; return *(&y-8); }'
assert 5 'main() { x=3; y=&x; *y=5; return x; }'
assert 7 'main() { x=3; y=5; *(&x+8)=7; return y; }'

assert 7 'main() { x=3; y=5; *(&y-8)=7; return x; }'

echo -e "\e[33mAll Test Passed.\e[0m"

make clean
