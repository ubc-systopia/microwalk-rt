#!/bin/bash
set -x

thisDir=$(pwd)
buildDir=$thisDir/build

mkdir -p $buildDir

#dwarfdump cpython/libpython3.13.so > $buildDir/libpython3.13.so.dwarf
objdump -d -Mintel cpython/libpython3.13.so > $buildDir/libpython3.13.so.dump

# Generate MAP file for library
pushd Microwalk/Tools/MapFileGenerator/bin/Release/net8.0/
dotnet MapFileGenerator.dll $thisDir/cpython/libpython3.13.so.1.0 $buildDir/libpython3.13.so.map
popd

# Build targets
for target in $(find . -maxdepth 1 -name "target-*.c" -print)
do
  targetName=$(basename -- ${target%.*})

  gcc main.c $targetName.c -g -fno-inline -fno-split-stack -L cpython -Wl,-rpath=$thisDir/cpython/ -lpython3.13 -I cpython -I cpython/Include -o $buildDir/$targetName

  pushd Microwalk/Tools/MapFileGenerator/bin/Release/net8.0/
  dotnet MapFileGenerator.dll $buildDir/$targetName $buildDir/$targetName.map
  popd

  #dwarfdump -l $buildDir/$targetName > $buildDir/$targetName.dwarf
  #objdump -d -Mintel $buildDir/$targetName > $buildDir/$targetName.dump
done
