var root = scene.root;
var container = scene.create({t:"object", parent:root, w:400, h:400, clip:false});

var url;
url = process.cwd() + "/../../images/skulls.png";
var url2;
url2 = process.cwd() + "/../../images/flower1.jpg";
var image = scene.createImage({url:url,stretchX:1,stretchY:1,parent:container,autoSize:false});

image.ready.then(function(e) {
  console.log("inside first image ready!");
  image.w = 500;
  image.h = 500;
});

image.on("onMouseDown", function(m) {

console.log("Inside onMouseDown");

  if( image.url== process.cwd() + "/../../images/skulls.png") {
    console.log("image is currently skull");
    image.url = url2;
    image.ready.then(function(text) {
      console.log("inside image.ready url2 (flower)");

      image.sx = 400/image.resource.w;
      image.sy = 400/image.resource.h;

    });
    
  } else {
    console.log("image is currently flower");
    image.url = url;
    image.ready.then(function(text) {
      console.log("inside image.ready url (skull)");

      image.sx = 400/image.resource.w;
      image.sy = 400/image.resource.h;

    });
}  

  
});


//image.ready.then(function(text) {
	//console.log("inside image.ready");

  //image.sx = 400/image.w;
	//image.sy = 400/image.h;

//});
