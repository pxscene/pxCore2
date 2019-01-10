px.import("px:scene.1.js").then(function ready(scene){
var root = scene.root;
var basePackageUri = px.getPackageBaseFilePath();

//the following url is a flickr url.  this should remain a flickr url to test and ensure the image can download with a proxy using ipv6

var background = scene.create({t:"rect",parent:root,clip:false, fillColor:0x00ff00ff,lineColor:0xffff0080,lineWidth:0,x:0,y:0,w:1280,h:720,a:1});
var url = "http://farm5.staticflickr.com/4170/34391367796_63bf9ce2fb_b.jpg";
var proxyUrl = px.appQueryParams.proxy;
if (proxyUrl === undefined)
{
  proxyUrl = "";
  console.log("not using a proxy");
}
else
{
  console.log("using the following proxy url: " + proxyUrl);
}
var imageRes = scene.create({t:"imageResource",url:url, proxy:proxyUrl});
var grayImage = scene.create({t:"image",resource:imageRes, parent:root,clip:false});


}).catch( function importFailed(e) {
  console.error("Import failed for test_proxy.js: " + e)
});

