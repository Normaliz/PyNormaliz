#/usr/bin/env bash

set -e  # exit on error
set -x  # print command before execution

export NMZ_PREFIX=${PWD}/local
export NORMALIZ_LOCAL_DIR=${NMZ_PREFIX}

# don't pollute the PyNormaliz directory
cd /tmp

git clone --depth=1 https://github.com/Normaliz/Normaliz
cd Normaliz

export MAKEFLAGS="-j2"

# install dependencies

if [[ $OSTYPE == darwin* ]]; then ## sets paths etc. for Homebrew LLVM
    source install_scripts_opt/common.sh
    [ "$EANTIC" == "yes" ] && ./install_scripts_opt/install_nmz_mpfr.sh && echo "mpfr complete!"
    [ "$EANTIC" == "yes" ] && ./install_scripts_opt/install_nmz_flint.sh && echo "flint complete!"
fi

[ "$COCOALIB" == "yes" ] && ./install_scripts_opt/install_nmz_cocoa.sh && echo "cocoalib complete!"
[ "$FLINT" == "yes" ] && ./install_scripts_opt/install_nmz_flint.sh && echo "flint complete!"
[ "$EANTIC" == "yes" ] && ./install_scripts_opt/install_nmz_flint.sh && echo "flint complete!"
[ "$EANTIC" == "yes" ] && ./install_scripts_opt/install_nmz_arb.sh && echo "arb complete!"
[ "$EANTIC" == "yes" ] && ./install_scripts_opt/install_nmz_e-antic.sh && echo "e-antic complete!"
[ "$NAUTY" == "yes" ] && ./install_scripts_opt/install_nmz_nauty.sh && echo "nauty complete!"

# finally install Normaliz
./bootstrap.sh
export CPPFLAGS="${CPPFLAGS} -I${NMZ_PREFIX}/include"
export LDFLAGS="${LDFLAGS} -L${NMZ_PREFIX}/lib"
./configure --prefix=${NMZ_PREFIX}
make
echo "//////"
echo ${NMZ_PREFIX}
make install
cd ..
