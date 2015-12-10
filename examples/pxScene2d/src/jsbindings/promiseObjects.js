var root = scene.root;
var mediumText = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog."

/****************** Test Rectangle ************************************/
var rect = scene.create({t:"rect", parent:root, x:100, y:100, w:400, h:400, fillColor:0x00000000, lineColor:0xFF0000FF, lineWidth:1, clip:false});

root.ready.then(function(sceneObj) {
  console.log("received scene promise");
});

var rectReady1 = rect.ready;
rect.ready.then(function(obj){
  console.log("received rect promise");
  //obj.fillColor = 0x00FFFFFF;
  obj.lineWidth = 5;
  
  if(obj.ready == rectReady1) 
    console.log("first rect ready is same as second ready");
  else 
    console.log("first rect ready is NOT same as second ready");
    
  obj.ready.then(function(obj2){
    console.log("received 2nd rect promise");
    console.log("rect's lineWidth="+obj2.lineWidth);
  });
});

/****************** Test Text ************************************/
var myText = scene.create({t:"text", parent:root, x:200,y:200,textColor:0xFFFFDDFF,text:"Just some test text to test promises"});
var textReady1 = myText.ready;
myText.ready.then(function(textObj) {
  console.log("received text promise");
  textObj.text = "Looks pretty good!";
  
  if(textObj.ready == textReady1) 
    console.log("first text ready is same as second ready");
  else 
    console.log("first text ready is NOT same as second ready");  
  
  obj.ready.then(function(textObj2){
    console.log("received 2nd text promise");
    console.log("text's text="+textObj2.text);
  });  
});

/****************** Test Text2 ************************************/
//!CLF: TO DO:  text2 y position is broken when it's inside root - or more generally?  As if it doubles so that y is off screen!!
var text2 = scene.create({t:"text2", clip:true, parent:root, x:200, y:150,
                            h:400,w:200,textColor:0xFFFFFFFF, pixelSize:20,
                            faceURL:"http://54.146.54.142/tom/receiverdotjs/fonts/XFINITYSansTT-New-Lgt.ttf",
                            text:"then again... not really!",wordWrap:true,truncation:0,
                             rx:0, ry:0, rz:0});

text2.ready.then(function(text2Obj){
  console.log("received text2 promise");
  text2Obj.textColor=0xDDFF00FF;
  text2Obj.pixelSize=15; // !CLF:  Why does pixelSize=25 truncate when height should allow for wordWrap?
  text2Obj.ready.then(function(text2Obj2) {
    console.log("received 2nd text2 promise");
  });
});

/****************** Test Image ************************************/
var url = "http://farm4.static.flickr.com/3307/5767175230_b5d2bf2312_z.jpg";
//var url = process.cwd()+"/../../images/ball.png";
// This works if load.js is updating the FPS text output on screen, but not if it's not. 
// Doesn't seem to respect the passed in w/h; instead, must be set in promise!
var myImage = scene.create({t:"image",url:url,parent:rect,w:200,h:200,autoSize:false,clip:true,xStretch:1,yStretch:1});
//,autoSize:false,xStretch:1,yStretch:1
myImage.ready.then(function(obj)
{
  console.log("received image promise");
  console.log("image dimensions before set are w="+myImage.w+" and h="+myImage.h);
  obj.w = 200;
  obj.h = 200;
  console.log("image dimensions are w="+myImage.w+" and h="+myImage.h);
  
  obj.ready.then(function(newObj)
  {
    console.log("received 2nd image promise");    
  });
});

/****************** Test Image9 ************************************/
var url = "http://farm4.static.flickr.com/3307/5767175230_b5d2bf2312_z.jpg";
//var url = process.cwd()+"/../../images/ball.png";
// This works if load.js is updating the FPS text output on screen, but not if it's not. 
// Doesn't seem to respect the passed in w/h; instead, must be set in promise!
var myImage = scene.create({t:"image9",url:url,parent:rect,y:200,x:200,w:200,h:200,autoSize:false,clip:true,xStretch:1,yStretch:1});
//,autoSize:false,xStretch:1,yStretch:1
myImage.ready.then(function(obj)
{
  console.log("received image9 promise");
  console.log("image dimensions before set are w="+myImage.w+" and h="+myImage.h);
  obj.w = 200;
  obj.h = 200;
  console.log("image dimensions are w="+myImage.w+" and h="+myImage.h);
  
  obj.ready.then(function(newObj)
  {
    console.log("received 2nd image9 promise");    
  });
});
