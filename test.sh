#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 0
try 42 42
try 41 " 12 + 34 - 5 "
try 8 "3+2*3-1"
try 9 "19-(8+2)"
try 3 "(4+5)/3"
try 10 "-10+20"
try 10 "+20-10"
try 1 "2 >= 1"
try 0 "2 < 1"
try 1 "1 == 1"
try 0 "1 != 1"
try 5 "ab = 1; de = 2 + 3; if(ab > de)return ab; else return de;"
try 11 "ab = 1; while(ab <= 10) ab = ab + 1; return ab;"

echo OK