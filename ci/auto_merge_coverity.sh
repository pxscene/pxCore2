#!/bin/sh
# Automatically merge master to coverity_scan branch
checkError()
{
  if [ "$1" -ne 0 ]
  then
    echo "auto_merge_coverity failed with errors !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "failure reason: $2"
    echo "Cause: $3"
    echo "Reproduction/How to fix: $4"
    exit 1
  fi
}

echo Automatically merge master to coverity_scan branch

# Keep track of where Travis put us.
# We are on a detached head, and we need to be able to go back to it.
build_head=$(git rev-parse HEAD)

# Fetch all the remote branches. Travis clones with `--depth`, which
# implies `--single-branch`, so we need to overwrite remote.origin.fetch to
# do that.
git config --replace-all remote.origin.fetch +refs/heads/*:refs/remotes/origin/*
git fetch
# optionally, we can also fetch the tags
# git fetch --tags

git checkout master
git status
git pull
echo 'status for master after pull'
git status

git checkout coverity_scan
echo 'status for coverity_scan before pull'
git status
git pull
echo 'status for coverity_scan after pull'
git status

#git status
echo 'Doing merge'
git merge -q master
echo 'status for coverity_scan after merge'
git status 

echo Doing push
if [ "$TRAVIS_EVENT_TYPE" = "cron" ] ;
then
  echo using TRAVIS_REPO_SLUG
  git push --repo="https://$GH_TOKEN@github.com/$TRAVIS_REPO_SLUG.git"
else
  echo not cron job
  git push --repo="https://$GH_TOKEN@github.com/$REPO_USER_NAME/$REPO_NAME.git"
fi
checkError $? "unable to commit data to repo" "" "check the credentials"

echo Done with push
# finally, checkout the branch at HEAD to get our last commit
git checkout coverity_scan
