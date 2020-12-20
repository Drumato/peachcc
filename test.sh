#!/bin/bash
make > /dev/null

assert() {
  expected="$1"
  input="$2"

  tmp_c_file=$(mktemp)
  echo $input > tmp_c_file

  ./peachcc tmp_c_file
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

assert 0 0
assert 42 42
assert 21 '5+20-4'
assert 41 ' 12 + 34 - 5 '
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'
assert 10 '-10+20'
assert 10 '- -10'
assert 10 '- - +10'

assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'

assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'

assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'

echo -e "\e[33mAll Test Passed.\e[0m"

make clean
