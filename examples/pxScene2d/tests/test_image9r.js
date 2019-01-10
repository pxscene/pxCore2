
px.import({ scene: 'px:scene.1.js',
             keys: 'px:tools.keys.js'
}).then( function importsAreReady(imports)
{
  var path        = px.getPackageBaseFilePath() + "/";

  var scene       = imports.scene;
  var keys        = imports.keys;
  var root        = scene.root;

  var bgUrl       = path + "images/cork.png";
  var bgShadowUrl = path + "images/radial_gradient.png";
  var ballUrl     = path + "images/spinningball2.png";

  var fontRes     = scene.create({t:"fontResource",url:"FreeSans.ttf"});

  var ii          = 9;
  var image9url   = path + "images/input2.png";

  var bg          = scene.create({t:"image", url:bgUrl,   parent:root, stretchX:2, stretchY:2});
  var ball        = scene.create({t:"image", url:ballUrl, parent:root});


  var sx = 1.0;
  var sy = 1.0;

  var dx = 75;
  var dy = 50;

  var tx = dx;
  var ty = dy + 150;

  var dw = 0;
  var dh = 0;

  var dr = 0;

  var iw = 30;
  var ih = 30;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function nextRow()
  {
    // Next Row
    dy += 200;
    ty  = dy + 125;

    // reset
    dx = 75;
    tx = 75;

    // reset
    iw = 30;
    ih = 30;
    dw = 0;
    dh = 0;
    dr = 0;

    sx = 1.0;
    sy = 1.0;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function nextColumn()
  {
    // Next Column
    dx += 200; //gap
    tx  =  dx; //gap

    // reset
    iw = 30;
    ih = 30;
    dw = 0;
    dh = 0;
    dr = 0;

    sx = 1.0;
    sy = 1.0;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if(true)
  {

  var useAlpha    = false;
  var useOutline  = true;
  var useTitles   = true;
  var useRotation = true;

  var count = 5;
  var pts   = 30;

  var images_text = [];
  var text_index = 0;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // IMAGE9 - Scaling using W, H...
  //
  var images91   = [];
  var promises91 = [];

  for(i = 0; i < count; i++)
  {
    images91[i] = scene.create({t: "image9", url: image9url, parent: bg, r: useRotation ? dr : 0, x: dx, y: dy,  w: iw + dw, h: ih + dh,
                       insetTop: ii, insetBottom: ii, insetLeft: ii, insetRight: ii });

    promises91[i] = images91[i].ready;

    dw += 15; dh += 15;  dx += (iw + dw);
  }

  if(useTitles)
  {
    images_text[text_index++] = scene.create({t:"text",text:"9 Slice (W x H - No Rotation)",
                                  font:fontRes, parent: bg, pixelSize: pts,textColor:0x000000ff,x: tx, y: ty});
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  nextColumn();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // IMAGE9 - Scaling using W, H... ROTATED
  //
  var images92   = [];
  var promises92 = [];

  for(i = 0; i < count; i++)
  {
    images92[i] = scene.create({t: "image9", url: image9url, parent: bg, r: useRotation ? dr : 0, x: dx, y: dy,  w: iw + dw, h: ih + dh,
                       insetTop: ii, insetBottom: ii, insetLeft: ii, insetRight: ii });

    promises92[i] = images92[i].ready;

    dr += 5; dw += 15; dh += 15;  dx += (iw + dw);
  }

  if(useTitles)
  {
    images_text[text_index++] = scene.create({t:"text",text:"9 Slice (W x H - With Rotation)",
                                  font:fontRes, parent: bg, pixelSize: pts,textColor:0x000000ff, x: tx, y: ty});
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  nextRow();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // IMAGE9 - Scaling using SX, SY...
  //

  var images93   = [];
  var promises93 = [];

  for(i = 0; i < count; i++)
  {
    images93[i] = scene.create({t: "image9", url: image9url, parent: bg, sx: sx, sy: sy, x: dx, y: dy,  w: iw, h: ih,
                      insetTop: ii, insetBottom: ii, insetLeft: ii, insetRight: ii });

    promises93[i] = images93[i].ready;

    sx += 0.5;  sy += 0.5;

    dw = (iw * sx);
    dh = (ih * sy);

    dx += dw;
  }

  if(useTitles)
  {
    images_text[text_index++] = scene.create({t:"text",text:"9 Slice (SX, SY - No Rotation)",
                                  font:fontRes, parent: bg, pixelSize: pts,textColor:0x000000ff, x: tx, y: ty});
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  nextColumn();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // IMAGE9 - Scaling using SX, SY... ROTATED
  //
  var images94   = [];
  var promises94 = [];

  for(i = 0; i < count; i++)
  {
    images94[i] = scene.create({t: "image9", url: image9url, parent: bg, sx: sx, sy: sy, r: useRotation ? dr : 0, x: dx, y: dy,  w: iw, h: ih,
                      insetTop: ii, insetBottom: ii, insetLeft: ii, insetRight: ii });

    promises94[i] = images94[i].ready;

    sx += 0.5;  sy += 0.5;

    dw = (iw * sx);
    dh = (ih * sy);

    dx += dw + 10;

    dr += 5;
  }

  if(useTitles)
  {
    images_text[text_index++] = scene.create({t:"text",text:"9 Slice (W x H - With Rotation)",
                                  font:fontRes, parent: bg, pixelSize: pts,textColor:0x000000ff, x: tx, y: ty});
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  nextRow();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // RECTANGLE - Scaling using W, H...
  //
  var images95   = [];
  var promises95 = [];

  for(i = 0; i < count; i++)
  {
    images95[i] = scene.create({t: "rect", parent: bg, r: dr, x: dx, y: dy,  w: iw + dw, h: ih + dh,
                                 fillColor:0xFF0000FF, lineWidth: useOutline ? 5 : 0, lineColor:0x000000FF  });

    promises95[i] = images95[i].ready;

    dw += 15; dh += 15;  dx += (iw + dw);
  }

  if(useTitles)
  {
    images_text[text_index++] = scene.create({t:"text",text:"Rectangle (W x H - No Rotation)",
                                  font:fontRes, parent: bg, pixelSize: pts,textColor:0x000000ff,x: tx, y: ty});
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  nextColumn();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // RECTANGLE - Scaling using W, H...
  //
  var images96   = [];
  var promises96 = [];

  for(i = 0; i < count; i++)
  {
    images96[i] = scene.create({t: "rect", parent: bg, r: useRotation ? dr : 0, x: dx, y: dy,  w: iw + dw, h: ih + dh,
                                 fillColor:0xFF0000FF, lineWidth: useOutline ? 5 : 0, lineColor:0x000000FF  });

    promises96[i] = images96[i].ready;

    dr += 5;
    dw += 15; dh += 15;  dx += (iw + dw);
  }

  if(useTitles)
  {
    images_text[text_index++] = scene.create({t:"text",text:"Rectangle (W x H - With Rotation)",
                                  font:fontRes, parent: bg, pixelSize: pts,textColor:0x000000ff,x: tx, y: ty});
  }

  ty += 60;
  tx = 255;
  var text_keys = scene.create({t:"text",text:"Use KEYS Toggle >>    (R)otation   Out(L)ine   (A)lpha",
                                font:fontRes, parent: bg, pixelSize: pts , textColor:0xFFFFFFff,x: tx, y: ty});

  ball.x = 1120;
  ball.y = 600;
  ball.a = 1.0;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}//false

//  var image9R = scene.create({t: "image9", url: image9url, parent: bg,  x: 500, y: 400,  w: 250, h: 250, r: 10,
//                      insetTop: ii, insetBottom: ii, insetLeft: ii, insetRight: ii });               deltas1();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function updateSize(w, h)
  {
    bg.w = w;
    bg.h = h;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  scene.root.on("onKeyDown", function(e)
  {
    var code = e.keyCode; var flags = e.flags;

    if(code == keys.L)
    {
      useOutline = !useOutline;

      for(i = 0; i < count; i++)
      {
        images92[i].lineWidth = useOutline ? 5 : 0;
        images94[i].lineWidth = useOutline ? 5 : 0;
        images96[i].lineWidth = useOutline ? 5 : 0;
      }
    }
    else
    if(code == keys.R)
    {
      useRotation = !useRotation;

      var r = 0;

      for(i = 0; i < count; i++)
      {
        images92[i].r = useRotation ? r : 0;
        images94[i].r = useRotation ? r : 0;
        images96[i].r = useRotation ? r : 0;

        r += 5;
      }
    }
    else
    if(code == keys.A)
    {
      useAlpha = !useAlpha;

      var aa = 0.5;

      for(i = 0; i < count; i++)
      {
        images92[i].a = useAlpha ? aa : 1.0;
        images94[i].a = useAlpha ? aa : 1.0;
        images96[i].a = useAlpha ? aa : 1.0;
      }

      ball.a      = useAlpha ? aa : 1.0;
      text_keys.a = useAlpha ? aa : 1.0;
    }
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  scene.on("onResize", function(e){ updateSize(e.w,e.h); });

  Promise.all([promises91[0].ready]).then(function(success, failure)
  {
    updateSize(scene.w, scene.h);
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}).catch( function importFailed(err)
{
  console.error("Import failed for test_image9r.js: " + err)
});




