#!/bin/bash
# clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp **/*.h

gyp --depth=. --build=Default

./ed2k/build.sh $1

