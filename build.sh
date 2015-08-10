#!/bin/bash
ABS=`pwd`
EXT_LIB_PATH=${ABS}/examples/pxScene2d/external
NODE_PATH=${EXT_LIB_PATH}/node
LOG_PATH=/var/tmp/pxscene/logs
DEPLOY_PATH=deploy/examples/pxScene2d
BIN_SOURCE_PATH=examples/pxScene2d
SKIPBUILD=false
NODMG=false
DMGTEMPLATE=~/Desktop/Pxscene.template.dmg
DMGMP=/Volumes/Pxscene

function usage() {
  printf "Build script for pxscene binaries.\n"
  printf "Usage:  $(basename $0) options...\n" 
  printf "Options:\n"
  printf "  --skipbuild/-s\n"
  printf "      skips the compilation and building of binaries and only does packaging\n"
  printf "  --nodmg/-n\n"
  printf "      skips creation of the dmg.\n"
  printf "  -h, --help        display this help.\n"
  exit 0
}

function osCheck() {
  # check if we are on a Mac
  if [ `uname` != "Darwin" ]; then
    printf "Not MacOS\n"
  fi
}

parseOptions() {
  while [[ $# > 0 ]]
    do
      key="$1"

      case $key in
        -h|--help)
        usage
        ;;
        -e|--extension)
        EXTENSION="$2"
        shift # past argument need shift for each arg the option takes
        ;;
        -s|--skipbuild)
        SKIPBUILD=true
        ;;
        -n|--nodmg)
        NODMG=true
        ;;
        --default)
        DEFAULT=YES
        ;;
        *)
              # unknown option
        ;;
      esac
    shift # past argument or value
  done
}

function getNode()  {
  # Get node 0.12.7 - a dependency for pxscene
  cd /var/tmp
  wget --no-check-certificate https://nodejs.org/dist/v0.12.7/node-v0.12.7-darwin-x64.tar.gz
  gunzip node-v0.12.7-darwin-x64.tar.gz
  tar xf node-v0.12.7-darwin-x64.tar
  mv node-v0.12.7-darwin-x64 ${ABS}/examples/pxScene2d/external/.
  cd ${ABS}/examples/pxScene2d/external
  ln -s node-v0.12.7-darwin-x64 node
  rm /var/tmp/node-v0.12.7-darwin-x64.tar
  cd ../../..
  printf "done.\n"
}

buildExternalLibraries() {
  # build external libraries - jpg, ft, curl, etc
  printf "Setting up external libraries:\n"
  cd examples/pxScene2d/external
  for i in jpg png curl ft ; do
    TS=`date "+%Y-%m-%d %H:%M:%S"`
    printf "${TS} > Starting ${i}...\n" &> ${LOG_PATH}/${i}.out
    cd ${i}
    printf "   Setting up ${i}, see ${LOG_PATH}/${i}.out for details.\n"
    printf "\tConfiguring ${i}..."
    ./configure &> ${LOG_PATH}/${i}.out
    printf "done.\n"
    printf "\tMaking ${i}..."
    make clean >> ${LOG_PATH}/${i}.out 2>&1
    make >> ${LOG_PATH}/${i}.out 2>&1
    printf "done.\n"
    TS=`date "+%Y-%m-%d %H:%M:%S"`
    printf "${TS} > Finished ${i}.\n" &> ${LOG_PATH}/${i}.out
    cd ..
  done
  cd ../../..

  # build glut
  printf "   Setting up glut, see ${LOG_PATH}/glut.out for details.\n"
  printf "\tMaking glut..."
  make -f Makefile.glut clean &> ${LOG_PATH}/glut.out
  make -f Makefile.glut >> ${LOG_PATH}/glut.out 2>&1
  printf "done.\n"

  cd examples/pxScene2d/src/jsbindings
  printf "   Setting up with node-gyp, see ${LOG_PATH}/node-gyp.out for details.\n"
  printf "\tConfiguring..."
  node-gyp configure &> ${LOG_PATH}/node-gyp.out
  printf "done.\n"
  printf "\tBuilding..."
  ./build.sh >> ${LOG_PATH}/node-gyp.out 2>&1
  printf "done.\n"
}

