set -ev
cd $TRAVIS_BUILD_DIR/examples/pxScene2d/external
./build.sh 2&>install_logs
cd $TRAVIS_BUILD_DIR
