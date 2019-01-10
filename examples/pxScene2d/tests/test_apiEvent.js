"use strict";
px.import({scene:"px:scene.1.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;

root.w = 800;
root.h = 300;

var basePackageUri = px.getPackageBaseFilePath();


// Create a child scene that displays text on the screen
var newSceneChild = scene.create({t:'scene',parent:root,url:basePackageUri+"/test_apiEventChild.js"});
var theChildApi; 

// This is the 'callback' that gets registered for the text change event
var onTextChange = function(value) { 
  console.log("New value for child text is ",value);
}


// When child scene is ready, get api and register for event
newSceneChild.ready.then(function(child)  {

  console.log("newSceneChild is ready ....... "+child);
  console.log("child is api ....... "+child.api);
  console.log("newSceneChild is api ....... "+newSceneChild.api);

  // Register listener for text value change in child
  child.api.on("onTextChange",onTextChange);

  // Change value 
  child.api.setText("Something new on screen!");

});


}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_apiEvent.js failed: " + err)
});
