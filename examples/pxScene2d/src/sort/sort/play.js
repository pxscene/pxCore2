px.import("px:scene.1.js").then( function ready(scene) {
  var root = scene.root;
  
  var w = 180;
  var h = 240;
  var across = 15;
  var baseURL = "http://54.146.54.142/images/dvd_tile.";
  for(var i = 1; i <= 209; i++) {
    var url = baseURL+i+".jpg";
    var j = i-1;
    scene.createImage({x:j%across*w,y:Math.floor(j/across)*h,url:url,
                       parent:root,cx:w/2,cy:h/2})
      .animateTo({r:-360},1.0,scene.PX_LINEAR,scene.PX_LOOP);
  } 
});
