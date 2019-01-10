px.import("px:scene.1.js").then( function ready(scene) {

var text = scene.create({
    t: 'textBox',
    text: 'The quick brown fox jumps over the lazy dog',
    size: 24,
    fontUrl: null,
    wordWrap: true,
    alignVertical: 0,
    alignHorizontal: 0,
});

var view = scene.create({
    t: 'object',
    w: 160,
    h: 205,
    x: 50,
    y: 50,
    parent: scene.root
});
text.w = view.w;
text.h = view.h;
text.parent = view;

var json = {
    'w': 10,
    'h': 10
};

text.ready.then(function(textObj) {
    view.w = json.w;
    view.h = json.h;
    textObj.animateTo(json, 12, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
});

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-79.js: " + err)
});
