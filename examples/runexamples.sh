#!/usr/bin/env sh
set -e
for a in *.py; do
    echo "#################### $a ####################"
    python $a
done
