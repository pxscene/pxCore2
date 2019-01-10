
// Import frameworks
var frameworks = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/frameworks/";

px.configImport({"frameworks:":frameworks});

px.import({scene:"px:scene.1.js", check:'frameworks:checks.js'}).then( function ready(imports)
{
  var scene  = imports.scene;
  var root   = scene.root;
  var checks = imports.check;

  var ans = scene.create({ t: "text", parent: root,  text: "Support for 'path' type ... ", x: 120, y: 50, textColor:0xEEEEEEff, pixelSize: 32});

  var myPath;

  // Use frameworks checks checkType function to check if pxscene has support for path
  if( checks.checkType("path")){
    myPath = scene.create({ t: "path", d:"M100 100 v-50", strokeColor: 0xFF00FFff, strokeWidth: 2, parent: root});

    ans.text =  "Support for 'path' type - TRUE";
    console.log("Support for 'path' type - TRUE");

  } else {
    ans.text =  "Support for 'path' type - FALSE";
    console.log("Support for 'path' type - FALSE");
  }
 
}).catch(function importFailed(e){
  console.error("Import failed for test_xre2-1150.js: " + e)
});
