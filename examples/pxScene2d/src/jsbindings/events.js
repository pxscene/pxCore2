var root = scene.root;

function randomInt(from, to) {
	var range = to-from;
	return Math.round(Math.random()*range + from);
}

var bg = scene.createRectangle({fillColor:0xccccccff, parent:root});

function updateSize(w, h) {
    bg.w = w;
    bg.h = h;
}

scene.on("resize", updateSize);
updateSize(scene.w, scene.h);

scene.on("keydown", function(keycode, flags) {
    var keytext = ""+Math.floor(keycode);
    var textbg = scene.createImage({a:0, x:randomInt(50,scene.w-150), y:scene.h+50, 
                                     url:process.cwd()+"/../../images/keybubble.png",
                                    parent:root,sx:0.75, sy:0.75});
    textbg.cx = textbg.w/2;
    textbg.cy = textbg.h/2;
    var text = scene.createText({text:keytext, parent:textbg});
    text.x = (textbg.w-text.w)/2;
    text.y = (textbg.h-text.h)/2;
    text.cx = text.w/2;
    text.cy = text.h/2;
    textbg.animateTo({a:1, y:randomInt(20,200), r:randomInt(-30,30)}, 0.2, 3, 0, function(t) { 
        t.animateTo({r:randomInt(-15,15), y: t.y+50}, 0.6, 0, 0, function(t) {
            t.animateTo({sx:1, sy: 1}, 0.01, 0, 0, function(t) {
                t.animateTo({a:0,sx:0.25,sy:0.25}, 0.2, 0, 0, function(t) {
                    t.remove();
                });
            });
        })
    });
});

scene.on("mousemove", function(x, y) {


    var textbg = scene.createImage9({url:process.cwd()+"/../../images/textballoon_red.png",
                                    parent:root,r:randomInt(-10,10)});
    textbg.x = x-textbg.w/2;
    textbg.y = y-textbg.h
    textbg.cx = textbg.w/2;
    textbg.cy = textbg.h/2;
    var text = scene.createText({text:"mousemove", parent:textbg, textColor:0x000000ff, sx:0.3, sy:0.3});
    text.x = (textbg.w-text.w)/2;
    text.y = (textbg.h-text.h-10)/2;
    text.cx = text.w/2;
    text.cy = text.h/2;
    textbg.animateTo({y: textbg.y-10}, 0.3, 0, 0, function(t) {
        t.animateTo({a:0}, 0.3, 4, 0, function(t) {
            t.remove();
        });
    });
});

scene.on("mousedown", function(x, y) {
    var text = scene.createText({r:randomInt(-10,10),
                                 text:"mousedown", parent:root});
    text.x = (x-text.w/2);
    text.y = y-text.h
    text.cx = text.w/2;
    text.cy = text.h/2;
    text.animateTo({y: text.y-10}, 0.3, 0, 0, function(t) {
        t.animateTo({a:0}, 0.3, 4, 0, function(t) {
            t.remove();
        });
    });
});

scene.on("mouseup", function(x, y) {
    var text = scene.createText({r:randomInt(-10,10),
                                 text:"mouseup", parent:root});
    text.x = (x-text.w/2);
    text.y = y-text.h
    text.cx = text.w/2;
    text.cy = text.h/2;
    text.animateTo({y: text.y-10}, 0.3, 0, 0, function(t) {
        t.animateTo({a:0}, 0.3, 4, 0, function(t) {
            t.remove();
        });
    });
});

function blah(eventname) {
    
    return function(x, y) {
        var text = scene.createText({r:randomInt(-10,10),
                                     text:eventname, parent:root});
        text.x = scene.w-text.w/2;
        text.y = scene.h-text.h/2;
        text.cx = text.w/2;
        text.cy = text.h/2;
        text.animateTo({y: text.y-10}, 0.3, 0, 0, function(t) {
            t.animateTo({a:0}, 0.3, 4, 0, function(t) {
                t.remove();
            });
        });
    }
}

scene.on("mouseenter", blah("mouseenter"));
scene.on("mouseleave", blah("mouseleave"));