px.import("px:scene.1.js").then( function ready(scene) {

var font = scene.create({t:"fontResource",url:"FreeSans.ttf"});

var textBox1 = scene.create({t:"textBox", parent:scene.root,h:1,w:1,fontUrl:"FreeSans.ttf",
                             alignVertical:scene.alignVertical.CENTER,text:"age 16+"});
scene.create({t:"textBox", parent:scene.root,x:60,h:1,w:1,fontUrl:"FreeSans.ttf",
                             alignVertical:scene.alignVertical.TOP,text:"age 16+"});
var textBox2 = scene.create({t:"textBox", parent:scene.root,x:120,h:1,w:1,fontUrl:"FreeSans.ttf",
                             alignVertical:scene.alignVertical.BOTTOM,text:"age 16+"});                            
font.ready.then(function(){ 
  var metrics = font.getFontMetrics(textBox1.pixelSize);
  textBox1.h = metrics.height;  
  textBox2.h = metrics.height;                          
  var measure = textBox1.measureText();
  console.log("textBox1 width is "+measure.bounds.x2);
  console.log("textBox1 y is "+measure.bounds.y1);
});
}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-54-simple.js: " + err)
});
