px.import("px:scene.1.js").then( function ready(scene) {
var root = scene.root;
var basePackageUri = px.getPackageBaseFilePath();
scene_w = scene.getWidth();
scene_h = scene.getHeight();
var images = [
              "http://tvxcts-c5-c00001-b.ch.tvx.comcast.com:10004/compliance/images/Image3.png",
              "http://tvxcts-c5-c00001-b.ch.tvx.comcast.com:10004/compliance/images/Image2.png",
              "http://tvxcts-c5-c00001-b.ch.tvx.comcast.com:10004/compliance/images/Image1.png"
            ];

var res = scene.create({t:"imageResource",url:images[0]});
var img1 = scene.create({t:"image",resource:res,parent:root,x:0,y:0,painting:true,stretchX:0.25});
var img2 = scene.create({t:"image",resource:res,parent:root,x:0,y:res.h,painting:true,stretchX:-5});

Promise.all([img1.ready,img2.ready]).then(function(){
  var img3 = scene.create({t:"image",resource:res,parent:root,x:res.w+50,y:100,painting:true,clip:true});
},function(err){
  root.removeAll();
  var Error_status = res.loadStatus.statusCode;
  var Error_http = res.loadStatus.httpStatusCode;
  var Error = "Could not Load" + images[0] + "\nERROR:" + Error_status + " HTTPERROR:" + Error_http;
  scene.create({t:"text",text:Error,parent:root})
});
}).catch( function importFailed(err){
  console.error("Import failed for cliptest.js: " + err)
});

