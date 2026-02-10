#!/bin/bash
set -x

thisDir=$(pwd)
buildDir=$thisDir/build
mwDir=$(realpath $thisDir/../Microwalk)

mkdir -p $buildDir

# Generate MAP file for library
pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
dotnet MapFileGenerator.dll $thisDir/dist/lib/libpython3.13.so.1.0 $buildDir/libpython3.13.so.map
popd

gcc bytecode-target.c ../wrapper.c -DPYTHON_BIN=\"$thisDir/venv/bin/python\" -DENABLE_INSTR -D_GNU_SOURCE -g -fno-inline -fno-split-stack -L$thisDir/dist/lib -Wl,-rpath="$thisDir/dist/lib" -lpython3.13 -I$thisDir/dist/include/python3.13 -o $buildDir/bytecode-target

pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
dotnet MapFileGenerator.dll $buildDir/bytecode-target $buildDir/bytecode-target.map
popd
