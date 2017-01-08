#!/bin/bash
set -e
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp #**/*.hpp **/*.cc **/*.c
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
if [ "$1" = "test" ];then
    export GYP_DEFINES="other_cflags='-fprofile-arcs -ftest-coverage' other_lflags='-fprofile-arcs -ftest-coverage'"
    rm -rf build
fi

# rm -rf build
gyp --depth=.
xcodebuild -project emule-x.xcodeproj -configuration Default

if [ "$1" = "test" ];then
    cd ./build/Default/
    if [ "$2" = "" ];then
        ./emule_x.test 2>err.log
    else
        ./emule_x.test --run_test=$2 2>err.log
    fi
    cd ../../
    rm -rf report
    mkdir -p report/html
    gcovr --html --html-details -o report/html/coverage.html -r . -v build/emule-x.build/Default/emule_x.build/ 1>c.log
fi

if [ "$1" = "run" ];then
    cd ./build/Default/
    ./emule_x.test.console
    cd ../../
fi

echo "All done..."