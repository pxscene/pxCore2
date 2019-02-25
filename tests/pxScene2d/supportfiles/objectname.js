px.import({ scene: 'px:scene.1.js' }).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var temp = scene.create({t:"scene", url:"supportfiles/object.js", parent:scene.root});
  temp.ready.then(function(s) {
    s.api = {"param1" : 1, "param2" : 2};
    s.api.param3 = 3;
  });
}).catch( function importFailed(err){
  console.error("Import failed for objectname.js: " + err);
});
