#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "after script stage failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    exit 1
  fi
}

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]
    then
      exit 0;
    fi
fi

cd $TRAVIS_BUILD_DIR
tar -cvzf logs.tgz logs/*
checkError $?
./ci/deploy_files.sh 96.118.6.151 logs.tgz;
checkError $?

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] ;
then
  mkdir release
  checkError $?
  mv logs release/.
  checkError $?
  mv artifacts release/.
  checkError $?
  tar -cvzf release.tgz release/*
  checkError $?
  ./ci/release_osx.sh 96.118.6.151 release.tgz
  checkError $?
fi

if [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ;
then
  ccache -s
fi

#update release  notes and info.plist in github
if [ "$TRAVIS_EVENT_TYPE" = "api" ] && [ "$UPDATE_VERSION" = "true" ] ;
then
   git checkout master
   checkError $?
   export linenumber=`awk '/CFBundleShortVersionString/{ print NR; exit }' $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist`
   checkError $?
   echo $linenumber
   sed -i '.bak' "`echo $((linenumber+1))`s/.*/       <string>$PX_VERSION<\\/string>/g" $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist
   checkError $?
   cp $TRAVIS_BUILD_DIR/RELEASE_NOTES $TRAVIS_BUILD_DIR/RELEASE_NOTES_bkp
   checkError $?
   echo "===============================================================\n\nRelease $PX_VERSION - `date +\"%d%b%Y\"`\n\n(github.com/johnrobinsn/pxCore: master - SHA `git log --oneline|head -n 1|awk '{print $1}'`)\n\nKnown Issues:\n\nCommits/Fixes:\n" > $TRAVIS_BUILD_DIR/RELEASE_NOTES 
   checkError $?
   git log  `git log --grep="Change version for release" --oneline -n 1|awk '{print $1}'`..HEAD --oneline --format=%s --no-merges >> $TRAVIS_BUILD_DIR/RELEASE_NOTES
   checkError $?
   echo "\n\n" >> $TRAVIS_BUILD_DIR/RELEASE_NOTES
   cat $TRAVIS_BUILD_DIR/RELEASE_NOTES_bkp >> $TRAVIS_BUILD_DIR/RELEASE_NOTES
   checkError $?
   rm -rf $TRAVIS_BUILD_DIR/RELEASE_NOTES_bkp
   checkError $?
   git add $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist
   git add $TRAVIS_BUILD_DIR/RELEASE_NOTES
   git commit -m "Change version for release $PX_VERSION [skip ci]"
   git push --repo="https://$REPO_USER_NAME:$GH_TOKEN@github.com/$REPO_USER_NAME/$REPO_NAME.git"
   checkError $?
fi
