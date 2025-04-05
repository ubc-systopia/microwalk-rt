#!/bin/bash
set -x

thisDir=$(pwd)
buildDir=$thisDir/build
mwDir=$(realpath $thisDir/../Microwalk)

mkdir -p $buildDir

# dwarfdump dist/lib/libpython3.13.so.1.0 > $buildDir/libpython3.13.so.dwarf
# objdump -d -Mintel dist/lib/libpython3.13.so.1.0 > $buildDir/libpython3.13.so.dump

# Generate MAP file for library
pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
dotnet MapFileGenerator.dll $thisDir/dist/lib/libpython3.13.so.1.0 $buildDir/libpython3.13.so.map
popd

# Build targets
for target in $(find . -maxdepth 1 -name "target-*.c" -print)
do
  targetName=$(basename -- ${target%.*})

  gcc main.c $targetName.c -g -fno-inline -fno-split-stack -L$thisDir/dist/lib -Wl,-rpath="$thisDir/dist/lib" -lpython3.13 -I$thisDir/dist/include/python3.13 -I$mwDir -o $buildDir/$targetName

  pushd "$mwDir/Tools/MapFileGenerator/bin/Release/net8.0/"
  dotnet MapFileGenerator.dll $buildDir/$targetName $buildDir/$targetName.map
  popd

  # dwarfdump -l $buildDir/$targetName > $buildDir/$targetName.dwarf
  # objdump -d -Mintel $buildDir/$targetName > $buildDir/$targetName.dump
done
