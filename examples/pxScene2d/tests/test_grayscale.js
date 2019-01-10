px.import("px:scene.1.js").then(function ready(scene){
var root = scene.root;
var basePackageUri = px.getPackageBaseFilePath();

var url = basePackageUri+"/../../../images/graytree.jpg";
var imageRes = scene.create({t:"imageResource",url:url});
var grayImage = scene.create({t:"image",resource:imageRes, parent:root,clip:false});


}).catch(function importFailed(e){
  console.error("Import failed for grayscale.js: " + e)
});

