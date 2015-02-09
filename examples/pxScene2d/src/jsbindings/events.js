var root = scene.root;

function randomInt(from, to) {
	var range = to-from;
	return Math.round(Math.random()*range + from);
}

scene.on("keydown", function(keycode, flags) {
    var keytext = ""+Math.floor(keycode);
    var text = scene.createText({x:randomInt(50,scene.w-50), y:scene.h+50, 
                                 text:keytext, parent:root});
    text.cx = text.w/2;
    text.cy = text.h/2;
    text.animateTo({y:randomInt(50,200), r:randomInt(-30,30)}, 0.2, 3, 0, function(t) { 
        t.animateTo({a:0}, 0.5, 0, 0, function(t) {
            t.remove();
        });
    });
});