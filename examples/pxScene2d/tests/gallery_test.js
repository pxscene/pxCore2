"use strict";

px.import("px:scene.1.js").then(function ready(scene) {
  var root = scene.root;
  var appURLs = ["picturepile2.js", "apng1.js", "dynamics.js", "mousetest2.js", "fancy.js", "apng2.js", "masktest.js", "fonts.js", "events.js"];

  var url;
  var basePackageUri = "http://pxscene.org/examples/px-reference/gallery/";

  url = basePackageUri + "/images/status_bg.png";
  var bgShade = scene.create({ t: "image", parent: root, url: url, stretchX: scene.stretch.STRETCH, stretchY: scene.stretch.STRETCH });

  var childPad = 48;
  var childAppWidth = 1280;
  var childAppHeight = 720;
  var childAcross = 2;
  var selectWidth = 1280 + 2 * childPad;
  var selectHeight = 720 + 2 * childPad;

  var select;

  var apps = scene.create({ t: "image", parent: root, sx: 0.25, sy: 0.25, w: 1280, h: 720 });

  var numUrls = appURLs.length;
  for (var i = 0; i < numUrls; i++) {
    var appUrl = basePackageUri + "/" + appURLs[i];
    if (appURLs[i] == "picturepile2.js")
      appUrl = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/picturepile2.js";
    var c = scene.create({
      t: "scene", url: appUrl, parent: apps,
      w: childAppWidth, h: childAppHeight, clip: true
    });

    c.on("onMouseDown", function (e) {
      var c = e.target;
      if (e.flags & 16) {
        // ctrl-mousedown
        c.cx = c.w / 2;
        c.cy = c.h / 2;

        // rewind if cancelled; reset to 0 when complete
        // ready for the next one
        c.animateTo({ r: 360 }, 3, scene.animation.TWEEN_STOP, scene.animation.OPTION_REWIND).then(function (o) {
          o.r = 0;
        });

        // TODO this should work too... see what's wrong
        //c.animateTo({r: 360}, 3, scene.animation.TWEEN_STOP).then(o=>{o.r=0}).catch(o=>{o.r=0});
      }
      c.focus = true;
      select.animateTo({ x: (c.x - childPad) * 0.25, y: (c.y - childPad) * 0.25 }, 0.3, scene.animation.TWEEN_STOP, scene.animation.OPTION_LOOP, 1);
    });

    if (i == 0) c.focus = true;
  }

  var url = basePackageUri + "/images/select.png";
  select = scene.create({ t: "image9",
    parent: root, url: url, insetLeft: 16, insetTop: 16, insetRight: 16, insetBottom: 16,
    w: selectWidth * 0.25, h: selectHeight * 0.25, x: 0, y: 0, interactive: false
  });
  select.ready.then(function () {
    select.w = selectWidth * 0.25;
    select.h = selectHeight * 0.25;
  });

  scene.root.on('onKeyDown', function (e) {
    if (e.keyCode == 32) {
      root.painting = !root.painting;
    }
  });

  function positionApps() {
    var numApps = apps.numChildren;
    for (var i = 0; i < numApps; i++) {
      var c = apps.children[i];
      c.animateTo({
        x: i % childAcross * (childAppWidth + childPad) + childPad,
        y: Math.floor(i / childAcross) * (childAppHeight + childPad) + childPad
      }, 0.3, scene.animation.TWEEN_STOP, scene.animation.OPTION_LOOP, 1);
    }
  }

  function updateSize(w, h) {
    bgShade.w = w;
    bgShade.h = h;
    root.w = w;
    root.h = h;
    childAcross = Math.floor(w / ((childAppWidth + childPad) * 0.25));
    if (childAcross < 1) childAcross = 1;
    positionApps();
  }

  scene.on("onResize", function (e) {
    updateSize(e.w, e.h);
  });
  updateSize(scene.getWidth(), scene.getHeight());
}).catch(function importFailed(err) {
  console.error("Import for gallery.js failed: " + err);
});
