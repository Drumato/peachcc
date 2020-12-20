#!/bin/bash
make > /dev/null
try() {
  expected="$1"
  input="$2"

  ./peachcc "$input"
  gcc -static -o tmp asm.s
  ./tmp
  actual="$?"

  if [ "$actual" == "$expected" ]; then
    echo -e "$(cat $input) \e[32m=> $actual\e[0m"
  else
    echo "$input: $expected expected, but got $actual"
    make clean
    exit 1
  fi
}

try 42 'examples/just_integer.c'
try 21 'examples/simple_adsub.c'
try 19 'examples/with_whitespace.c'
try 2 'examples/four_arithmetic.c'
try 12 'examples/paren_expr.c'
try 19 'examples/unary_expr.c'
echo -e "\e[33mAll Test Passed.\e[0m"

make clean
