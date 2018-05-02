px.import({
    scene: 'px:scene.1.js'
  })
  .then(function importsReady(imports) {
    var scene = imports.scene;
    var root = scene.root;

    let rect_one = scene.create({
      parent: scene.root,
      t: 'rect',
      id: 'one',
      x: 50,
      y: 50,
      w: 100,
      h: 100
    });

    function listener(event) {
      console.log("Event received");
    }

    scene.on("addEventsProper", function() { rect_one.on('onListener', listener); console.log("Received add events proper **************");  });
    scene.on("removeEventsProper", function() { rect_one.delListener("onListener", listener); });
    scene.on("removeEventsImProper", function() { rect_one.delListener("onListener", null); });
  });
