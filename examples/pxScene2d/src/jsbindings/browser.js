px.import("px:scene.1.js").then( function ready(scene) {
var root = scene.root;
var bg = scene.create({t:"image",url:"../../images/status_bg.png",parent:root,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH});
var inputRes = scene.create({t:"imageResource",url:"../../images/input2.png"});
var inputbg = scene.create({t:"image9",resource:inputRes,a:0.9,x:10,y:10,w:400,insetLeft:10,insetRight:10,insetTop:10,insetBottom:10,parent:bg});
var prompt = scene.create({t:"text",text:"Enter Url to JS File or Package",parent:inputbg,pixelSize:24,textColor:0x869CB2ff,x:10,y:2});
var url = scene.create({t:"text",text:"",parent:inputbg,pixelSize:24,textColor:0x303030ff,x:10,y:2});
var cursor = scene.create({t:"rect", w:2, h:inputbg.h-10, parent:inputbg,x:10,y:5});

cursor.animateTo({a:0},0.5,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_OSCILLATE);


var contentBG = scene.create({t:"rect",x:10,y:60,parent:bg,fillColor:0xffffffff,a:0.05,draw:false});
var content = scene.create({t:"scene",x:10,y:60,parent:bg,clip:true});

inputbg.on("onChar",function(e) {
  if (e.charCode == 13) 
    return;
  // TODO should we be getting an onChar event for backspace
  if (e.charCode != 8) {
    url.text += String.fromCharCode(e.charCode);
    prompt.a = (url.text)?0:1;
    cursor.x = url.x+url.w;
  }
});

inputbg.on("onKeyDown", function(e) {
  if (e.keyCode == 13) {

    var u = url.text;
    // TODO Temporary hack
    if (u.indexOf(':') == -1)
      u = 'http://www.pxscene.org/examples/px-reference/gallery/' + u;

    content.url = u;
    //scene.setFocus(content);
    content.focus = true;
    content.ready.then(function() {
      contentBG.draw = true;
/*
      console.log("api after promise:"+content.api);
      if (content.api) {
        content.api.test("john");
        content.api.middle.fillColor=0x000000ff;
      }
*/
    },
                       function() {
                         contentBG.draw = false;
                         console.log("scene load failed");
                       }
    );
  }
  else if (e.keyCode == 8) {
    var s = url.text.slice();
    console.log(url.text);
    console.log(s.slice(0,url.text.length-2));
//    url.text = s.slice(0,s.length-2);
    url.text = s.slice(0,-1);
  }
  prompt.a = (url.text)?0:1;
  cursor.x = url.x+url.w;
});

inputbg.on("onFocus", function(e) {
  cursor.draw = true;
});

inputbg.on("onBlur", function(e) {
  cursor.draw = false;
});

inputbg.on("onMouseUp", function(e) {
  //scene.setFocus(inputbg);
  inputbg.focus = true;
});

content.on("onMouseUp", function(e) {
  scene.setFocus(content);
  content.focus=true;
});

function updateSize(w,h) {
  bg.w = w;
  bg.h = h;
  inputbg.w = w-20;
  content.w = w-20;
  content.h = h-70;
  contentBG.w = w-20;
  contentBG.h = h-70;
}

scene.root.on("onPreKeyDown", function(e) {
  if (e.keyCode == 76 && e.flags == 16) { // ctrl-l
    //console.log("api:"+content.api);
//    if (content.api) content.api.test(32);
    //scene.setFocus(inputbg);
    inputbg.focus = true;
    url.text = "";
    prompt.a = (url.text)?0:1;
    cursor.x = 10;
    e.stopPropagation();
  }
});

scene.on("onResize", function(e) { updateSize(e.w,e.h); });
updateSize(scene.w,scene.h);

//scene.setFocus(inputbg);
inputbg.focus = true;
}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err)
});

