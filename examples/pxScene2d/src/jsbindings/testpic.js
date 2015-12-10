var root = scene.root;

var url = "http://farm4.static.flickr.com/3307/5767175230_b5d2bf2312_z.jpg";
//var url = process.cwd()+"/../../images/ball.png";
// This works if load.js is updating the FPS text output on screen, but not if it's not!
var myImage = scene.createImage({parent:root,w:200,h:200,autoSize:false,clip:true,xStretch:1,yStretch:1});
myImage.url = url;
//,autoSize:false,xStretch:1,yStretch:1
myImage.ready.then(function(obj)
{
  console.log("promise received");
  console.log("image dimensions before set are w="+myImage.w+" and h="+myImage.h);
  //obj.w = 200;
  //obj.h = 200;
  console.log("image dimensions are w="+myImage.w+" and h="+myImage.h);
  
  obj.ready.then(function(newObj)
  {
    console.log("promise received");    
  });
});
