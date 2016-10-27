./configure --shared
make -j8
ln -s libnode.so.48 out/Release/obj.target/libnode.so
