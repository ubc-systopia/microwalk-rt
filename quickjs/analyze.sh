#!/bin/bash
set -x

thisDir=$(pwd)
export WORK_DIR="$thisDir/work"
export MICROWALK_DIR=$(realpath "$thisDir/../Microwalk")
#export PIN_ROOT=$(realpath "$thisDir/../../pin")

mkdir -p $WORK_DIR

for target in $(find . -maxdepth 1 -name "target-*.c" -print)
do
  targetName=$(basename -- ${target%.*})

  echo "Running target ${targetName}..."

  export TESTCASE_DIRECTORY=$(realpath "$thisDir/../testcases")
  export TARGET_NAME=$targetName

  mkdir -p $WORK_DIR/$targetName/work
  mkdir -p $WORK_DIR/$targetName/persist

  time dotnet "$MICROWALK_DIR/Microwalk/bin/Release/net8.0/Microwalk.dll" $thisDir/config.yml -p "$MICROWALK_DIR/Microwalk.Plugins.PinTracer/bin/Release/net8.0"

  echo "Running target ${targetName} successful"
done
