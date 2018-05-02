
px.import({ scene: 'px:scene.1.js', keys:      'px:tools.keys.js',
             Optimus: 'optimus.js' }).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var optimus = imports.Optimus;
  var root  = imports.scene.root;

  //this is needed to prevent black background
  module.exports.wantsClearscreen = function()
  {
    return false;
  };

  scene.addServiceProvider(function(serviceName, serviceCtx){                   
    if (serviceName === ".hello")
    {
      return {helloMethod:function(v){console.log("Hello " + value);}};
    }
    return "allow";          
  });

  var SPARK_APP_OPTIMUS_ID = "1001";

  
  optimus.setScene(scene);

  console.log("available apps via optimus: " + optimus.getAvailableApplications());

  var sparkAppProperties = {
            id:SPARK_APP_OPTIMUS_ID, priority:1, x:0, y:0, w:scene.getWidth(), h:scene.getHeight(), cx:0, cy:0, sx:1.0, sy:1.0, r:0, a:1, interactive:true,
            painting:true, clip:false, mask:false, draw:true, launchParams:{ "cmd":"http://www.pxscene.org/examples/px-reference/gallery/fancy.js" },
        };

  optimus.createApplication(sparkAppProperties);

}).catch( function importFailed(err){
  console.error("Import failed for startup.js: " + err);
});
