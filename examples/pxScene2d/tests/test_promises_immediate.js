"use strict";
px.import({scene:"px:scene.1.js"}).then( function ready(imports) {

  var scene = imports.scene;
  var root = imports.scene.root;

// Does not work, pressing a key does not print anything
        function doAsync() {
            return new Promise((res, rej) => {
              console.log("Success!");
                res("Success!");
            });
        }
        function call() {
            doAsync().then(result => {
                console.log('got result ' + result);
            });
        }
        scene.root.on("onKeyDown", key => {
            call();
        });

}).catch( function importFailed(err){
  console.error("Import failed for test_promises.js: " + err)
});