#!/bin/sh
# Format C/C++ files
# Don't format vendor files

find . -type d -name "vendor" -prune -o -iname *.c -print -o -iname *.h -print | xargs clang-format-15 --style=file:.clang-format -i
