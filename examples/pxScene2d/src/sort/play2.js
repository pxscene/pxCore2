px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;
  
  var w = 68;
  var h = 40;
  var baseURL = "http://54.146.54.142/images/logo.";
  for (var k = 0; k < 1; k++) {
    for(var i = 1; i <= 29; i++) {
      var url = baseURL+i+".png";
      console.log(url);
      var j = i-1;
      scene.createImage({x:j%10*w,y:Math.floor(j/10)*h,url:url,parent:root});
    } 
  }

});
