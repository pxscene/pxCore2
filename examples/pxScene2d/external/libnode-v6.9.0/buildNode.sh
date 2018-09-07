./configure --shared
make -j
ln -sf libnode.so.48 out/Release/obj.target/libnode.so
ln -sf libnode.48.dylib out/Release/libnode.dylib
