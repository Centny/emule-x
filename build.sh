#!/bin/bash
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp **/*.h


cd ed2k
    gyp --depth=. --build=Default
    if [ "$1" = "test" ];then
        ./build/Default/ed2k_test
    fi
cd ../
gyp --depth=. --build=Default

