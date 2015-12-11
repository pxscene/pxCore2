var root = scene.root;

var url = "http://farm4.static.flickr.com/3307/5767175230_b5d2bf2312_z.jpg";

//var container = scene.create({t:"object", parent:root, w:root.w, h:root.h});
//scene.createImage({url:url,parent:container});

// This works if load.js is updating the FPS text output on screen, but not if it's not!
var myImage = scene.create({t:"image",parent:root,url:url,w:-1,h:-1,autoSize:false,clip:false});

myImage.ready.then(function(obj)
{
  console.log("promise received");
  console.log("image dimensions before set are w="+myImage.w+" and h="+myImage.h);
  //obj.w = 200;
  //obj.h = 200;
  console.log("image dimensions are w="+myImage.w+" and h="+myImage.h);
  
  obj.ready.then(function(newObj)
  {
    console.log("second promise received");    
  });
});
