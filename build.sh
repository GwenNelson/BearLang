#!/usr/bin/env bash
pushd vendor/gc-7.6.4
./configure
make all install
popd

pushd vendor/gmp-6.1.2
./configure
make
make check
make install
popd

pushd vendor/replxx
mkdir -p build
cd build
cmake ..
make
make install
popd

mkdir -p build
cd build
cmake ..
make

