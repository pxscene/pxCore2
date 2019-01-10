
px.import("px:scene.1.js").then( function ready(scene)
{
  var path    = px.getPackageBaseFilePath() + "/";
  var root    = scene.root;

  var useJPG  = true;
  var usePNG  = true;

  var fontRes = scene.create({t:"fontResource",url:"FreeSans.ttf"});

  var rjUrl   = path + "images/red200x200.jpg";
  var gjUrl   = path + "images/green200x200.jpg";
  var bjUrl   = path + "images/blue200x200.jpg";

  var rj      = useJPG ? scene.create({t:"image", url:rjUrl,  parent:root}) : 0;
  var gj      = useJPG ? scene.create({t:"image", url:gjUrl,  parent:root}) : 0;
  var bj      = useJPG ? scene.create({t:"image", url:bjUrl,  parent:root}) : 0;

  var rpUrl   = path + "images/red200x200.png";
  var gpUrl   = path + "images/green200x200.png";
  var bpUrl   = path + "images/blue200x200.png";

  var rp      = usePNG ? scene.create({t:"image", url:rpUrl,  parent:root}) : 0;
  var gp      = usePNG ? scene.create({t:"image", url:gpUrl,  parent:root}) : 0;
  var bp      = usePNG ? scene.create({t:"image", url:bpUrl,  parent:root}) : 0;

  var rgbjUrl = path + "images/rgb.jpg";
  var rgbpUrl = path + "images/rgb.png";

  var rgbj    = useJPG ? scene.create({t:"image", url:rgbjUrl, parent:root, stretchX:1, stretchY:1}) : 0;
  var rgbp    = usePNG ? scene.create({t:"image", url:rgbpUrl, parent:root, stretchX:1, stretchY:1}) : 0;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function updateSize(w, h)
  {
    console.log("updateSize() - WxH: "+w+" x " +h);

    var pts = 40;

    var rw = useJPG ? rj.resource.w : usePNG ? rp.resource.w : 0;
    var rh = useJPG ? rj.resource.h : usePNG ? rp.resource.h : 0;

    var rows = (useJPG && usePNG) ? 2 : 1;

    // Spare room
    var cx = w - (rw * 4);
    var cy = h - (rh * rows);

    // Incremental - proportion of spare room
    var dx = (1.0 + (rw / cx)) * rw;
    var dy = (1.0 + (rh / cy)) * rh;

    var px = (cx - rw)/2;
    var py = ( (rows == 2) ? (cy - rh)/2 : (h - rh)/2);

    // console.log("updateSize() - px: "+ px + " py: " + py + "  cx: " + cx + " cy: "+ cy + "  dx: " + dx + " dy: " + dy + "  rw: " + rw + " rh: " + rh);

    if(useJPG)
    {
      // JPG - RED
      //
      rj.x = px; px += dx;
      rj.y = py;

      var boxRJ = scene.create({t:"textBox" , parent: rj, w: rw, h: rh,
              text:      "RED",
              textColor: 0xFFFFFFff,
              font:      fontRes,
              pixelSize: pts,
              alignHorizontal: scene.alignHorizontal.CENTER,
              alignVertical:   scene.alignVertical.CENTER,
              });

      // JPG - GREEN
      //
      gj.x = px; px += dx;
      gj.y = py;

      var boxGJ = scene.create({t:"textBox" , parent: gj, w: rw, h: rh,
              text:      "GREEN",
              textColor: 0xFFFFFFff,
              font:      fontRes,
              pixelSize: pts,
              alignHorizontal: scene.alignHorizontal.CENTER,
              alignVertical:   scene.alignVertical.CENTER,
              });

      // JPG - BLUE
      //
      bj.x = px; px += dx;
      bj.y = py;

      var boxGJ = scene.create({t:"textBox" , parent: bj, w: rw, h: rh,
              text:      "BLUE",
              textColor: 0xFFFFFFff,
              font:      fontRes,
              pixelSize: pts,
              alignHorizontal: scene.alignHorizontal.CENTER,
              alignVertical:   scene.alignVertical.CENTER,
              });

      // JPG - RGB + TEXT
      //
      rgbj.w = 150;
      rgbj.h = 150;

      rgbj.x = px + 10;
      rgbj.y = py;

      var txtJ = scene.create({t:"textBox" , parent: rgbj, w: rgbj.w, h: rh,
              text:      "JPG",
              textColor: 0xFFFFFFff,
              font:      fontRes,
              pixelSize: pts,
              alignHorizontal: scene.alignHorizontal.CENTER,
              alignVertical:   scene.alignVertical.BOTTOM,
              });

      txtJ.y = 10;

    } // JPG

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Next ROW
    if(useJPG && usePNG)
    {
      px = cx/2 - rw/2;
      py += dy;
    }
    // else
    // {
    //    px = cx/2 -  rw/2;
    // }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(usePNG)
    {
      // PNG - RED
      //
      rp.x = px; px += dx;
      rp.y = py;

      var boxRP = scene.create({t:"textBox" , parent: rp, w: rw, h: rh,
              text:      "RED",
              textColor: 0xFFFFFFff,
              font:      fontRes,
              pixelSize: pts,
              alignHorizontal: scene.alignHorizontal.CENTER,
              alignVertical:   scene.alignVertical.CENTER,
              });

      // PNG - GREEN
      //

      gp.x = px; px += dx;
      gp.y = py

      var boxGP = scene.create({t:"textBox" , parent: gp, w: rw, h: rh,
              text:      "GREEN",
              textColor: 0xFFFFFFff,
              font:      fontRes,
              pixelSize: pts,
              alignHorizontal: scene.alignHorizontal.CENTER,
              alignVertical:   scene.alignVertical.CENTER,
              });

      // PNG - BLUE
      //
      bp.x = px; px += dx;
      bp.y = py

      var boxBP = scene.create({t:"textBox" , parent: bp, w: rw, h: rh,
              text:      "BLUE",
              textColor: 0xFFFFFFff,
              font:      fontRes,
              pixelSize: pts,
              alignHorizontal: scene.alignHorizontal.CENTER,
              alignVertical:   scene.alignVertical.CENTER,
              });

      // PNG - RGB + TEXT
      //
      rgbp.w = 150;
      rgbp.h = 150;

      rgbp.x = px + 10;
      rgbp.y = py

      var txtP = scene.create({t:"textBox" , parent: rgbp, w: rgbp.w, h: rh,
                text:      "PNG",
                textColor: 0xFFFFFFff,
                font:      fontRes,
                pixelSize: pts,
                alignHorizontal: scene.alignHorizontal.CENTER,
                alignVertical:   scene.alignVertical.BOTTOM,
                });

      txtP.y = 10;
    }
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  scene.on("onResize", function(e){updateSize(e.w,e.h);});

  Promise.all([rj.ready, rp.ready]).then(function(success, failure)
  {
    updateSize(scene.w, scene.h);
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}).catch( function importFailed(err)
{
  console.error("Import failed for test_color.js: " + err)
});




