#!/bin/sh

function echo_success {
  echo -en "\\33[60G$1: [\\33[1;32m"
  echo -n "  OK  "
  echo -en "\\33[0;39m]\n"
}

function echo_failed {
  echo -en "\\33[67G[\\33[1;31m"
  echo -n "FAILED"
  echo -en "\\33[0;39m]\n"
}


function lbfind {
  if [ -x $1 ]; then
    echo -n $1\'x found
    echo_success " TRUE"
  else
    echo -n $1\'x NOT found
    echo_failed
    exit 1
  fi
}

function lbtrue {
  if $* > /dev/null; then
    echo -n $*
    echo_success " TRUE"
  else
    echo -n $*
    echo_failed
    rm -f $A_OUT
    exit 1
  fi
}

function lbfalse {
  if $* > /dev/null; then
    echo -n $*
    echo_failed
    rm -f $A_OUT
    exit 1
  else
    echo -n $*
    echo_success "FALSE"
  fi
}

if which lua.exe > /dev/null; then
  LBEXE=./l-bia.exe
  A_OUT=a.exe
  H_OUT=hello.exe
else
  LBEXE=./l-bia
  A_OUT=a
  H_OUT=hello
fi
LBLUA=./l-bia.lua
HELLO=./hello.lua

lbfind  $LBEXE
lbfind  $LBLUA
lbfind  $HELLO

lbtrue  $LBEXE -h
lbtrue  $LBLUA -h
lbtrue  $LBEXE -v
lbtrue  $LBLUA -v
lbtrue  $LBEXE --help
lbtrue  $LBLUA --help
lbtrue  $LBEXE --version
lbtrue  $LBLUA --version
lbtrue  $LBEXE
lbtrue  $LBLUA
lbtrue  $LBEXE $HELLO
lbfalse $LBLUA $HELLO
lbfalse $LBEXE --best
lbfalse $LBLUA -i $LBEXE --best
lbfalse $LBEXE $HELLO $HELLO
lbfalse $LBLUA -i $LBEXE $HELLO $HELLO
lbfalse $LBEXE -i $LBEXE -i $LBEXE $HELLO
lbfalse $LBLUA -i $LBEXE -i $LBEXE $HELLO
lbfalse $LBEXE -o $A_OUT -o $A_OUT $HELLO
lbfalse $LBLUA -o $A_OUT -o $A_OUT $HELLO
for i in -cf -cl -sf -sl -df -dl -rf -rl --best; do
  lbtrue  $LBEXE $i $HELLO
  lbtrue  $LBEXE -i $LBEXE $HELLO $i
  lbtrue  $LBEXE $HELLO -o $A_OUT $i
  lbtrue  $LBEXE -o $A_OUT $i -i $LBEXE $HELLO

  lbfalse $LBLUA $i $HELLO
  lbtrue  $LBLUA -i $LBEXE $HELLO $i
  lbfalse $LBLUA $HELLO -o $A_OUT $i
  lbtrue  $LBLUA -o $A_OUT $i -i $LBEXE $HELLO

  lbfalse $LBEXE $i
  lbfalse $LBLUA $i --best
done

rm -f $A_OUT $H_OUT
echo "Success! There may be other bugs; review your code!"
