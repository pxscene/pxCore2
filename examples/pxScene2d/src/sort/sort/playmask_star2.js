px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;

  var url;

  var w = 180;
  var h = 180;
  var across = 15;
  var baseURL = "http://54.146.54.142/images/dvd_tile.";
  for(var i = 1; i <= /*209*/90; i++) {
    var url = baseURL+i+".jpg";
    var j = i-1;
    var maskurl = process.cwd()+"/../../images/star.png";
    var container = scene.createImage({x:j%across*w,y:Math.floor(j/across)*h,w:w,h:h,parent:root,mask:maskurl,cx:w/2,cy:h/2});
    container.animateTo({r:360},2.0,scene.PX_LINEAR,scene.PX_LOOP);
    scene.createImage({url:url, parent:container, onReady:function(e){
      var o = e.target;
      
      o.x = (container.w-o.w)/2;
      o.y = (container.h-o.h)/2;
      o.cx = o.w/2;
      o.cy = o.h/2;

      o.animateTo({r:-360},1.0,scene.PX_LINEAR,scene.PX_LOOP);
    }});
  }  
  
  function updateSize(w, h) {
  }
  
  scene.on("onResize", function(e) {updateSize(e.w,e.h);});
  updateSize(scene.w, scene.h);
  
});
