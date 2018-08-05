px.import({scene:"px:scene.1.js",TextInput:'../TextInput.js'}).then( function ready(imports) {
  var scene = imports.scene;
  var root = scene.root;
  var TextInput = imports.TextInput;
}).catch(function importFailed(err){
    console.error("Import failed for test_import.js " + err);
});
