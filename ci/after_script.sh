#!/bin/sh

checkError()
{
  if [ "$1" -ne 0 ]
  then
    printf "\n\n*********************************************************************";
    printf "\n********************* SCRIPT FAIL DETAILS *****************************";
    printf "\nCI failure reason: $2"
    printf "\nCause: $3"
    printf "\nReproduction/How to fix: $4"
    printf "\n*********************************************************************\n\n";
    exit 1
  fi
}

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ]
    then
      echo "Ignoring after script stage for $TRAVIS_EVENT_TYPE event";
      exit 0;
    fi
fi

cd $TRAVIS_BUILD_DIR
if [ "$TRAVIS_EVENT_TYPE" = "push" ] && [ -z "${TRAVIS_TAG}" ] 
then
  tar -cvzf logs.tgz logs/*
  checkError $? "Unable to compress logs folder" "Check for any previous tasks failed" "Retry"
  ./ci/deploy_files.sh 96.116.56.119 logs.tgz;
  checkError $? "Unable to send log files to 96.116.56.119" "Possible reason - Server could be down" "Retry"
fi

if [ "$TRAVIS_EVENT_TYPE" = "cron" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ]
then
  mkdir release
  checkError $? "unable to create release directory" "Could be permission issue?" "Retry"
  mv logs release/.
  checkError $? "unable to copy logs to release directory" "" "Retry"
  mv artifacts release/.
  checkError $? "unable to move artifacts folder to release directory" "artifacts directory created" "Retry"
  tar -cvzf release.tgz release/*
  checkError $? "unable to compress release folder" "release folder present?" "Retry"
  if [ "$TRAVIS_REPO_SLUG" = "pxscene/pxCore" ] && ( [ "$TRAVIS_BRANCH" = "master" ] || [ "$TRAVIS_BRANCH" = "$TRAVIS_TAG" ] );
  then
    ./ci/release_osx.sh 96.116.56.119 release.tgz 
    checkError $? "unable to send artifacts to 96.116.56.119" "96.116.56.119 down?" "Retry"
  fi
fi

if ( [ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "pull_request" ] ) && [ -z "${TRAVIS_TAG}" ] 
then
  ccache -s
fi

#update release  notes and info.plist in github
if ( [ "$TRAVIS_EVENT_TYPE" = "api" ] || [ ! -z "${TRAVIS_TAG}" ] ) && [ "$UPDATE_VERSION" = "true" ] 
then
   git checkout master
   checkError $? "unable to checkout master branch in pxscene" "" "check the credentials"
   export linenumber=`awk '/CFBundleShortVersionString/{ print NR; exit }' $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist`
   checkError $? "unable to find CFBundleShortVersionString entry in Info.plist" "" "check the repo"
   echo $linenumber
   sed -i '.bak' "`echo $((linenumber+1))`s/.*/       <string>$PX_VERSION<\\/string>/g" $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist
   checkError $? "unable to find versions entry in Info.plist" "" "check the repo"
   cp $TRAVIS_BUILD_DIR/RELEASE_NOTES $TRAVIS_BUILD_DIR/RELEASE_NOTES_bkp
   checkError $? "" "" ""
   echo "===============================================================\n\nRelease $PX_VERSION - `date +\"%d%b%Y\"`\n\n(github.com/johnrobinsn/pxCore: master - SHA `git log --oneline|head -n 1|awk '{print $1}'`)\n\nKnown Issues:\n\nCommits/Fixes:\n" > $TRAVIS_BUILD_DIR/RELEASE_NOTES 
   checkError $? "" "" ""
   git log  `git log --grep="Change version for release" --oneline -n 1|awk '{print $1}'`..HEAD --oneline --format=%s --no-merges >> $TRAVIS_BUILD_DIR/RELEASE_NOTES
   checkError $? "" "" ""
   echo "\n\n" >> $TRAVIS_BUILD_DIR/RELEASE_NOTES
   cat $TRAVIS_BUILD_DIR/RELEASE_NOTES_bkp >> $TRAVIS_BUILD_DIR/RELEASE_NOTES
   checkError $? "" "" ""
   rm -rf $TRAVIS_BUILD_DIR/RELEASE_NOTES_bkp
   checkError $? "" "" ""
   git add $TRAVIS_BUILD_DIR/examples/pxScene2d/src/macstuff/Info.plist
   git add $TRAVIS_BUILD_DIR/RELEASE_NOTES
   git commit -m "Change version for release $PX_VERSION [skip ci]"
   git push --repo="https://$REPO_USER_NAME:$GH_TOKEN@github.com/$REPO_USER_NAME/$REPO_NAME.git"
   checkError $? "unable to commit data to repo" "" "check the credentials"
fi
