#!/bin/bash
abs=`pwd`
logpath=/var/tmp/xre2log
SKIPBUILD=false
NODMG=false
DMGTEMPLATE=~/Desktop/Pxscene.template.dmg
DMGMP=/Volumes/Pxscene

# check if we are on a Mac
if [ `uname` != "Darwin" ]; then
  printf "Not MacOS\n"
  exit
fi

mkdir -p ${logpath}

while [[ $# > 0 ]]
  do
    key="$1"

    case $key in
      -e|--extension)
      EXTENSION="$2"
      shift # past argument
      ;;
      -s|--skipbuild)
      SKIPBUILD=true
      # shift # past argument
      ;;
      -n|--NODMG)
      NODMG=true
      # shift # past argument
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

# Get node 0.12.7 - a dependency for xre2
if [ ! -d "${abs}/examples/pxScene2d/external/node" ]; then
  printf "Node not found, getting node..."
  cd /var/tmp
  wget --no-check-certificate https://nodejs.org/dist/v0.12.7/node-v0.12.7-darwin-x64.tar.gz
  gunzip node-v0.12.7-darwin-x64.tar.gz
  tar xf node-v0.12.7-darwin-x64.tar
  mv node-v0.12.7-darwin-x64 ${abs}/examples/pxScene2d/external/.
  cd ${abs}/examples/pxScene2d/external
  ln -s node-v0.12.7-darwin-x64 node
  rm /var/tmp/node-v0.12.7-darwin-x64.tar
  cd ../../..
  printf "done.\n"
else
  printf "Node...found in ${abs}/examples/pxScene2d/external\n"
fi

export PATH=$abs/examples/pxScene2d/external/node/bin:$abs/examples/pxScene2d/external/node/lib/node_modules/npm/bin/node-gyp-bin:$PATH

# build external libraries - jpg, ft, curl, etc
if [ "$SKIPBUILD" = false ]; then
  printf "Setting up external libraries:\n"
  cd examples/pxScene2d/external
  for i in jpg png curl ft ; do
    cd $i
    printf "   Setting up $i, see $logpath/$i.out for details.\n"
    printf "\tConfiguring $i..."
    ./configure &> $logpath/$i.out
    printf "done.\n"
    printf "\tMaking $i..."
    make clean >> $logpath/$i.out 2>&1
    make >> $logpath/$i.out 2>&1
    printf "done.\n"
    cd ..
  done
  cd ../../..

  # build glut
  printf "   Setting up glut, see $logpath/glut.out for details.\n"
  printf "\tMaking glut..."
  make -f Makefile.glut clean &> $logpath/glut.out
  make -f Makefile.glut >> $logpath/glut.out 2>&1
  printf "done.\n"

  cd examples/pxScene2d/src/jsbindings
  printf "   Setting up with node-gyp, see $logpath/node-gyp.out for details.\n"
  printf "\tConfiguring..."
  node-gyp configure &> $logpath/node-gyp.out
  printf "done.\n"
  printf "\tBuilding..."
  ./build.sh >> $logpath/node-gyp.out 2>&1
  printf "done.\n"
else
  printf "Skipping build of ft, jpg, png, curl, glut and jsbindings.\n"
fi

cd $abs
# copy required binaries
printf "Creating directories for Binaries and examples..."
mkdir -p deploy/examples/pxScene2d/images
mkdir -p deploy/examples/pxScene2d/external
mkdir -p deploy/examples/pxScene2d/src/jsbindings/build/Debug
cp -R examples/pxScene2d/images/* deploy/examples/pxScene2d/images/.
cp -R examples/pxScene2d/external/* deploy/examples/pxScene2d/external/.
cp examples/pxScene2d/src/jsbindings/*.js deploy/examples/pxScene2d/src/jsbindings/.
cp examples/pxScene2d/src/jsbindings/*.ttf deploy/examples/pxScene2d/src/jsbindings/.
cp examples/pxScene2d/src/jsbindings/*.sh deploy/examples/pxScene2d/src/jsbindings/.
cp examples/pxScene2d/src/jsbindings/build/Debug/px.node deploy/examples/pxScene2d/src/jsbindings/build/Debug/.
printf "done.\n"
if [ "${NODMG}" = false ]; then
  printf "Creating DMG...\n"
  hdiutil attach -mountpoint "${DMGMP}" "${DMGTEMPLATE}"
  rm -rf ${DMGMP}/Pxscene.app/Contents/Resources/examples
  mv -f deploy/examples ${DMGMP}/Pxscene.app/Contents/Resources/.
  printf "done.\n"
  cd deploy/MacOSX
  printf "Creating dmg..."
  DISKIMAGENAME=Pxscene
  hdiutil detach "${DMGMP}"
  hdiutil convert -format UDZO -ov -o "${DISKIMAGENAME}" "${DMGTEMPLATE}"
  printf "done.\n"
fi
printf "$0 done.\n"
