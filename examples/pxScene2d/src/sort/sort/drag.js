px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;
  
  var bg = scene.createRectangle({fillColor:0xffffffff, parent:root, w: scene.w, h: scene.h});
  function clamp(v, min, max) {
    return Math.min(max, Math.max(min,v));
  }
  
  scene.on("onResize", function(e) {
    bg.w = e.w;
    bg.h = e.h;
  });
  
  var txt1 = scene.createText({text:"Hello",textColor:0x000000ff,parent:root});
  var rect1 = scene.createRectangle({fillColor:0xff0000ff, x:0,y:100,w:100, h:100, cx:50, cy:50, parent:root,
                                     onMouseEnter:function(e){txt1.text = "onMouseEnter";},
                                     onMouseLeave:function(e){txt1.text = "onMouseLeave";},
                                     onMouseDown:function(e){txt1.text="onMouseDown";}});


  var btn1 = scene.createRectangle({fillColor:0x0000ffff, x:200,y:100,w:100, h:100, parent:root});
  var btn2 = scene.createRectangle({fillColor:0xff00ffff, x:400,y:100,w:100, h:100, parent:root});
  
  btn1.on("onMouseDown", function(e) {
    rect1.animateTo({r:360},1,0,scene.PX_LOOP);
  });
  
  btn2.on("onMouseDown", function(e) {
    rect1.r = 10;
  });
  
  
  var track = scene.createRectangle({x:100,y:300,h:4,w:500,parent:root,fillColor:0xd0d0d0ff});
  var trackFill = scene.createRectangle({h:track.h,w:400,fillColor:0x00ff00ff,parent:track});
  var trackKnob = scene.createRectangle({x:-6,y:(4-12)/2,h:12,w:12,fillColor:0x00dd00ff,lineColor:0x707070ff, lineWidth:0,parent:track,cx:6,cy:6});
  
  var startX, startY;
  trackKnob.on("onMouseEnter", function(e) {
    startX = trackKnob.x;
    startY = trackKnob.y;
    trackKnob.animateTo({sx:1.5,sy:1.5},0.1,scene.PX_LINEAR, scene.PX_END);
  });
  
  trackKnob.on("onMouseLeave", function(e) {
    trackKnob.animateTo({sx:1.0,sy:1.0},0.1,scene.PX_LINEAR, scene.PX_END);
  });
  
  
  trackKnob.on("onMouseDrag", function(e) {
    v = clamp(startX + (e.x-e.startX),0,500);
    trackKnob.x = v-6;
    trackFill.w = v;
  });
  
  rect1.on("onMouseDown", function(e) {
    txt1.text = "onMouseDown";
  });
  
  rect1.on("onMouseEnter", function(e) {
    e.target.fillColor = 0x00ff00ff;
    txt1.text = "onMouseEnter";
  });
  
  rect1.on("onMouseLeave", function(e) {
    e.target.fillColor = 0xff0000ff;
    txt1.text = "onMouseLeave";
  });
  

  rect1.on("onMouseDown", function(e) {
    console.log("onMouseDown");
    startX = rect1.x;
    startY = rect1.y;
  });
  
  rect1.on("onMouseUp", function(e) {
    console.log("onMouseUp");
  });
  
  rect1.on("onMouseDrag", function(e) {
    console.log("onMouseDrag");
    rect1.x = startX + (e.x-e.startX);
    rect1.y = startY + (e.y-e.startY);
    console.log("xy: ", rect1.x, rect1.y);
  });
  
});
