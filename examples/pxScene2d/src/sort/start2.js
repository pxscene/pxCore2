
setInterval(function() {
  if (global.gc)
  {
 //   console.log("trying to run gc");
    global.gc();
  }
},1000);



var pxRoot2 = require('rcvrcore/pxRoot2')(0, 0, processArgs['screenWidth'], processArgs['screenHeight']);

pxRoot.addScene({url:processArgs['url'],w:processArgs['screenWidth'],h:processArgs['screenHeight'],
  useNodeBasedImports:processArgs['useNodeBasedImports']});
  
pxRoot.setOriginalUrl(processArgs['url']);
