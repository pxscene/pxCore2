
px.import({ scene: 'px:scene.1.js' }).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;

  var myStretch = scene.stretch.STRETCH;

  print("about.js start")
                                           
  var LIGHT_BLUE  = 0x2B9CD8ff;
  var DARK_GRAY   = 0xDADADAff;
  var LIGHT_GRAY  = 0xBABABAff;
  var LIGHT_GRAY2 = 0xB0B0B0ff;
                                           
  var autoDismiss  = null;
  var textRows     = {};
  var textRowBG    = null;

  var label_w   = 175;
  var label_h   = 26;
  var value_w   = 225;
  var value_h   = 26;

  var max_w     = label_w + value_w;
  var max_h     = 175;
  var title_dy  = 30;

  var logo_url  = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/images/pxscene.png";
                                           
  var fontRes   = scene.create({ t: "fontResource",  url: "FreeSans.ttf" });
  var panel     = scene.create({ t: "object",  parent: root, x: 40, y: -720, w: (max_w + 4), h: max_h, a: 0.0 });
  var panel_bg  = scene.create({ t: "rect",    parent: panel, fillColor: LIGHT_GRAY2, w: panel.w + 16, h: panel.h + 8, x: -8, y: -8});
  var rows      = scene.create({ t: "object",  parent: panel_bg, a: 1.0 });

  var logo      = scene.create({ t: "image",   parent: panel, url: logo_url, x: 0, y: 5 });

  var dismissTXT = scene.create({ t: "textBox", parent: panel, textColor: 0x000000ff,
                            w: max_w, h: 20, x: 8, y: 0,
                            font: fontRes, pixelSize: 12, wordWrap: true,
                            text: "Press SPACE to dismiss",
                            alignHorizontal: scene.alignHorizontal.CENTER,
                            alignVertical: scene.alignVertical.CENTER
  })

  logo.ready.then(
                    function(o) { },
                    function(o) { max_h -= 170; title_dy = 10; }
                  );

  var textPts   = 16;
  var titlePts  = 28;
                                           
  var title     = null;
  var title_bg  = scene.create({ t: "rect", parent: panel, fillColor: LIGHT_BLUE, w: panel.w, h: 26, x: 0, y: 0});

  //##################################################################################################################################

  function addRow(pp, key, label, value)
  {
    var  i = rows.children.length;
    var ix = 10;             // text insets for textbox
    var lx = 0;              // label x
    var vx = (label_w + 4);  // value x
    var py = (label_h * i) + title.h + title.y + 5;

    var row = scene.create({ t: "object",  parent: pp, x: 8});
                                           
    var labelRect  = scene.create({ t: "rect",    parent: row, fillColor: (i%2) ? DARK_GRAY : LIGHT_GRAY,
                                    w: label_w, h: label_h + 4, x: lx, y: py });

    var labelTxt   = scene.create({ t: "textBox", parent: labelRect, textColor: 0x000000ff,
                                    w: label_w, h: label_h,     x: 0, y: 0,
                                    font: fontRes, pixelSize: textPts, wordWrap: true,
                                    xStartPos: ix, xStopPos: ix,
                                    text: label, // ### LABEL
                                    alignHorizontal: scene.alignHorizontal.LEFT,
                                    alignVertical:   scene.alignVertical.CENTER})

     var valueRect = scene.create({ t: "rect",    parent: row, fillColor: (i%2) ? DARK_GRAY : LIGHT_GRAY,
                                    w: value_w, h: value_h + 4, x: vx, y: py });

     var valueTxt  = scene.create({ t: "textBox", parent: valueRect, textColor: 0x000000ff,
                                    w: value_w, h: value_h,     x: 0, y: 0,
                                    font: fontRes, pixelSize: textPts, wordWrap: true,
                                    xStartPos: ix, xStopPos: ix,
                                    text: value, // ### VALUE
                                    alignHorizontal: scene.alignHorizontal.LEFT,
                                    alignVertical:   scene.alignVertical.CENTER})
    textRows[key] = valueTxt;

    max_h += (value_h + 4);
  }

  function createPanel()
  {
    var gfx = parseInt( parseInt( scene.info.gfxmemory ) / 1024);

    addRow(rows, "InfoBuildVersion", "Version: ", scene.info.version);
    addRow(rows, "InfoJsEngine", "Engine: ", scene.info.engine);
    addRow(rows, "InfoBuildDate",    "Build Date: ",      scene.info.build.date.replace(/"/g, '') ); // global RegEx
    addRow(rows, "InfoBuildTime",    "Build Time: ",      scene.info.build.time.replace(/"/g, '') ); // global RegEx
    addRow(rows, "InfoGfxMemory",    "Base GFX memory: ", gfx.toLocaleString()  + " KB");
  }

  function updateSize(w,h)
  {
    if(panel.y < 0) // offscreen
    {
        logo.x  = (panel.w - logo.resource.w)/2;

        panel.x = (w - panel.w)/2; // center horizontally in parent
        panel.a = 1.0;
                                                                                        
        showPanel(1500);
    }
  }

  function showPanel(delay_ms)
  {
    if(autoDismiss === null)
    {
      var cy = (scene.h - panel.h)/2;  // offscreen top
           
      panel.x = (scene.w - panel.w)/2; // center horizontally in parent
      panel.animateTo({ y: cy }, 1.25, scene.animation.EASE_OUT_ELASTIC,
                                       scene.animation.OPTION_FASTFORWARD, 1);
             
    //  autoDismiss = setTimeout(function hideMe(){ hidePanel() }, delay_ms);
    }
  }

  function hidePanel()
  {
    if (autoDismiss != null) {
        clearTimeout(autoDismiss);
    }

    autoDismiss = null;

    panel.animateTo({ y: scene.h + panel.h }, 1.0,  // offscreen bottom
        scene.animation.EASE_IN_ELASTIC,
        scene.animation.OPTION_FASTFORWARD, 1);
  }

  scene.on("onResize",  function(e) { updateSize(e.w,e.h); });
  //panel.on("onMouseUp", function(e) { console.log("PANEL onMouseUp"); hidePanel() } );
  panel_bg.on("onFocus",   function(e) { /*showPanel(5000);*/ } );
  panel_bg.on("onKeyDown", function (e) { if (e.keyCode) { hidePanel(); } });

  Promise.all([fontRes.ready, logo.ready, title_bg.ready, panel.ready]).catch( function (err)
  {
      console.log(">>> Loading Assets ... err = " + err);

  }).then( function() 
  {
    title = scene.create({ t: "text", text: "About pxscene", font: fontRes, parent: panel, pixelSize: titlePts, textColor: 0xFFFFFFff });
        
    title.ready.then( function ()
    {
        var titleM = fontRes.measureText(titlePts, title.text);

        title.x    = (panel.w - titleM.w)/2;
        title.y    = logo.y + logo.resource.h + title_dy;
        title.h    = titleM.h;
        title_bg.y = title.y;
        title_bg.h = title.h;

        max_h += title.h;
        max_h += dismissTXT.h + 15;

        createPanel();

        dismissTXT.y = max_h - dismissTXT.h - 10;
          
        panel.h    = max_h;
        panel_bg.h = max_h;
                   
        updateSize(scene.w, scene.h);

        panel_bg.focus = true;
    });
  }); // PROMISE.ALL

}).catch( function importFailed(err){
  console.error("Import failed for about.js: " + err);
});
