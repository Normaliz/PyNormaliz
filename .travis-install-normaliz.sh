#/usr/bin/env bash

set -e  # exit on error
set -x  # print command before execution

if [ "$EANTIC" == "yes" ]
then
    NMZ_ENF="--enable-flint --enable-enfnormaliz"

    # install arb
    wget https://github.com/fredrik-johansson/arb/archive/2.16.0.tar.gz
    tar xf 2.16.0.tar.gz
    cd arb-2.16.0/
    ./configure --prefix=/usr
    make
    sudo make install
    cd ..

    # install e-antic
    wget http://www.labri.fr/perso/vdelecro/e-antic/e-antic-0.1.3b0.tar.gz
    tar xf e-antic-0.1.3b0.tar.gz
    cd e-antic-0.1.3b0/
    ./configure --prefix=/usr
    make
    sudo make install
    cd ..

elif [ "$EANTIC" == "no" ]
then
    NMZ_ENF="--disable-flint --disable-enfnormaliz"
else
    exit 1
fi

# install nauty
NAUTY_VERSION="27rc2"
wget http://pallini.di.uniroma1.it/nauty${NAUTY_VERSION}.tar.gz
tar xvf nauty${NAUTY_VERSION}.tar.gz
cd nauty${NAUTY_VERSION}
./configure
make all CFLAGS=-fPIC
sudo mkdir -p /usr/local/include/nauty
sudo cp nauty.h /usr/local/include/nauty
sudo cp nauty.a /usr/local/lib/libnauty.a

# install normaliz
git clone --depth=1 https://github.com/Normaliz/Normaliz
cd Normaliz
./bootstrap.sh
./configure --disable-scip --disable-nmzintegrate $NMZ_ENF --prefix=/usr
make
sudo make install
cd ..

