var root = scene.root;
var container = scene.create({t:"object", parent:root, w:400, h:400, clip:false});

var url;
url = process.cwd() + "/../../images/skulls.png";
var url2;
url2 = process.cwd() + "/../../images/flower1.jpg";
var image = scene.createImage({url:url,xStretch:2,yStretch:2,parent:container, autoSize:false});

image.on("onMouseDown", function(e) {

  if( image.url== process.cwd() + "/../../images/skulls.png") {
    console.log("image is currently skull");
    image.url = url2;
    image.ready.then(function(text) {
      console.log("inside image.ready url2 (flower)");

      image.sx = 400/image.w;
      image.sy = 400/image.h;

    });
    
  } else {
    console.log("image is currently flower");
    image.url = url;
    image.ready.then(function(text) {
      console.log("inside image.ready url (skull)");

      image.sx = 400/image.w;
      image.sy = 400/image.h;

    });
}  

  
});
image.ready.then(function(e) {
  console.log("inside first image ready!");
  image.w = 50;
  image.h = 50;
});

//image.ready.then(function(text) {
	//console.log("inside image.ready");

  //image.sx = 400/image.w;
	//image.sy = 400/image.h;

//});
