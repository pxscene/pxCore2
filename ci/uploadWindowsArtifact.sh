#!/bin/bash
counter=0
jobNum=0
jobNum=`echo "$(curl -sS --header "Content-type: application/json" "https://ci.appveyor.com//api/projects/pxscene/pxcore" | jq -r ".build.buildNumber")"` 
jobId=0dev.$jobNum
while [  $counter -lt 20 ]; do
  buildList=".builds[$counter].version"
  jobId=0dev.$jobNum
  let counter=counter+1
  let jobNum=jobNum-1 
  if [ "$jobId" != null ] ; then
    build="$(curl -sS --header "Content-type: application/json" "https://ci.appveyor.com/api/projects/pxscene/pxcore/build/"$jobId)" 
    if [[ "${build}" =  *"Build not found"* ]] ; then
      continue
    fi
    artifactStr=".build.jobs[0].artifactsCount"
    buildStr=".build.jobs[0].jobId"
    tagValStr=".build.isTag"
    artifactCounts=$(echo $build | jq -r  $artifactStr)
    buildVer=$(echo $build | jq -r  $buildStr)
    isTagTrue=$(echo $build | jq -r $tagValStr)

    if [[ $artifactCounts -gt 0 ]] && [[ "$isTagTrue" = "false" ]] ; then
      downloadArtifact="wget -q https://ci.appveyor.com/api/buildjobs/"$buildVer"/artifacts/spark-setup.exe"
      echo "Artifact count : $artifactCounts, Build version :  $buildVer, JobId : $jobId)"
      $downloadArtifact
      #DOWNLOAD_ARTIFACT="$(curl -sS --header "Content-type: application/json" "https://ci.appveyor.com/api/buildjobs/"$buildVer"/artifacts/spark-setup.exe")" 
      echo "::::wget -q https://ci.appveyor.com/api/buildjobs/"$buildVer"/artifacts/spark-setup.exe :::: " $downloadArtifact
      break;
    fi
  fi
done

filename="spark-setup.exe"
DEPLOY_USER="${DEPLOY_USER:-ubuntu}"
REMOTE_HOST="96.116.56.119"
REMOTE_DIR="/var/www/html/edge/windows/sparkEdge-setup.exe"

scp -P 2220 -o StrictHostKeyChecking=no ${filename} ${DEPLOY_USER}@${REMOTE_HOST}:${REMOTE_DIR}
