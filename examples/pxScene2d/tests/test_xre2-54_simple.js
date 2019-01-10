px.import("px:scene.1.js").then( function ready(scene) {

var textBox1 = scene.create({t:"textBox", parent:scene.root,h:1,w:1,fontUrl:"FreeSans.ttf",
                             alignVertical:scene.alignVertical.CENTER,text:"age 16+"});
scene.create({t:"textBox", parent:scene.root,x:60,h:1,w:1,fontUrl:"FreeSans.ttf",
                             alignVertical:scene.alignVertical.TOP,text:"age 16+"});
                             
var measure = textBox1.measureText();
console.log("textBox1 width is "+measure.bounds.x2);
console.log("textBox1 y is "+measure.bounds.y1);

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-54-simple.js: " + err)
});
