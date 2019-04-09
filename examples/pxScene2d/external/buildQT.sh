qtZipfile=qt-everywhere-src-5.12.0.tar.xz
qtZipFolder=qt-everywhere-src-5.12.0
qtZipUrl=https://download.qt.io/archive/qt/5.12/5.12.0/single/qt-everywhere-src-5.12.0.tar.xz
make_parallel=16

if [ -f $qtZipfile ]; then
   echo "File $qtZipfile exists, skip download .."
else
   echo "start download $qtZipfile"
   wget $qtZipUrl
fi

if [ -d $qtZipFolder ]; then
    echo "$qtZipFolder folder exists, skil unzip .."
else
    tar -xvf $qtZipfile
fi

cd $qtZipFolder
echo "current make parallel = $make_parallel"

./configure -developer-build -opensource -nomake examples -nomake tests
make "-j${make_parallel}"