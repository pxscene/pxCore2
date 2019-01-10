px.import({scene:"px:scene.1.js"}).then( function ready(imports) {
  
  var scene = imports.scene;
  var root = imports.scene.root;
  var basePackageUri = px.getPackageBaseFilePath();
  var localfile = px.getFile("file:///opt/browser.js"); 
  localfile.then(function()  {
       console.log("local file download succeeded .....");
   }, function() {
       console.log("local file download failed .....");
   });

}).catch( function importFailed(err){
  console.log("err: "+err);
});
