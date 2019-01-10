px.import("px:scene.1.js").then( function ready(scene) {
  var root = scene.root;
  var basePackageUri = px.getPackageBaseFilePath();
  
  
  var fonts = [
              "http://www.pxscene.org/examples/px-reference/fonts/Pacifico.ttf"
              ];
  
  var str = "Pxscene is an allows application engine that has been added to the RDK.\
  It is a scene graph API exposed to a Javascript engine.  \
  In other words, it allows for set top box applications to be authored in javascript.\
  The authored javascript has access to the pxscene API for visual elements allow that are used for composition."
  
  var rect1 = scene.create({t:"rect", parent:root,w:300, h:40,x:300,y:0,fillColor:0xffffffff, lineColor:0xFFFFFFFF, lineWidth:1, clip:false});
  var rect2 = scene.create({t:"rect", parent:root,w:300, h:40,x:300,y:100,fillColor:0xffffffff, lineColor:0xFFFFFFFF, lineWidth:1, clip:false});
  var rect3 = scene.create({t:"rect", parent:root,w:300, h:40,x:300,y:200,fillColor:0xffffffff, lineColor:0xFFFFFFFF, lineWidth:1, clip:false});
  var rect4 = scene.create({t:"rect", parent:root,w:300, h:40,x:300,y:300,fillColor:0xffffffff, lineColor:0xFFFFFFFF, lineWidth:1, clip:false});
  var rect5 = scene.create({t:"rect", parent:root,w:300, h:40,x:300,y:400,fillColor:0xffffffff, lineColor:0xFFFFFFFF, lineWidth:1, clip:false});
  var rect6 = scene.create({t:"rect", parent:root,w:300, h:40,x:300,y:500,fillColor:0xffffffff, lineColor:0xFFFFFFFF, lineWidth:1, clip:false});
  
  var txt = scene.create({t:"textBox",wordWrap:false,w:300,h:40,x:0,y:0,textColor:0xff0000ff,text:str,parent:rect1,pixelSize:15,truncation:0,clip:false,alignHorizontal:0});
  var txt = scene.create({t:"textBox",wordWrap:false,w:300,h:40,x:0,y:0,textColor:0xff0000ff,text:str,parent:rect2,pixelSize:15,truncation:0,clip:true,alignHorizontal:0});
  var txt = scene.create({t:"textBox",wordWrap:false,w:300,h:40,x:0,y:0,textColor:0xff0000ff,text:str,parent:rect3,pixelSize:15,truncation:0,clip:false,alignHorizontal:1});
  var txt = scene.create({t:"textBox",wordWrap:false,w:300,h:40,x:0,y:0,textColor:0xff0000ff,text:str,parent:rect4,pixelSize:15,truncation:0,clip:true,alignHorizontal:1});
  var txt = scene.create({t:"textBox",wordWrap:false,w:300,h:40,x:0,y:0,textColor:0xff0000ff,text:str,parent:rect5,pixelSize:15,truncation:0,clip:false,alignHorizontal:2});
  var txt = scene.create({t:"textBox",wordWrap:false,w:300,h:40,x:0,y:0,textColor:0xff0000ff,text:str,parent:rect6,pixelSize:15,truncation:0,clip:true,alignHorizontal:2});
  
  Promise.all([txt.ready]).then(function(){
  },function(err){
    root.removeAll();
    var Error_status = txt.font.loadStatus.statusCode;
    var Error_http = txt.font.loadStatus.httpStatusCode;
    var Error = "Could not Load" + fonts[0] + "\nERROR:" + Error_status + " HTTPERROR:" + Error_http;
    scene.create({t:"text",text:Error,parent:root})
  });
  }).catch( function importFailed(err){
    console.error("Import failed for charWidthAlignment.js: " + err)
  });