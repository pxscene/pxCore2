var root = scene.root;

var url = "http://farm4.static.flickr.com/3307/5767175230_b5d2bf2312_z.jpg";
//var container = scene.create({t:"object", parent:root, w:root.w, h:root.h});
//scene.createImage({url:url,parent:container});
// This works now with changes to remove autoSize and use w/h as -1
var myImage = scene.create({t:"image",parent:root,url:url,clip:false,  w:150, h:150});

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
