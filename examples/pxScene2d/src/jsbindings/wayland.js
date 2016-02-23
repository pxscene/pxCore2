var westonDir = "/home/johnrobinson/Documents/";

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

//var wayland = scene.createWayland( {displayName:"nested0", width:640, height:360, parent:root} );

var txt1= scene.create( {t:"text", x:60, y:50, pixelSize: 30, h:40, text:"", parent:root} );
var txt2= scene.create( {t:"text", x:60, y:90, pixelSize: 30, h:35, text:"", parent:root} );
var txt3= scene.create( {t:"text", x:640, y:50, pixelSize: 30, h:40, text:"", parent:root} );
var txt4= scene.create( {t:"text",x:640, y:90, pixelSize: 30, h:35, text:"", parent:root} );

var wayland1, wayland2;

setTimeout(function(){

  var cmd = westonDir+"weston/weston-terminal";
  var owayland1 = scene.create( {t:"rect", x:200,y:200,w:750,h:440,parent:root ,fillColor:0xffffffff});
  wayland1 = scene.create( {t:"wayland", displayName:"nested2", x:0, y:0, w:750, h:440, parent:owayland1, cmd:cmd, fillColor:0x00000033} );

  wayland1.on("onMouseUp", function(e) {
    console.log("here");
    wayland1.focus = true;
  });

  wayland1.on("onClientConnected", function(e) { txt1.text="Client connected: pid: "+e.pid; txt2.text="" });
  wayland1.on("onClientDisconnected", function(e) { txt1.text="Client disconnected: pid: "+e.pid; });
  wayland1.on("onClientStopped", function(e) { 
    if ( e.crashed == true ) {
       txt2.text="Client crashed: pid: "+e.pid+" signo:"+e.signo; 
       txt2.textColor=0xFF8080FF;
    } else {
       txt2.text="Client ended normally: pid: "+e.pid+" code:"+e.exitCode; 
       txt2.textColor=0x20FF20FF; 
    }
  });
  
  //scene.setFocus(wayland1);
  wayland1.focus = true;
  owayland1.cx = owayland1.w/2;
  owayland1.cy = owayland1.h/2;
  owayland1.animateTo({r:360},5,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP);

  wayland1.cx = wayland1.w/2;
  wayland1.cy = wayland1.h/2;
  wayland1.animateTo({r:360,a:0.5},5,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_OSCILLATE);

 
}, 4000 );

setTimeout(function(){
  var cmd = westonDir+"weston/weston-clickdot";
//  var cmd = westonDir+"weston/weston-simple-egl";
//  var cmd = westonDir+"weston/weston-flower";
   wayland2 = scene.create( {t:"wayland", x:640, y:360, w:640, h:460, r:-20, parent:root, cmd:cmd, sx:0.5, sy:0.5, fillColor:0x00000055} );

   wayland2.on("onClientConnected", function(e) { txt3.text="Client connected: pid: "+e.pid; txt2.text="" });
   wayland2.on("onClientDisconnected", function(e) { txt3.text="Client disconnected: pid: "+e.pid; });
   wayland2.on("onClientStopped", function(e) { 
    if ( e.crashed == true ) {
       txt4.text="Client crashed: pid: "+e.pid+" signo:"+e.signo; 
       txt4.textColor=0xFF8080FF;
    } else {
       txt4.text="Client ended normally: pid: "+e.pid+" code:"+e.exitCode; 
       txt4.textColor=0x20FF20FF; 
    }
  });
}, 10000 );


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
//  scene.setFocus(content);
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
//inputbg.focus = true;
//scene.setFocus(wayland);
wayland.focus = true;

}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err)
});

