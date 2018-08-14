#!/bin/sh

travis_retry() {
  local result=0
  local count=1
  while [ $count -le 3 ]; do
    [ $result -ne 0 ] && {
      echo -e "\n$The command \"$@\" failed *******************. Retrying, $count of 3.\n" >&2
    }
    "$@"
    result=$?
    [ $result -eq 0 ] && break
    count=$(($count + 1))
    sleep 1
  done

  [ $count -gt 3 ] && {
    echo -e "\n$The command \"$@\" failed 3 times *******************.\n" >&2
  }

  return $result
}

#start the monitor
$TRAVIS_BUILD_DIR/ci/monitor.sh &

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ]
    then
      sudo apt-get install jq
      sudo apt-get install wget
      exit 0;
    fi
fi

#do the license check
if [ "$TRAVIS_OS_NAME" = "linux" ] ;
then
  $TRAVIS_BUILD_DIR/ci/licenseScanner.sh
  if [ "$?" != "0" ] 
  then
    printf "\n!*!*!* licenseScanner.sh detected files without proper license. Please refer to the logs above. !*!*!*\n"
    exit 1;
  fi
fi

#install necessary basic packages for linux and mac 
if [ "$TRAVIS_OS_NAME" = "linux" ] ; 
then 
  travis_retry sudo apt-get update
  travis_retry sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf valgrind libyaml-dev lcov cmake gdb quilt
fi

if [ "$TRAVIS_OS_NAME" = "osx" ] ;
then
  brew update;
  #brew upgrade cmake;
  brew install quilt
  sudo /usr/sbin/DevToolsSecurity --enable
  lldb --version
  lldb --help
  cmake --version
  man lldb
fi

#install lighttpd, code coverage binaries for mac
if [ "$TRAVIS_OS_NAME" = "osx" ] ; 
then
  if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ -z "${TRAVIS_TAG}" ]
  then
#    brew install lighttpd
    brew install gcovr
    brew install lcov
    brew install ccache
    ls -al $HOME/.ccache
  fi
fi

#setup lighttpd server
#if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]
#then
#  if [ "$TRAVIS_OS_NAME" = "linux" ] ;
#  then
#    sudo cp $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/* /var/www/.
#    sudo chmod -R 777 $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/
#    sudo chmod -R 777 /var/www
#    ls -lrt /etc/lighttpd/lighttpd.conf
#    sudo /etc/init.d/lighttpd stop
#    sudo sed -i "s/server.modules = (/server.modules = (\n\t\"mod_setenv\"\,/g" /etc/lighttpd/lighttpd.conf
#    echo "setenv.add-response-header += (\"Cache-Control\" => \"public, max-age=1000\")"|sudo tee -a /etc/lighttpd/lighttpd.conf
#    cat /etc/lighttpd/lighttpd.conf
#    sudo /etc/init.d/lighttpd start
#  elif [ "$TRAVIS_OS_NAME" = "osx" ] ;
#  then
#    brew services stop lighttpd
#    sudo mkdir -p /usr/local/var/www
#    sudo mkdir -p /var
#    sudo ln -s /usr/local/var/www /var/www
#    sudo cp $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/* /var/www/.
#    sudo chmod -R 777 $TRAVIS_BUILD_DIR/tests/pxScene2d/supportfiles/
#    sudo chmod -R 777 /var/www
#    sudo sed -i -n "s/server.port = 8080/server.port = 80/g" /usr/local/etc/lighttpd/lighttpd.conf
#    sudo sed -i -n "s/#  \"mod_setenv\"/   \"mod_setenv\"/g" /usr/local/etc/lighttpd/modules.conf
#    echo "setenv.add-response-header += (\"Cache-Control\" => \"public, max-age=1000\")"|sudo tee -a /usr/local/etc/lighttpd/modules.conf
#    echo "Displaying lighttpd file ***************************"
#    cat /usr/local/etc/lighttpd/lighttpd.conf
#    echo "Displaying modules.conf file ***************************"
#    cat /usr/local/etc/lighttpd/modules.conf
#    echo "Displaying modules.conf file completed ***************************"
#    sudo chmod -R 777 /usr/local
#    ls -lrt /usr/local/var/
#    sudo lighttpd -f /usr/local/etc/lighttpd/lighttpd.conf &
#    ps -aef|grep lighttpd
#    sudo netstat -a
#  fi
#fi


#install codecov
if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ -z "${TRAVIS_TAG}" ]
then
	if [ "$TRAVIS_OS_NAME" = "osx" ] ; 
	then
		git clone https://github.com/pypa/pip 
		sudo easy_install pip
	elif [ "$TRAVIS_OS_NAME" = "linux" ] ;
	then
		sudo apt-get install python-pip
	fi
	sudo pip install codecov
fi
