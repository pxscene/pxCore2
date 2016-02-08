var root = scene.root;
var container = scene.create({t:"object", parent:root, w:400, h:400, clip:false});

var url;
url = process.cwd() + "/../../images/skulls.png";
var url2;
url2 = process.cwd() + "/../../images/flower1.jpg";

var resource = scene.create({t:"imageResource", url:url});
console.log("resource url is "+resource.url);
var resource2 = scene.create({t:"imageResource",url:url2});
console.log("resource2 url is "+resource2.url);

var image = scene.create({t:"image", resource:resource,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH,parent:container,autoSize:false});

console.log("image object description is "+image.description); 

image.ready.then(function(e) {
  console.log("inside first image ready!");
  image.w = 500;
  image.h = 500;
});

image.on("onMouseDown", function(m) {

console.log("Inside onMouseDown");

  if( image.url== process.cwd() + "/../../images/skulls.png") {
    console.log("image is currently skull");
    //image.url = url2;
    image.resource = resource2;
    image.ready.then(function(i) {
      console.log("image ready1 is "+image.ready);
    
      console.log("inside image.ready url2 (flower)");
      //console.log("flower w="+i.resource.w+" and h="+i.resource.h);
      image.sx = 400/image.resource.w;
      image.sy = 400/image.resource.h;
      //var status = i.resource.loadStatus;
      //console.log("Status:");
      //console.log(status);
      //console.log("image statusCode is "+i.resource.loadStatus.statusCode);
    });
    
  } else {
    console.log("image is currently flower: "+image.url);
    //image.url = url;
//    image.resource = resource;
console.log("image ready is "+image.ready);

    image.ready.then(function(i2) {
      console.log("inside image.ready url (skull)");
      //console.log("skull w="+i2.resource.w+" and h="+i2.resource.h);
      image.sx = 400/image.resource.w;
      image.sy = 400/image.resource.h;
      //var status = i2.resource.loadStatus;
      //console.log("Status:");
      //console.log(status);
      //console.log("image statusCode is "+i2.resource.loadStatus.statusCode);
    });
    resource2.ready.then(function(o) {
        console.log("Resource2 says it's ready! "+resource.url);
    });
}  

  
});


//image.ready.then(function(text) {
	//console.log("inside image.ready");

  //image.sx = 400/image.w;
	//image.sy = 400/image.h;

//});
