#!/bin/bash
set -x

thisDir=$(pwd)
buildDir=$thisDir/build
mwDir=$(realpath $thisDir/../Microwalk)

mkdir -p $buildDir

# dwarfdump dist/lib/quickjs/libquickjs.so > $buildDir/libquickjs.so.dwarf
# objdump -d -Mintel dist/lib/quickjs/libquickjs.so > $buildDir/libquickjs.so.dump

# Generate MAP file for library
pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
dotnet MapFileGenerator.dll "$thisDir/dist/lib/quickjs/libquickjs.so" "$buildDir/libquickjs.so.map"
popd

# Build targets
for target in $(find . -maxdepth 1 -name "target-*.c" -print)
do
  targetName=$(basename -- ${target%.*})

  gcc main.c $targetName.c -g -fno-inline -fno-split-stack -L$thisDir/dist/lib/quickjs -Wl,-rpath="$thisDir/dist/lib/quickjs" -lquickjs -I quickjs/dist/include/quickjs -I$mwDir -o $buildDir/$targetName

  pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
  dotnet MapFileGenerator.dll $buildDir/$targetName $buildDir/$targetName.map
  popd

  # dwarfdump -l $buildDir/$targetName > $buildDir/$targetName.dwarf
  # objdump -d -Mintel $buildDir/$targetName > $buildDir/$targetName.dump
done
