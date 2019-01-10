// This test will use various uri schemes to load imported files and modules

/** Test using configImport:
 * local/relative jar load, 
 * test https jar load, 
 * test https js load, 
 * test local/relative js load
 */
px.configImport({"framework:":"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/",
                 "framework2:":"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/"}); 


px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"framework:test-run/tools_manualTests.js",
           scr_cap_utils:"framework:tests/screen_capture_utils.jar",
           local_jar:"framework2:../tests/jarForTest.jar"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;
var local_jar = imports.local_jar;
var utils = imports.scr_cap_utils;


var text = scene.create({t:'text',text:'My text value for child 2',parent:root,w:200,h:200, x:300, y:300});



module.exports.validateImports =  function() {
    var results = [];
    return new Promise( function(resolve, reject) {

      try {
        
        if( manual === undefined) 
          results.push(assert(false, "import for 'manual' using configImport failed"));
        else 
          results.push(assert(true, "import for 'manual' using configImport was successful"));

        if( utils === undefined) 
          results.push(assert(false, "import for 'scr_cap_utils' jar using configImport failed"));
        else 
          results.push(assert(true, "import for 'scr_cap_utils' jar using configImport was successful"));

        if( local_jar === undefined) 
          results.push(assert(false, "import for 'local_jar' jar using configImport failed"));
        else 
          results.push(assert(true, "import for 'local_jar' jar using configImport was successful"));

      }
      catch(err) {
        results.push(assert(false, "newSceneChild page load failed with exception"));
      }
      finally{
        resolve(results);
      }
    });
  }


 
  }).catch( function importFailed(err){
  console.error("Import for test_httpsImport_child2.js failed: " + err)
});
