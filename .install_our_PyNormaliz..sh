#/usr/bin/env bash

set -e  # exit on error
set -x  # print command before execution

export NMZ_PREFIX=${PWD}/local
export NORMALIZ_LOCAL_DIR=${NMZ_PREFIX}

mkdir -p /tmp/Normaliz/local
cp -r ${NMZ_PREFIX} /tmp/Normaliz

mkdir -p /tmp/Normaliz/PyNormaliz
cp -r * /tmp/Normaliz/PyNormaliz

# don't pollute the PyNormaliz directory
cd /tmp
cd Normaliz

echo "%%%%%%%%%"
ls

export MAKEFLAGS="-j2"

./install_pynormaliz.h --user


