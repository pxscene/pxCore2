src=../examples/pxScene2d/src
bundleRes=$src/$1
mkdir -p $bundleRes
cp -a $src/browser $bundleRes
cp $src/FreeSans.ttf $bundleRes
cp $src/package.json $bundleRes
cp -r $src/rcvrcore_duktape $bundleRes/rcvrcore
for mod_name in {init,shell,bluebird,fs,http,https,path,punycode,querystring,url,vm} ; do
    cp $src/duktape/$mod_name.js $bundleRes/$mod_name.js
done

cp $src/browser.js $bundleRes/browser.js
cp $src/about.js $bundleRes/about.js
cp $src/browser/editbox.js $bundleRes/browser/editbox.js
cp -a $src/node_modules $bundleRes/node_modules

echo 'copy resouce to debug done..';
