#!/usr/bin/env bash

cd cpython
./configure --enable-shared --prefix=$(realpath ../dist)
make -j$(nproc) install
cd ..

patchelf --set-rpath '$ORIGIN/../lib/' dist/bin/python3.13
dist/bin/python3.13 -m venv venv
