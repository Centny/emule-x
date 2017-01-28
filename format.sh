#!/bin/bash
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp #**/*.hpp **/*.cc **/*.c
