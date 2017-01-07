#!/bin/bash
set -e
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp #**/*.hpp **/*.cc **/*.c
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
if [ "$1" = "test" ];then
    export GYP_DEFINES="other_cflags='-fprofile-arcs -ftest-coverage' other_lflags='-fprofile-arcs -ftest-coverage'"
fi

# rm -rf build
gyp --depth=.
xcodebuild -project emule-x.xcodeproj -configuration Default

if [ "$1" = "test" ];then
    cd ./build/Default/
    ./emule_x.test
    cd ../../
    rm -rf report
    mkdir -p report/html
    gcovr --html --html-details -o report/html/coverage.html -r . -v build/emule-x.build/Default/emule_x.build/
fi

if [ "$1" = "run" ];then
    cd ./build/Default/
    ./emule_x.test.console
    cd ../../
fi