copyBinaries() {
  cd ${ABS}
  # copy required binaries
  printf "Creating directories for Binaries and examples..."
  mkdir -p ${DEPLOY_PATH}/images
  mkdir -p ${DEPLOY_PATH}/external
  mkdir -p ${DEPLOY_PATH}/src/jsbindings/build/Debug
  printf "done.\n"
  printf "Copying files to package directory..."
  cp -R ${BIN_SOURCE_PATH}/images/* ${DEPLOY_PATH}/images/.
  cp -R ${BIN_SOURCE_PATH}/external/* ${DEPLOY_PATH}/external/.
  cp ${BIN_SOURCE_PATH}/src/jsbindings/*.js ${DEPLOY_PATH}/src/jsbindings/.
  cp ${BIN_SOURCE_PATH}/src/jsbindings/*.ttf ${DEPLOY_PATH}/src/jsbindings/.
  cp ${BIN_SOURCE_PATH}/src/jsbindings/*.sh ${DEPLOY_PATH}/src/jsbindings/.
  cp ${BIN_SOURCE_PATH}/src/jsbindings/build/Debug/px.node ${DEPLOY_PATH}/src/jsbindings/build/Debug/.
  printf "done.\n"
}

createDMG() {
#need to update with new DMG method and args to function to support edge - see /var/tmp/createdmg.sh
#template based code
#  printf "Creating DMG...\n"
#  hdiutil attach -mountpoint "${DMGMP}" "${DMGTEMPLATE}"
#  rm -rf ${DMGMP}/Pxscene.app/Contents/Resources/examples
#  mv -f deploy/examples ${DMGMP}/Pxscene.app/Contents/Resources/.
#  printf "done.\n"
#  cd deploy/MacOSX
#  printf "Creating dmg..."
#  DISKIMAGENAME=Pxscene
#  hdiutil detach "${DMGMP}"
#  hdiutil convert -format UDZO -ov -o "${DISKIMAGENAME}" "${DMGTEMPLATE}"
#  printf "done.\n"
#end template based code
  DMG_RES_DIR=deploy/MacOSX/dmgresources
  WINX=10	#opened DMG X position
  WINY=10	#opened DMG Y position
  WINW=470	#opened DMG width
  WINH=570	#opened DMG height
  ICON_SIZE=128
  VOLUME_NAME=Pxscene	#mounted DMG name
  VOLUME_ICON_FILE=${DMG_RES_DIR}/pxscenevolico.icns
  BACKGROUND_FILE=${DMG_RES_DIR}/background.png
  BACKGROUND_FILE_NAME="$(basename ${BACKGROUND_FILE})"
  BACKGROUND_CLAUSE="set background picture of opts to file \".background:${BACKGROUND_FILE_NAME}\""
  REPOSITION_HIDDEN_FILES_CLAUSE="set position of every item to {theBottomRightX + 100, 100}"
  POSITION_CLAUSE="${POSITION_CLAUSE}set position of item \"Pxscene.app\" to {240, 140}"
  APPLICATION_LINK=true
  APPLICATION_CLAUSE="set position of item \"Applications\" to {240, 390}"
  TEXT_SIZE=16

  DMG_FILE="Pxscene.dmg"
  DMG_DIRNAME="$(dirname "${DMG_FILE}")"
  DMG_DIR="$(cd "${DMG_DIRNAME}" > /dev/null; pwd)"
  DMG_NAME="$(basename "${DMG_FILE}")"
  DMG_TEMP_NAME="${DMG_DIR}/rw.${DMG_NAME}"
  SRC_FOLDER="$(cd "deploy/MacOSX/Pxscene" > /dev/null; pwd)"

  printf "Creating DMG...\n"
  test -z "${VOLUME_NAME}" && VOLUME_NAME="$(basename "${DMG_FILE}" .dmg)"

  if [ -f "${SRC_FOLDER}/.DS_Store" ]; then
    echo "Deleting any .DS_Store in source folder"
    rm "${SRC_FOLDER}/.DS_Store"
  fi

  # Create the image
  echo "Creating disk image..."
  test -f "${DMG_TEMP_NAME}" && rm -f "${DMG_TEMP_NAME}"
  ACTUAL_SIZE=`du -sm "${SRC_FOLDER}" | cut -f1`
  DISK_IMAGE_SIZE=$(expr ${ACTUAL_SIZE} + 20)
  hdiutil create -srcfolder "${SRC_FOLDER}" -volname "${VOLUME_NAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${DISK_IMAGE_SIZE}m "${DMG_TEMP_NAME}"

  # mount it
  echo "Mounting disk image..."
  MOUNT_DIR="/Volumes/${VOLUME_NAME}"
  echo "${MOUNT_DIR}"

  # try unmount dmg if it was mounted previously (e.g. developer mounted dmg, installed app and forgot to unmount it)
  echo "Unmounting disk image..."
  DEV_NAME=$(hdiutil info | egrep '^/dev/' | sed 1q | awk '{print $1}')
  echo "${DMG_TEMP_NAME}"
  test -d "${MOUNT_DIR}" && hdiutil detach "${DEV_NAME}"

  echo "Mount directory: ${MOUNT_DIR}"
  DEV_NAME=$(hdiutil attach -readwrite -noverify -noautoopen "${DMG_TEMP_NAME}" | egrep '^/dev/' | sed 1q | awk '{print $1}')
  echo "Device name:     ${DEV_NAME}"

  if ! test -z "${BACKGROUND_FILE}"; then
    echo "Copying background file..."
    test -d "${MOUNT_DIR}/.background" || mkdir "${MOUNT_DIR}/.background"
    cp "${BACKGROUND_FILE}" "${MOUNT_DIR}/.background/${BACKGROUND_FILE_NAME}"
  fi

  if ! test -z "${APPLICATION_LINK}"; then
    echo "making link to Applications dir"
    echo ${MOUNT_DIR}
    ln -s /Applications "${MOUNT_DIR}/Applications"
  fi

  if ! test -z "${VOLUME_ICON_FILE}"; then
    echo "Copying volume icon file '${VOLUME_ICON_FILE}'..."
    cp "${VOLUME_ICON_FILE}" "${MOUNT_DIR}/.VolumeIcon.icns"
    SetFile -c icnC "${MOUNT_DIR}/.VolumeIcon.icns"
  fi

  # run applescript
  APPLESCRIPT=$(mktemp -t createdmg)
  printf 'on run (volumeName)
	tell application "Finder"
		tell disk (volumeName as string)
			open
			
			set theXOrigin to WINX
			set theYOrigin to WINY
			set theWidth to WINW
			set theHeight to WINH
			
			set theBottomRightX to (theXOrigin + theWidth)
			set theBottomRightY to (theYOrigin + theHeight)
			set dsStore to \"\\\"\" & "/Volumes/" & volumeName & "/" & ".DS_STORE\\\""
			
			tell container window
				set current view to icon view
				set toolbar visible to false
				set statusbar visible to false
				set the bounds to {theXOrigin, theYOrigin, theBottomRightX, theBottomRightY}
				set statusbar visible to false
				REPOSITION_HIDDEN_FILES_CLAUSE
			end tell
			
			set opts to the icon view options of container window
			tell opts
				set icon size to ICON_SIZE
				set text size to TEXT_SIZE
				set arrangement to not arranged
			end tell
			BACKGROUND_CLAUSE
			
			-- Positioning
			POSITION_CLAUSE
			
			-- Hiding
			HIDING_CLAUSE
			
			-- Application Link Clause
			APPLICATION_CLAUSE
            close
            open
			
			update without registering applications
			-- Force saving of the size
			delay 1
			
			tell container window
				set statusbar visible to false
				set the bounds to {theXOrigin, theYOrigin, theBottomRightX - 10, theBottomRightY - 10}
			end tell
			
			update without registering applications
		end tell
		
		delay 1
		
		tell disk (volumeName as string)
			tell container window
				set statusbar visible to false
				set the bounds to {theXOrigin, theYOrigin, theBottomRightX, theBottomRightY}
			end tell
			
			update without registering applications
		end tell
		
		--give the finder some time to write the .DS_Store file
		delay 3
		
		set waitTime to 0
		set ejectMe to false
		repeat while ejectMe is false
			delay 1
			set waitTime to waitTime + 1
			
			if (do shell script "[ -f " & dsStore & " ]; echo $?") = "0" then set ejectMe to true
		end repeat
		log "waited " & waitTime & " seconds for .DS_STORE to be created."
	end tell
  end run' | sed -e "s/WINX/${WINX}/g" -e "s/WINY/${WINY}/g" -e "s/WINW/${WINW}/g" -e "s/WINH/${WINH}/g" -e "s/BACKGROUND_CLAUSE/${BACKGROUND_CLAUSE}/g" -e "s/REPOSITION_HIDDEN_FILES_CLAUSE/${REPOSITION_HIDDEN_FILES_CLAUSE}/g" -e "s/ICON_SIZE/${ICON_SIZE}/g" -e "s/TEXT_SIZE/${TEXT_SIZE}/g" | perl -pe  "s/POSITION_CLAUSE/${POSITION_CLAUSE}/g" | perl -pe "s/APPLICATION_CLAUSE/${APPLICATION_CLAUSE}/g" | perl -pe "s/HIDING_CLAUSE/${HIDING_CLAUSE}/" >"${APPLESCRIPT}"

  echo "Running Applescript: /usr/bin/osascript \"${APPLESCRIPT}\" \"${VOLUME_NAME}\""
  "/usr/bin/osascript" "${APPLESCRIPT}" "${VOLUME_NAME}" || true
  echo "Done running the applescript..."
  sleep 4

  rm "${APPLESCRIPT}"

  # make sure it's not world writeable
  echo "Fixing permissions..."
  chmod -Rf go-w "${MOUNT_DIR}" &> /dev/null || true
  echo "Done fixing permissions."

  # make the top window open itself on mount:
  echo "Blessing started"
  bless --folder "${MOUNT_DIR}" --openfolder "${MOUNT_DIR}"
  echo "Blessing finished"

  if ! test -z "${VOLUME_ICON_FILE}"; then
    # tell the volume that it has a special file attribute
    SetFile -a C "${MOUNT_DIR}"
  fi

  # unmount
  echo "Unmounting disk image..."
  echo ${DEV_NAME}
  sleep 60
  hdiutil detach "${DEV_NAME}"

  # compress image
  echo "Compressing disk image..."
  hdiutil convert "${DMG_TEMP_NAME}" -format UDZO -imagekey zlib-level=9 -o "${DMG_DIR}/${DMG_NAME}"
  rm -f "${DMG_TEMP_NAME}"
  echo "Disk image done"
}


osCheck
mkdir -p ${LOG_PATH}
parseOptions $*
if [ ! -d "${NODE_PATH}" ]; then
  printf "Node not found, getting node..."
  getNode
else
  printf "Node...found in ${NODE_PATH}\n"
fi
printf "Setting node in PATH..."
export PATH=${NODE_PATH}/bin:${NODE_PATH}/lib/node_modules/npm/bin/node-gyp-bin:${PATH}
printf "done.\n"
if [ "$SKIPBUILD" = false ]; then
  buildExternalLibraries
else
  printf "Skipping build of ft, jpg, png, curl, glut and jsbindings.\n"
fi
copyBinaries
if [ "${NODMG}" = false ]; then
  createDMG
fi
printf "$0 done.\n"
