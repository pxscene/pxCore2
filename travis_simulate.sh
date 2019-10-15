#!/bin/bash
#cleanup

if [ -e simulateTravis ]; then
  rm -rf simulateTravis
fi
if [ -e temp ]; then
  rm -rf temp
fi
if [ -e reports ]; then
  rm -rf reports
fi

#setup environment
export TRAVIS_BUILD_DIR=`pwd`
export TRAVIS_EVENT_TYPE="pull_request"
########### BEFORE INSTALL ############################################
if [ "$(uname)" = "Darwin" ]; then
    echo "Simulating for osx"
    export TRAVIS_OS_NAME="osx"
    brew install gcovr
    brew install lcov
elif [ "$(uname)" = "Linux" ]; then
    echo "Simulating for linux"
    export TRAVIS_OS_NAME="linux"
    sudo apt-get install lcov
fi

if [ "$TRAVIS_OS_NAME" = "linux" ] ;
then
  $TRAVIS_BUILD_DIR/ci/licenseScanner.sh
  if [ "$?" != "0" ]
  then
    printf "\n!*!*!* licenseScanner.sh detected files without proper license. Please refer to the logs above. !*!*!*\n"
    exit 1;
  fi
fi

#setup externals from remote repo
mkdir simulateTravis
cd simulateTravis
replacePath=`pwd`
git clone --branch=master https://github.com/pxscene/Spark-Externals.git
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  sed -i -n "s#/Users/travis/build/pxscene#$replacePath#g" Spark-Externals/artifacts/$TRAVIS_OS_NAME/lib/pkgconfig/*  
else
  sed -i "s#/home/travis/build/pxscene#$replacePath#g" Spark-Externals/artifacts/$TRAVIS_OS_NAME/lib/pkgconfig/*  
fi
cd Spark-Externals
ln -sf artifacts/$TRAVIS_OS_NAME extlibs
cd extlibs
mkdir lib_orig
cp -R lib/* lib_orig/.
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  rm -rf lib_orig/libgif.7.dylib
  rm -rf lib_orig/libpng.dylib
  rm -rf lib_orig/libsqlite3.dylib
  rm -rf lib_orig/libjpeg.dylib
fi
cd $TRAVIS_BUILD_DIR

########## INSTALL ############################################
mkdir -p $TRAVIS_BUILD_DIR/logs
touch $TRAVIS_BUILD_DIR/logs/build_logs
#build externals
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
./build_release.sh --release-externals-path $TRAVIS_BUILD_DIR/simulateTravis/Spark-Externals
cd $TRAVIS_BUILD_DIR

########## BEFORE SCRIPT ############################################
./ci/before_script.sh
cd $TRAVIS_BUILD_DIR

########## SCRIPT ############################################
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

checkError()
{
  if [ "$1" -ne 0 ]
  then
    printf "\n\n*********************************************************************";
    printf "\n********************* SCRIPT FAIL DETAILS *****************************";
    printf "\nCI failure reason: $2"
    printf "\nCause: $3"
    printf "\nReproduction/How to fix: $4"
    printf "\n*********************************************************************";
    printf "\n*********************************************************************\n\n";
    exit 1
  fi
}

export DUMP_STACK_ON_EXCEPTION=1
cd $TRAVIS_BUILD_DIR/ci
sh "build_$TRAVIS_OS_NAME.sh"
checkError $? "#### Build/unittests/execution [build_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"

sh "unittests_$TRAVIS_OS_NAME.sh" 300
checkError $? "#### Build/unittests/execution [unittests_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"

sh "execute_$TRAVIS_OS_NAME.sh" 3200
checkError $? "#### Build/unittests/execution [execute_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"
if [[ ! -z $ENABLE_CODE_COVERAGE ]] && [[ $ENABLE_CODE_COVERAGE -eq 1 ]]
then
  sh "code_coverage_$TRAVIS_OS_NAME.sh" "--gen-reports"
  checkError $? "#### Build/unittests/execution [code_coverage_$TRAVIS_OS_NAME.sh] failed" "Either build problem/execution problem" "Analyze corresponding log file"
fi

########## AFTER SCRIPT AND CLEANUP ############################################
cd $TRAVIS_BUILD_DIR/
rm -rf simulateTravis
rm -rf examples/pxScene2d/external
git checkout examples/pxScene2d/external
cd $TRAVIS_BUILD_DIR
echo "Travis build completed successfully !!"
