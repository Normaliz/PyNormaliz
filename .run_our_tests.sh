#/usr/bin/env bash

set -e  # exit on error
set -x  # print command before execution

cd /tmp/Normaliz/PyNormaliz
python2 tests/run_tests.py
python3 tests/run_tests.py
