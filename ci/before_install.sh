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

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]
    then
      echo "Ignoring before install stage for $TRAVIS_EVENT_TYPE event";
      exit 0;
    fi
fi

#install necessary basic packages for linux and mac 
if [ "$TRAVIS_OS_NAME" = "linux" ] ; 
then 
  travis_retry sudo apt-get update
  travis_retry sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf valgrind libyaml-dev lcov cmake x11proto-input-dev lighttpd gdb
  cd /usr/include/X11/extensions && sudo ln -s XI.h XInput.h
fi

if [ "$TRAVIS_OS_NAME" = "osx" ] ;
then
  brew update;
fi

#install code coverage binaries for mac
if [ "$TRAVIS_OS_NAME" = "osx" ] ; 
then
  if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]
  then
    brew install gcovr
    brew install lcov
    brew install --HEAD ccache
    ls -al $HOME/.ccache
  fi
fi

#install codecov
if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]
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
