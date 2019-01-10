
px.import("px:scene.1.js").then( function ready(scene)
{
  var path    = px.getPackageBaseFilePath() + "/";
  var root    = scene.root;

  var bgUrl   = path + "images/cork.png";
  var bg      = scene.create({t:"image", url:bgUrl, parent:root, stretchX:2, stretchY:2});

  var fontRes = scene.create({t:"fontResource", url: "FreeSans.ttf"});
  var image   = scene.create({t:"image",        url: path + "images/rgb.png", parent:bg, stretchX:1, stretchY:1, a: 0});


  var alphaText = []; // Text
  var alphaImage = []; // Textures
  var alphaRect = [];


  var plain_text;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  var textBox1 = scene.create({t: "textBox" , parent: image, w: 100, h: 50, a: 0,
                            text: "FADES",
                       textColor: 0xFFFFFFff,
                            font: fontRes,
                       pixelSize: 40,
                 alignHorizontal: scene.alignHorizontal.CENTER,
                   alignVertical: scene.alignVertical.BOTTOM,
                               });

  var textBox2 = scene.create({t: "textBox" , parent: bg, w: 400, h: 100, a: 0,
                            text: "STATIC",
                       textColor: 0xFFFFFFff,
                            font: fontRes,
                       pixelSize: 40,
                 alignHorizontal: scene.alignHorizontal.CENTER,
                   alignVertical: scene.alignVertical.BOTTOM,
                               });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  var redBox;  // LHS
  var rhsBox;   // RHS container

  rhsBox = scene.create({t: "object", parent: bg, a: 0.0});

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function updateSize(w, h)
  {
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
    image.a = 1.0;

    // TEXTBOX 1
    textBox1.w = image.w
    textBox1.h = rh;
    textBox1.y = 50;
    textBox1.a = 1.0;

    // TEXTBOX 2
    textBox2.x = (w - textBox2.w)/2;
    textBox2.y = 50;
    textBox2.a = 1.0;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function createAlphaObjects(w,h)
  {
    var tx  = 250;
    var ty  = 10;
    var pts = 25;

    plain_text = scene.create({t:"text", text:"Plain Blue Text (FADES) - Below are TextBoxes",
                            font:fontRes, parent: bg, pixelSize: 40 , textColor:0x0000FFff,x: tx, y: ty});

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // ALPHA TEXT
    //
    tx = 30;
    ty = 60;

    redBox = scene.create({t: "rect", parent: bg, x: 15, y: ty,  w: 200, h: 350,
                                  fillColor:0xFF0000FF, lineWidth: 5, lineColor:0x000000ff  });

    redBox.a = 0.0;

    ty = 10;

    for(i = 0; i <= 10; i++)
    {
      alphaText[i] = scene.create({t:"text", text:"Alpha = "+ Math.round(0.1 * i * 100) +"%", a: 0.1 * i,
                             font:fontRes, parent: redBox, pixelSize: pts, textColor:0xFFFFFFff,x: tx, y: ty});
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

    for(i = 0; i <= 10; i++)
    {
      alphaRect[i] = scene.create({t: "rect", parent: rhsBox, x: tx, y: ty,  w: ww, h: ww, a: 0.1 * i,
                                  fillColor:0xFF0000ff, lineWidth: lw, lineColor:0x000000ff  });

      ty += alphaRect[i].h + 10;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // ALPHA TEXTURE
    //
    tx = w  - 120;
    ty = 50 - ww;

    for(i = 0; i <= 10; i++)
    {
      alphaImage[i] = scene.create({t:"image", parent: rhsBox, x: tx, y: ty,  w: ww, h: ww, a: 0.1 * i,
                                url: path + "images/rgb2.png", stretchX:1, stretchY:1});

      ty += alphaImage[i].h + 10;
    }
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function animate(t)
  {
     plain_text.animateTo({a:0.0}, t, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, scene.animation.COUNT_FOREVER);
          image.animateTo({a:0.0}, t, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, scene.animation.COUNT_FOREVER);
     //  textBox2.animateTo({a:0.0}, t, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, scene.animation.COUNT_FOREVER);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  createAlphaObjects(1280,720);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  scene.on("onResize", function(e){ updateSize(e.w, e.h); });

  Promise.all([image.ready, bg.ready]).then(function(success, failure)
  {
    updateSize(scene.w, scene.h);

    redBox.a = 1.0;
    rhsBox.a = 1.0

    animate(2);
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}).catch( function importFailed(err)
{
  console.error("Import failed for test_textBox.js: " + err)
});




