var root = scene.root;

var green = 0x00FF00FF;
var blue = 0x0000FFFF;
var red = 0xFF0000FF;
var yellow = 0xFFFF00FF;
var orange = 0xFF8C00FF;
var textA = "ÉéÈèÇçËëÒòÔôÖöÙùÀàÂâAaBbCcDdEeFfGgHhIiKkLlMmNnOoPpQqRrSsTtVvXxYyZz123456789";
var textB = "The quick brown fox jumps over the lazy dog.";

function createRect(props) {
    var obj = {t:"rect", fillColor:0x00000000};
    for (var i in props) obj[i] = props[i];
    return scene.create(obj);
}

function onTextReady(text, props, bg) {
    
    console.log("onTextReady for text "+text.text);
    // measure
    var metrics = text.font.getFontMetrics(text.pixelSize);
    var measurements = text.measureText();
    var bounds = measurements.bounds;
    var charFirst = measurements.charFirst;
    var charLast = measurements.charLast;
    var w = bounds.x2 - bounds.x1;
    var spacing = metrics.height + props.leading;
    var baseline = metrics.baseline  + text.y;

    // show measurements
    var x = 0;
    var y = 0;
    do {
        createRect({parent:bg, fillColor:green, x:x, y:y + baseline - metrics.ascent, w:w, h:metrics.ascent});
        createRect({parent:bg, fillColor:blue, y:y + baseline, w:w, h:metrics.descent});
        createRect({parent:bg, lineColor:red, lineWidth:1, x:x, y:y, w:w, h:metrics.height});
        y += spacing;
    } while (y < bounds.y2);
    createRect({parent:bg, lineColor:yellow, lineWidth:1, y:bounds.y1, w:w, h:bounds.y2 - bounds.y1});
    createRect({parent:bg, lineColor:orange, lineWidth:1, x:charFirst.x, y:charFirst.y, w:charLast.x - charFirst.x, h:charLast.y - charFirst.y});
}

function createText(x, y, props) {
    var bg = scene.create({t:"object", parent:root, x:x, y:y, w:1000, h:1000, clip:false});
    var container = scene.create({t:"object", parent:root, x:x, y:y, w:1000, h:1000, clip:false});
    var obj = {t:"textBox", w:1000, h:1000, parent:container, textColor:0xFFFFFFFF, pixelSize:25, x:0, y:0, leading:0, wordWrap:false, clip:false};
    for (var i in props) obj[i] = props[i];
    var text2 = scene.create(obj);
    text2.ready.then(function(text) { onTextReady(text, obj, bg); });
    return text2;
}

createText(0, 0, { text:(textA + "\n" + textA + "\n" + textA), wordWrap:false, clip:false});
createText(0, 200, {fontUrl:"http://www.w3schools.com/cssref/sansation_light.woff", text:(textB + " " + textB + " " + textB), wordWrap:true, clip:false,pixelSize:30});
createText(0, 400, {fontUrl:"http://54.146.54.142/tom/receiverdotjs/fonts/DejaVuSans.ttf", text:(textB + "\n" + textB + "\n" + textB), wordWrap:true, clip:false, pixelSize:35, leading:20});

//http://www.w3schools.com/cssref/sansation_light.woff"
//http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-Med.ttf
