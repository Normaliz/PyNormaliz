#/usr/bin/env bash

set -e  # exit on error
set -x  # print command before execution

cd /tmp/Normaliz/PyNormaliz
python3 tests/runtests.py
