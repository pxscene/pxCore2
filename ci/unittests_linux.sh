#!/bin/sh
sudo cp $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/* /var/www/.
sudo /etc/init.d/lighttpd stop
sudo rm -rf /etc/lighttpd/lighttpd.conf
sudo ln -s $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/lighttpd.conf /etc/lighttpd/lighttpd.conf
sudo /etc/init.d/lighttpd start
cd $TRAVIS_BUILD_DIR/tests/pxScene2d;
touch $TRAVIS_BUILD_DIR/logs/test_logs;
TESTLOGS=$TRAVIS_BUILD_DIR/logs/test_logs;
sudo ./pxscene2dtests.sh>$TESTLOGS 2>&1;
#testing
ls -lrt /var/www/
#testing
$TRAVIS_BUILD_DIR/ci/check_dump_cores.sh `pwd` pxscene2dtests $TESTLOGS
grep "FAILED TESTS" $TESTLOGS
retVal=$?
cd $TRAVIS_BUILD_DIR;
if [ "$retVal" -eq 0 ]
then
if [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
cat $TESTLOGS
fi
exit 1;
else
exit 0;
fi
