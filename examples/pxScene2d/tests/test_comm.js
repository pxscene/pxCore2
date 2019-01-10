/** 
 * This example demonstrates the mechanism by which a pxApp can communicate
 * with a server app via XREPluginResource.
 * 
 * When this app exports a function, e.g. "sampleMethod", that function
 * is then exposed via this pxApp's api property and can be invoked by the
 * server app.
 * 
 * When the  server sets an event, e.g., "onSampleEvent" the plugin then 
 * creates a method on this app's api object inside the plugin. When that 
 * method is called from this pxApp, the plugin tells the server and passes 
 * along any data.
 */


px.import("px:scene.1.js").then( function ready(scene){
var root = scene.root;
var basePackageUri = px.getPackageBaseFilePath();

var background = scene.create({t:"rect",parent:root,clip:false, fillColor:0xff0000ff,lineColor:0xffff0080,lineWidth:0,x:0,y:0,w:1280,h:720,a:1});

module.exports.sampleProperty = 1;
/** This exported function is exposed so that the server can call it
 * to execute this function in this app */
module.exports.sampleMethod = function(name,callback) {
  console.log("inside sampleMethod!");
  console.log("sampleProperty: " + module.exports.sampleProperty);
}

scene.root.on("onPreKeyDown", function(e) {
  if (module.exports.onSampleEvent !== undefined)
  {
    var testMe = {"name": "sample", "value":3}
    /** This is a call to the function added to this pxApp's api property
     * by the plugin when the server set an event called "onSampleEvent".
     * It is a way for the pxApp to send data or signal information to the
     * plugin and thus to the server app. For instance, a pxApp could 
     * signal that the Exit key had been pressed, thus notifying the server 
     * that the plugin should be torn down. 
      */
    module.exports.onSampleEvent(testMe);
  }
});



}).catch(function importFailed(e) {
  console.error("Import failed for pxscenetest.js: " + e)
});

