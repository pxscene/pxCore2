px.import({ scene: 'px:scene.1.js' }).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var temp = scene.create({t:"scene", url:"supportfiles/object.js", parent:scene.root});
  temp.ready.then(function(s) {
    var data = [1 ,2, 3, 4];
    s.api = data;
  });
}).catch( function importFailed(err){
  console.error("Import failed for objectindex.js: " + err);
});
