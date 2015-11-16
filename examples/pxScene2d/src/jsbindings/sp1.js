px.import("px:scene.1.js").then(function ready(scene) {


  scene.create({t:"text", text:"Hello",parent:scene.root, pixelSize:24});

  var child = scene.createScene({url:"sp2.js",y:100,w:200,h:200,parent:scene.root});


  child.ready.then(function() {
    console.log("child scene is ready");
  });

  //child.makeReady(true);

});
