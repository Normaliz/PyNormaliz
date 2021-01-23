#/usr/bin/env bash

set -e  # exit on error
set -x  # print command before execution

cd /tmp/Normaliz/PyNormaliz
python2 tests/runtests.py
python3 tests/runtests.py
