#!/bin/bash
set -e

thisDir=$(pwd)
resultsDir=$thisDir/results
export WORK_DIR=$thisDir/work

mkdir -p $resultsDir
mkdir -p $WORK_DIR

reports=""

for target in $(find . -name "target-rsa-test.c" -print)
do
  targetName=$(basename -- ${target%.*})

  echo "Running target ${targetName}..."

  export TESTCASE_DIRECTORY=$thisDir/testcases/$targetName
  export TARGET_NAME=$targetName

  mkdir -p $WORK_DIR/$targetName/work
  mkdir -p $WORK_DIR/$targetName/persist

  cd ~/508/Microwalk/Microwalk/bin/Release/net8.0
  time dotnet Microwalk.dll $thisDir/config.yml -p ~/508/Microwalk/Microwalk.Plugins.PinTracer/bin/Release/net8.0

  cd $thisDir
  cp $WORK_DIR/$targetName/persist/results/call-stacks.txt $resultsDir/call-stacks-$targetName.txt

  echo "Running target ${targetName} successful"
done
