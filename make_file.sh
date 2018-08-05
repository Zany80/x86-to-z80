#! /bin/bash

clang++-3.9 -std=c++1z -c -O3 -o- -Wall -Wextra -m32 -march=i386 -ggdb -S $1 > $1.x86.asm
cat $1.x86.asm | ~/Documents/Development/x86-to-z80/build/x86-to-z80 
#cat $1.z80.asm | sed -e "/^\t\..*$/d" > $1.asm

