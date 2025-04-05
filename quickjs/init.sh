#!/usr/bin/env bash

cd quickjs
PREFIX=$(realpath ../dist) make -j$(nproc) install
cd ..

patchelf --set-rpath '$ORIGIN/../lib/' dist/bin/qjs
patchelf --set-rpath '$ORIGIN/../lib/' dist/bin/qjsc
