// create and destroy objects every second - to test whether objects counts in the log are affected.
// After starting this app, monitor the logs at 
//      tail -f /private/var/tmp/pxscene.log
//
// logs should contain, the number of objects created and destroyed so far by this app, and the 
// number of pxObjects
//
// Example -
//
// So , below the pxObjects reported is 52, even though 49 objects were created and then destroyed
//
// 60 fps   pxObjects: 51
// 60 fps   pxObjects: 52
// 60 fps   pxObjects:----- Run Count = 46
// ----- Run Count = 47
// ----- Run Count = 48
// ----- Run Count = 49

// Jason Coelho

px.import("px:scene.1.js").then(function ready(scene) {

    var runCount = 0
    var delayed = function () {
        scene.create({
            t: 'image',
            url: 'http://localhost:8080/screensaver/images/cork.png', 
            w: 100, h: 100, parent: scene.root, x: 0, y: 0
        }).ready.then(function(image){
            console.log('removing image')
            image.removeAll()
            image.remove()
        })
        console.log('----- Run Count = ' + runCount++)

        setTimeout(delayed, 1000);
    }

    setTimeout(delayed, 1000);

}).catch( function importFailed(err){
  console.error("Import failed for fonts2_extras.js: " + err.stack)
});
