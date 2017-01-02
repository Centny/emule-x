#!/bin/bash
sleep 0.5
clang-format -style=file -i -sort-includes **/*.cpp **/*.h #**/*.hpp **/*.cc **/*.c
