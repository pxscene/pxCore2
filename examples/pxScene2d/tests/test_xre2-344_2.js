px.import("px:scene.1.js").then( function ready(scene) {

var clearscreen = px.appQueryParams.clear;
if(clearscreen === undefined || clearscreen == 1) 
  clearscreen = true;
else
  clearscreen = false;
  
console.log("clearscreen is "+clearscreen);

module.exports.wantsClearscreen = function()
{
  return clearscreen;
};

var root = scene.root;

var basePackageUri = ".";
var container = scene.create({t:"object",parent:root});
var msgAlerts   = scene.create({t:"text",parent:container,pixelSize:24,x:700,y:10,h:30, w:50});
msgAlerts.text = "First image";

setTimeout(function() 
{ 
 container.draw  = false;
}, 5000);

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-344.js: " + err)
});
