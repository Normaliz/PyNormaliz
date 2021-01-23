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

if [[ $OSTYPE == darwin* ]]; then ## sets paths etc. for Homebrew LLVM
    source install_scripts_opt/common.sh
fi

./install_pynormaliz.sh --user

