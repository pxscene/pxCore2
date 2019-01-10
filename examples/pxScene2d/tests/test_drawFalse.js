module.exports.wantsClearscreen = function() { return false; };

px.import({scene: "px:scene.1.js",
          assert:"../test-run/assert.js",
          manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();


var tests = {

  test_textBoxDrawFalse: function() {
    return new Promise(function(resolve, reject) {

      var results = [];

      var textBox = scene.create({
          t:"textBox",                
          parent: scene.root,      
          x:100, y:100,
          w:300, y:20,
          text: "not yet resolved...",
          truncation: 1,
          ellipsis: true,
          draw: false,  // PROBLEM... SET THIS TO TRUE for resolution
          textColor:0xDDDDDDFF,   
          pixelSize: 16});        


      var testPromise = Promise.all([textBox.ready]);

      var timer = setTimeout(function() {
        results.push(assert(false,"TextBox with draw=false never got promise!"));
        resolve(results);
      }, 10000);
      testPromise.then(function() {
          console.log("resolved");
          textBox.text = 'Successfully resolved';
          textBox.draw = true;
          results.push(assert(true,"TextBox promise was resolved even though draw was false"));
      }, function failure() {
        // Failure is expected here, so this result is a pass
        results.push(assert(false,"TextBox got rejected promise."));
      }).then(function(error) {
        clearTimeout(timer);
        resolve(results);
      });

    });
  }
}
module.exports.tests = tests;


if(manualTest === true) {

  manual.runTestsManually(tests);

}
}).catch(function importFailed(err) {
    console.error("Imports failed: " + err);
});
