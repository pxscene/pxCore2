
px.import("px:scene.1.js").then( function ready(scene)
{
  var path    = px.getPackageBaseFilePath() + "/";
  var root    = scene.root;

  var bgUrl   = path + "images/cork.png";
  var bg      = scene.create({t:"image", url:bgUrl, parent:root, stretchX:2, stretchY:2});

  var fontRes = scene.create({t:"fontResource", url: "FreeSans.ttf"});
  var image   = scene.create({t:"image",        url: path + "images/rgb.png", parent:bg, stretchX:1, stretchY:1});

  var textBox1;
  var textBox2;
  var plain_text;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function updateSize(w, h)
  {
//    console.log("updateSize() - WxH: "+w+" x " +h);

    bg.w = w;
    bg.h = h;

    var rw = image.resource.w;
    var rh = image.resource.h;

    // JPG - RGB + TEXT
    //
    image.w = rw;
    image.h = rh;

    image.x = (w - image.w)/2;
    image.y = 200;

    textBox1 = scene.create({t: "textBox" , parent: image, w: image.w, h: rh,
                          text: "CHILD123",
                     textColor: 0xFFFFFFff,
                          font: fontRes,
                     pixelSize: 40,
               alignHorizontal: scene.alignHorizontal.CENTER,
                 alignVertical: scene.alignVertical.BOTTOM,
                             });

    textBox1.y = 50;

    textBox2 = scene.create({t: "textBox" , parent: bg, w: 400, h: 100,
                          text: "ROOT",
                     textColor: 0xFFFFFFff,
                          font: fontRes,
                     pixelSize: 40,
               alignHorizontal: scene.alignHorizontal.CENTER,
                 alignVertical: scene.alignVertical.BOTTOM,
                             });

    textBox2.x = (w - textBox2.w)/2;
    textBox2.y = 50;

    var tx  = 250;
    var ty  = 10;
    var pts = 25;

    plain_text = scene.create({t:"text",text:"This is Plain Text - Below are TextBoxes",
                            font:fontRes, parent: bg, pixelSize: 40 , textColor:0xFFFFFF88,x: tx, y: ty});

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // ALPHA TEXT
    //
    tx = 30;
    ty = 60;

    var rect = scene.create({t: "rect", parent: bg, x: 15, y: ty,  w: 200, h: 350,
                                  fillColor:0xFF0000FF, lineWidth: 5, lineColor:0x000000FF  });

    var alphatext = [];

    for(i = 0; i <= 10; i++)
    {
      alphatext[i] = scene.create({t:"text",text:"Alpha = "+ Math.round(0.1 * i * 100) +"%", a: 0.1 * i,
                             font:fontRes, parent: bg, pixelSize: pts, textColor:0xFFFFFF88,x: tx, y: ty});
      ty += pts + 5;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // ALPHA RECT
    //

    var ww = pts + 20;
    var lw = 4;

    tx = w  - 60;
    ty = 50 - ww;

    var alpharect = [];

    for(i = 0; i <= 10; i++)
    {
      alpharect[i] = scene.create({t: "rect", parent: bg, x: tx, y: ty,  w: ww, h: ww, a: 0.1 * i,
                                  fillColor:0xFF0000FF, lineWidth: lw, lineColor:0x000000FF  });

      ty += alpharect[i].h + 10;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // ALPHA TEXTURE
    //
    tx = w  - 120;
    ty = 50 - ww;

    var alphatex = [];

    for(i = 0; i <= 10; i++)
    {
      alphatex[i] = scene.create({t:"image", parent: bg, x: tx, y: ty,  w: ww, h: ww, a: 0.1 * i,
                                url: path + "images/rgb.png", stretchX:1, stretchY:1});

      ty += alphatex[i].h + 10;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    var t = 2;

     plain_text.animateTo({a:0.0}, t, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, scene.animation.COUNT_FOREVER);
          image.animateTo({a:0.0}, t, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, scene.animation.COUNT_FOREVER);
       textBox2.animateTo({a:0.0}, t, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, scene.animation.COUNT_FOREVER);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  scene.on("onResize", function(e){ updateSize(e.w, e.h); });

  Promise.all([image.ready, bg.ready]).then(function(success, failure)
  {
    updateSize(scene.w, scene.h);
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}).catch( function importFailed(err)
{
  console.error("Import failed for test_textBox.js: " + err)
});




