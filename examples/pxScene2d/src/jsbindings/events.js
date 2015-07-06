var root = scene.root;

function randomInt(from, to) {
	var range = to-from;
	return Math.round(Math.random()*range + from);
}

var bg = scene.createRectangle({fillColor:0xcccccc00, parent:root});
var back = scene.createImage({parent:bg});
var front = scene.createImage({parent:bg});

function updateSize(w, h) {
  bg.w = w;
  bg.h = h;
}

scene.on("onResize", function(e) {updateSize(e.w, e.h);});
updateSize(scene.w, scene.h);

scene.root.on("onKeyDown", function(e) {
  var keycode = e.keyCode; var flags = e.flags;
  var keytext = ""+Math.floor(keycode);
  var textbg = scene.createImage({a:0, x:randomInt(50,scene.w-150), 
                                  y:scene.h+50, 
                                  url:process.cwd()+"/../../images/keybubble.png",
                                  parent:back,sx:0.75, sy:0.75});
  textbg.cx = textbg.w/2;
  textbg.cy = textbg.h/2;
  var text = scene.createText({text:keytext,parent:textbg,pixelSize:48});
  text.x = (textbg.w-text.w)/2;
  text.y = (textbg.h-text.h)/2;
  text.cx = text.w/2;
  text.cy = text.h/2;
  textbg.animateTo({a:1,y:randomInt(20,200),r:randomInt(-30,30)},0.2,scene.PX_STOP,0)
    .then(function(t) { 
      t.animateTo({r:randomInt(-15,15), y: t.y+50}, 0.6, 0, 0)
        .then(function(t) {
          t.animateTo({sx:1, sy: 1}, 0.01, 0, 0)
            .then(function(t) {
              t.animateTo({a:0,sx:0.25,sy:0.25}, 0.2, 0, 0)
                .then(function(t) {
                  t.remove();
                })
            })
        })
    });
});


function balloon(eventName, imageURL, textColor, offset, parent) {
  return function(e) {
      var x = e.x; var y = e.y;

    var textbg = scene.createImage9({url:imageURL,parent:parent,r:randomInt(-10,10)});
    textbg.x = x-textbg.w/2;
    textbg.y = y-textbg.h
    textbg.cx = textbg.w/2;
    textbg.cy = textbg.h/2;
    var text = scene.createText({text:eventName, parent:textbg, 
                                 textColor:textColor});
    text.x = (textbg.w-text.w)/2;
    text.y = (textbg.h-text.h-10)/2;
    text.cx = text.w/2;
    text.cy = text.h/2;
    textbg.animateTo({y: textbg.y-10-offset}, 0.3, 0, 0)
      .then(function(t) {
        t.animateTo({a:0}, 0.3, 4, 0)
          .then(function(t) {
            t.remove();
          })
      });
  }
}

scene.on("onMouseMove", balloon("mousemove", process.cwd()+"/../../images/textballoon_white.png", 0x000000ff,0,
                              back));
scene.on("onMouseDown", balloon("mousedown", process.cwd()+"/../../images/textballoon_blue.png", 0xffffffff,0,
                              front));
scene.on("onMouseUp",   balloon("mouseup",   process.cwd()+"/../../images/textballoon_red.png", 0xffffffff,25,
                              front));

function blah(eventname) {
  
  return function(x, y) {
    var text = scene.createText({r:randomInt(-10,10),
                                 text:eventname, parent:root});
    text.x = scene.w-text.w/2;
    text.y = scene.h-text.h/2;
    text.cx = text.w/2;
    text.cy = text.h/2;
    text.animateTo({y: text.y-10}, 0.3, 0, 0)
        .then(function(t) {
          t.animateTo({a:0}, 0.3, 4, 0)
            .then(function(t) {
              t.remove();
            })
        });
  }
}

scene.on("mouseenter", blah("mouseenter"));
scene.on("mouseleave", blah("mouseleave"));
