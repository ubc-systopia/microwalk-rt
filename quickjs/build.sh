#!/bin/bash
set -x

thisDir=$(pwd)
buildDir=$thisDir/build
mwDir=$(realpath $thisDir/../Microwalk)

mkdir -p $buildDir

# Generate MAP file for library
pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
dotnet MapFileGenerator.dll "$thisDir/dist/lib/quickjs/libquickjs.so" "$buildDir/libquickjs.so.map"
popd

gcc bytecode-target.c wrapper.c -DENABLE_HANDLER_EXPORT -D_GNU_SOURCE -g -fno-inline -fno-split-stack -L$thisDir/dist/lib/quickjs -Wl,-rpath="$thisDir/dist/lib/quickjs" -lquickjs -I quickjs/dist/include/quickjs -o $buildDir/bytecode-target

pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
dotnet MapFileGenerator.dll $buildDir/bytecode-target $buildDir/bytecode-target.map
popd
