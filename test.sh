#!/bin/bash
make > /dev/null
try() {
  expected="$1"
  input="$2"

  ./peachcc "$input" > tmp.s
  gcc -static -o tmp tmp.s
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
echo -e "\e[33mAll Test Passed.\e[0m"

make clean
