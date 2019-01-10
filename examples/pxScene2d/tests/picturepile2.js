px.import("px:scene.1.js").then( function ready(blah) {

var $ = (function(s) {
  var pxQuery = function(selector, context) {
    console.log("Selector: ", selector);
  }

  Object.setPrototypeOf(pxQuery, s);

  pxQuery.blah = function(s) {
    console.log("blahblahbAH",s);
  }

  return pxQuery; // publish
})(blah);

$("zzzzzzzzzzz");
$.blah("foo");


var root = $.root;
var basePackageUri = "http://pxscene.org/examples/px-reference/gallery/";
var bgUrl = basePackageUri+"/images/cork.png";
var bgShadowUrl = basePackageUri+"/images/radial_gradient.png";
var shadowUrl = basePackageUri+"/images/BlurRect.png";

function randomInt(from, to) {
	var range = to-from;
	return Math.round(Math.random()*range + from);
}

function getImageURL() {
  var urls = [
    "IMG_2225.jpg",
    "IMG_2810.jpg",
    "IMG_4321.jpg",
    "IMG_2804.jpg",
    "IMG_4765.jpg",
    "IMG_4077.jpg",
  ];
	return basePackageUri+"/images/photos/"+
    urls[randomInt(0,urls.length-1)];
}

var maxCover = 0.7;
var maxW;
var maxH;

var bg = $.create({t:"image",url:bgUrl,parent:root,stretchX:2,stretchY:2});
var bgShadow = $.create({t:"image",url:bgShadowUrl,parent:bg,stretchX:1,stretchY:1,a:0.75});

var numPictures = 0;
// back layer
var picturesBg = $.create({t:"object",parent:root});
// middle layer
var pictures = $.create({t:"object",parent:root});
// front layer
var picturesFg = $.create({t:"object",parent:root});

function doIt() {
   console.log("PICTURE PILE : called doIT");  
	var urlIndex = 0;
	
  function newPicture() {
    
   console.log("PICTURE PILE : created fg");  
    var url = getImageURL();
    var picture = $.create({t:"object",parent:picturesFg,
                                x:(randomInt(0,1)==0)?-1000:$.w+2000,
                                 y:randomInt(-200, 800),
                                 sx: 3, sy: 3, 
                                 r: randomInt(-45,45),a:0});
   console.log("PICTURE PILE : created picture");  
    var shadow = $.create({t:"image9",x:-37,y:-37,w:200,h:200,url:shadowUrl,parent:picture,a:0.45,insetTop:48,insetBottom:48,insetLeft:48,insetRight:48});
   console.log("PICTURE PILE : creating fg" + url);  
    var fg = $.create({t:"image",x:0,y:0,parent:picture,url:url,sx:0.25,sy:0.25});
    
   console.log("PICTURE PILE : created fg");  
    fg.ready.then(function(pic){
   console.log("PICTURE PILE : fg ready");  
      picture.paint = false;
      picture.a = 1;
      var picW = pic.resource.w;
      var picH = pic.resource.h;
      var sx = Math.min(1,maxW/pic.resource.w);
      var sy = Math.min(1,maxH/pic.resource.h);
      var s = (sx<sy)?sx:sy;
      fg.sx = fg.sy = s;
      shadow.w = (pic.resource.w*s)+37+41;
      shadow.h = (pic.resource.h*s)+37+41;
      picture.cx = shadow.w/2;
      picture.cy = shadow.h/2;

      picture.animateTo({x:randomInt(50,$.w-(picW*fg.sx)-50),
                          y:randomInt(50,$.h-(picH*fg.sx)-50),
                          r:randomInt(-15,15),sx:1,sy:1},1,$.animation.TWEEN_STOP,$.animation.OPTION_LOOP, 1)
        .then(function() {
   console.log("PICTURE PILE : picture animate called " + pictures.numChildren);  
          picture.parent = pictures;
          pictures.painting = true; 
          pictures.painting=false;
          if (pictures.numChildren > 10) {
            var f = pictures.getChild(0);
            f.parent = picturesBg;
            pictures.painting = true; pictures.painting = false;
            f.animateTo({a: 0}, 0.75, $.animation.TWEEN_LINEAR, $.animation.OPTION_LOOP, 1)
              .then(function(f){
   console.log("PICTURE PILE : f remove about to call");  
                f.remove();
              });
          }
   console.log("PICTURE PILE : calling new picture");  
          newPicture();
        });    
    },function(){
      var res = picture.resource;
      console.log("Error loading image statusCode:"+res.loadStatus.statusCode+
                  " httpStatusCode:"+res.loadStatus.httpStatusCode);
      picture.remove();
      newPicture();
    });
   } 
   console.log("PICTURE PILE : calling new picture from outside");  
  newPicture();
}

function updateSize(w, h) {

  bg.w = w;
  bg.h = h;

  bgShadow.w = w;
  bgShadow.h = h;

  pictures.w = w;
  pictures.h = h;
  pictures.painting = true; pictures.painting = false;

  maxW = w*maxCover;
  maxH = h*maxCover;
}

$.on("onResize", function(e){updateSize(e.w,e.h);});
updateSize($.w, $.h);

doIt();

}).catch( function importFailed(err){
  console.error("Import failed for picturepile.js: " + err)
});




