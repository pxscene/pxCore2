px.import("px:scene.1.js").then( function ready(scene) {
  var root = scene.root;

  var r = scene.create({t:"rect",x:900,parent:root,y:10,w:350,h:720,fillColor:0x000000FF});
  var t1Size =   scene.create({t:"text", parent:r,w:70,h:70,x:100,y:10});
  var font = scene.create({t:"fontResource", url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/DejaVuSans.ttf"});

  font.ready.then(obj=>{
    var size = font.measureText(25, 'hello!');
    t1Size.text = size.w;
    console.log("text size ",size.w);
  }).catch(err=>{
      console.log("error on font loading",err);
  })

}).catch( err=>{
  console.error("Import failed for test_fontMeasureText.js: " + err)
});