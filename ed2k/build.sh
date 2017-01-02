#!/bin/bash
clang-format -style=file -i -sort-includes *.cpp **/*.hpp
gyp --depth=. --build=Default
if [ "$1" = "test" ];then
    ./build/Default/ed2k_test
fi
