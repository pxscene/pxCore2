/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

"use strict";


px.configImport({
  "root:"       : px.getPackageBaseFilePath(),
  "components:" : px.getPackageBaseFilePath() + "/components/"
});

px.import({       scene: 'px:scene.1.js',
                   keys: 'px:tools.keys.js',
                   apps: 'appconfig.js',
             scrollable: 'components:scrollable.js',
//              sceneview: 'components:sceneview.js'
}).then( function importsAreReady(imports)
{
  module.exports.wantsClearscreen = function()
  {
    return false; // skip clearscreen by framework... using opaque bg.
  };

  var scene = imports.scene;
  var root  = imports.scene.root;
  var keys  = imports.keys;
  var apps  = imports.apps;

  var Scrollable = imports.scrollable.Scrollable;
  // var SceneView  = imports.sceneview;

  var AR = scene.w / scene.h;
  var pos_x  = 0, pos_y  = 0;

  const SHADOW_COLOR = "#444";
  const STRETCH      = scene.stretch.STRETCH;
  const EASING       = scene.animation.EASE_IN_QUAD;
  const LINEAR       = scene.animation.TWEEN_LINEAR;
  const FFWD         = scene.animation.OPTION_FASTFORWARD;
  const HL           = scene.alignHorizontal.LEFT;
  const HC           = scene.alignHorizontal.CENTER;
  const VC           = scene.alignVertical.CENTER;

  var appTiles = [];
  var appURLs  = apps.rdkItems;

  var SS       = 1.09;
  var SA       = 0.60;

  // Animations
  var frameFocusAnim  = null;
  var titleFocusAnim  = null;
  var shadowFocusAnim = null;

  var frameBlurAnim   = null;
  var titleBlurAnim   = null;
  var shadowBlurAnim  = null;

  // Components
  var container       = null;
  // var shade           = null;
  var placeholderRes  = null;
  var roundedRectRes  = null;
  var scrollable      = null;
  // var sceneview       = null;
  var selected        = null;

  var cols = 3;
  var rows = Math.ceil(appURLs.length / cols);

  var screen_cols = cols;
  var screen_rows = 3;

  var last_page   = 0;
  var this_page   = 0;

  var tileW = Math.round((scene.w - 10)/screen_cols); // 10 px for scroll bar
  var tileH = Math.round(tileW / AR);

  var borderW = 25; // .. 10 px border
  var frameW  = tileW - (2 * borderW); // .. less border
  var frameH  = Math.round(frameW / AR);

  let font    = scene.create({ t: "fontResource",  url: "FreeSans.ttf" });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function createScene(w, h)
  {
    var placeholderSVG = px.getPackageBaseFilePath() + "/images/placeholder373x210r15.svg";

    ////////////////////
    //
    // TODO:  Use 'inline' SVG in the furure.
    //
    // var placeholderSVG = 'data:image/svg, <svg width="'+frameW+'" height="'+frameH+'"><linearGradient id="PHL" gradientUnits="userSpaceOnUse" x1="32.0229" y1="142.4004" x2="171.9757" y2="61.5986" gradientTransform="matrix(-4.371139e-08 -1 1 -4.371139e-08 -2 201.9995)"><stop  offset="0.0056" style="stop-color:#375270"/><stop  offset="1.0000" style="stop-color:#7DA7D9"/></linearGradient><rect x="0" y="0" fill="url(#PHL)" width="'+frameW+'" height="'+frameH+'"/></svg>';
    //
    // let rx = 15, ry = 15;
    // var rounded = (' rx="'+rx+'" ry="'+ry );
    //
    // let fill = SHADOW_COLOR;
    // var roundedRectSVG =  'data:image/svg,<svg width="'+frameW+'px" height="'+frameH+'px">'+
    //                       '<rect fill="'+fill+'" stroke="none" stroke-width="0"'+
    //                       rounded+' "width="'+frameW+'" height="'+frameH+'"/></svg>'
    //
    ////////////////////
    if(scrollable !== null) { scrollable = null; }
    // if(sceneview  !== null) { sceneview  = null; }
    // if(shade      !== null) { shade.remove(); shade = null };

    var roundedRectSVG = px.getPackageBaseFilePath() + "/images/rrect373x210r15.svg";
    if(container == null)
    {
        container = scene.create({ t: "object", parent: root, w: w, h: h, focus: true, draw: true, clip: true });
    }
    else
    {
      container.w = w;
      container.h = h;
    }

    // if(shade == null)
    // {
    //   shade = scene.create({ t: "rect", parent: container, w: w, h: h, fillColor: "#000", a: 0 });
    // }
    // else
    // {
    //   shade.w = w
    //   shade.h = h
    // }

    if(placeholderRes == null)
    {
      placeholderRes = scene.create({ t: "imageResource", url: placeholderSVG, w: frameW, h: frameH });
    }
    if(roundedRectRes == null)
    {
      roundedRectRes = scene.create({ t: "imageResource", url: roundedRectSVG, w: frameW, h: frameH });
    }

    let assets = [container.ready, /* shade.ready, */ placeholderRes.ready, roundedRectRes.ready ];

    return Promise.all(assets).then( (o) =>
    {
      // shade.moveForward();

      container.on('onPreKeyDown', onPreKeyDown);

      scrollable = new Scrollable(scene, container, {rowScrollHeight: tileH});
      // sceneview  = new  SceneView(scene, container);

      return Promise.all([scrollable.ready /*, sceneview.ready*/]).then( (o) =>
      {
        scrollable.root.h = tileH * Math.ceil(appURLs.length / cols);
        scrollable.update();

        appTiles.map( (o) =>
        {
          o.remove(); // tidy up ... if needed
        });

        appTiles = [];
        createTiles(appURLs);
      });
    });
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function createTiles(urls)
  {
    const tw = tileW - borderW;

    urls.map( (app, n) =>
    {
      if(app.type == "Spark")
      {
        let appUrl = app.attributes.url;
        let imgUrl = app.image;

        var pos    = ind2sub(n);

        var tileX  = (pos.x * tileW);
        var tileY  = (pos.y * tileH);
        var titleY = (frameH - 26);

        var pp = scrollable.root;

        let tile   = scene.create({ t: "rect",   parent: pp,    x: tileX,   y: tileY,     w: tileW,  h: tileH,  interactive: false, fillColor:  "#000", lineColor: "#000", lineWidth: 0  });
        let frame  = scene.create({ t: "object", parent: tile,  x: borderW, y: borderW/2, w: frameW, h: frameH, interactive: true });

        tile.id = app.title;

        ////////////////////
        //
        // TODO:  Use 'inline' SVG in the furure.
        //
        // let shadow   = scene.create({ t: "image",   parent: frame,    x: 0,       y: 0,         w: frameW, h: frameH, interactive: false, resource: roundedRectRes });
        //
        ////////////////////

        var ii = 25; //48
        var shadowUrl = px.getPackageBaseFilePath() + "/images/BlurRect2.png";
        var insets    = { insetTop: ii, insetBottom: ii, insetLeft: ii, insetRight: ii };
        let shadow    = scene.create( Object.assign(insets, {t:"image9", x:0, y:0, w:frameW*1.08, h:frameH*1.15, url:shadowUrl, interactive: false,  parent: frame, a: 0 }) );

       // shadow.ready.then( o => o.moveForward() );

        ////////////////////

        let image    = scene.create({ t: "object",  parent: frame,    x: 0, y: 0,      w: frameW, h: frameH, interactive: false, fillColor: "#000", lineColor: "#000", lineWidth: 0  });
        let img      = scene.create({ t: "image",   parent: image,    x: 0, y: 0,      w: frameW, h: frameH, interactive: false, stretchX: STRETCH, stretchY: STRETCH,  resource: placeholderRes});
        let msk      = scene.create({ t: "image",   parent: image,    x: 0, y: 0,      w: frameW, h: frameH, interactive: false, resource: roundedRectRes, mask: true, draw: false });

        let titleBox = scene.create({ t: "rect",    parent: image,    x: 0, y: titleY, w: frameW, h: 26,     interactive: false, cx: tw, cy: 13, fillColor: "#222", a: 0  });
        let title    = scene.create({ t: "textBox", parent: titleBox, x: 0, y: 0,      w: frameW, h: 26,     interactive: false, cx: tw, cy: 13, textColor: "#fff", pixelSize: 16,
                                                                                                        text: app.title, font: font, alignHorizontal: HC, alignVertical: VC});
        // Why does this not work ?
        //
        // Promise.all([image.ready, img.ready, msk.ready]).then((o) =>
        // {
        //   image.painting = false;
        // });

        image.cx    = image.w/2;
        image.cy    = image.h/2;

        tile.frame  = image;
        tile.title  = titleBox;
        tile.shadow = shadow;
        tile.info   = app;

        appTiles.push(tile);

          tile.on("onFocus",     function(o) { doFocus(o.target);     });
          tile.on("onBlur",      function(o) { doBlur(o.target);      });
         frame.on("onMouseDown", function(o) { doMouseDown(o.target); });
  //        imgUrl = imgUrl.replace("https", "http");

        scene.create({ t: "imageResource", url: imgUrl }).ready
        .then( function(o)
        {
          img.resource = o;
          image.painting = false;
        })
        .catch( (err) => {
          console.error("Exception: >> " + imgUrl + " << load failed.  Err:" + err);
        });

        // console.log(" APP["+n+"]   Title: " + app.title);
      }
    });//map

    var tiles_ready = appTiles.map( o => o.ready); // collect promises

    return Promise.all( tiles_ready ).then( function(o)
    {
        pos_x = 1; pos_y = 1; // middle-middle
        moveTo(pos_x, pos_y); // Initial position
    });
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function doFocus(o)
  {
//    console.log("doFocus()  >>> " + o.info.title);

    let frame  = o.frame;
    let shadow = o.shadow;
    let title  = o.title;

    const TT = 0.25;

    frameFocusAnim  =  frame.animate({ sx: SS, sy: SS,        }, TT, EASING, FFWD, 1);
    shadowFocusAnim = shadow.animate({ sx: SS, sy: SS, a: SA  }, TT, EASING, FFWD, 1);
    titleFocusAnim  =  title.animate({                 a: 0.8 }, TT, EASING, FFWD, 1);

    return Promise.all([frameFocusAnim.done, shadowFocusAnim.done]);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function doBlur(o)
  {
//    console.log("doBlur()  >>> " + o.info.title);

    let frame  = o.frame;
    let shadow = o.shadow;
    let title  = o.title;

    const TT = 0.125;
    const ar = 1/AR;

    frameBlurAnim  =  frame.animate({ sx: 1.0, sy: 1.0,      },   TT, EASING, FFWD, 1);
    shadowBlurAnim = shadow.animate({ sx: 1.0, sy: 1.0, a: 0 },   TT, EASING, FFWD, 1);
    titleBlurAnim  =  title.animate({ a: 0.0                 }, 2*TT, EASING, FFWD, 1);

    return Promise.all([frameBlurAnim.done, shadowBlurAnim.done]);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function doMouseDown(o)
  {
    if(o.parent.focus == false)
    {
      var index = appTiles.indexOf(o.parent)
      var pos   = ind2sub(index);

      pos_x = pos.x;
      pos_y = pos.y;

      moveTo(pos_x, pos_y);
    }
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function onPreKeyDown(e)
  {
    if ( keys.is_CTRL(e.flags) && (e.keyCode == keys.L) ) // LAST
    {
        hideInfo();

        selected.focus = true; // restore

        e.stopPropagation();
    }
    else
    if ( keys.is_CTRL_ALT(e.flags) && (e.keyCode == keys.ENTER) ) // ENTER
    {
        hideInfo();

        selected.focus = true; // restore

        e.stopPropagation();
    }
    else
    if(selected.focus == true)
    {
      var dx = 0, dy = 0;
      var target = e.target;

      switch(e.keyCode)
      {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.ENTER: showScene(target); e.stopPropagation(); return; // SELECT App
        case keys.UP:    dx =  0; dy = -1;  e.stopPropagation(); break;  //    UP a row
        case keys.DOWN:  dx =  0; dy =  1;  e.stopPropagation(); break;  //  DOWN a row
        case keys.LEFT:  dx = -1; dy =  0;  e.stopPropagation(); break;  //  LEFT a square
        case keys.RIGHT: dx =  1; dy =  0;  e.stopPropagation(); break;  // RIGHT a square
        default: return;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      }//SWITCH

      if( dx != 0 || dy !=0 ) // Any ARROW key change ?
      {
        pos_x += dx;  pos_y += dy;

        if(pos_x < 0)     pos_x = cols - 1; // wrap
        if(pos_y < 0)     pos_y = rows - 1; // wrap
        if(pos_x >= cols) pos_x = 0;        // wrap
        if(pos_y >= rows) pos_y = 0;        // wrap

        var index = 0;
        var maxIndex = (appTiles.length - 1);

        if ( (index = sub2ind(pos_x, pos_y)) > maxIndex )  // previous row ? ... landed on an empty spot on this row.
        {
          if(Math.abs(dx))
          {
            if(dx > 0)  pos_x = 0;
            if(dx < 0)  pos_x = cols - 2; // 2nd last row index
          }
          else
          if(Math.abs(dy))
          {
            if(dy > 0)  pos_y = 0;
            if(dy < 0)  pos_y = rows - 2; // 2nd last row index
          }
        }

        moveTo(pos_x, pos_y);

        this_page = Math.round(pos_y / screen_rows);

        if(last_page != this_page)
        {
          var e = ( this_page - last_page ) > 0 ? {keyCode: keys.PAGEDOWN} : {keyCode: keys.PAGEUP};

          scrollable.onKeyDown(e); // SEND fake KEY

          last_page = this_page;
        }
      }
    }
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  var showSceneAlready = false;
  var appFrame = null;
  var appScene = null;

  function showScene(target)
  {
    if(showSceneAlready) return;
    showSceneAlready = true;

    var info = target.info;

    appFrame = scene.create({ t: "rect", parent: container, fillColor: "#000",  x: 0,  y: 0,
                      w: scene.w, h: scene.h, sx: 0, sy: 0, a: 0 });

    appScene = scene.create({ t: "scene", parent: appFrame, url: info.attributes.url,
                       w: scene.w, h: scene.h, clip: true });

    Promise.all([ appFrame.ready, appScene.ready ])
    .then( function()
    {
      appFrame.cx = appFrame.w/2;
      appFrame.cy = appFrame.h/2;
      appScene.cx = appScene.w/2;
      appScene.cy = appScene.h/2;

      appScene.focus = true;

      appFrame.animateTo({ sx: 1, sy: 1, a: 1 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
    })
    // shade.animateTo({ a: 0.5 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);

    // sceneview.showInfo(info)
    // .then( (o) =>
    // {
    //   sceneview.focus = true;
    // });
  }

  function hideInfo()
  {
    // shade.animateTo({ a: 0.0 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);

    appFrame.animateTo({ sx: 0, sy: 0, a: 0 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1)
    .then( o =>
    {
      appFrame.remove()
      appFrame = null;

      appScene.remove()
      appScene = null;

      showSceneAlready = false;
    });


    // sceneview.hideInfo()
    // .then( (o) =>
    // {
    //   showSceneAlready = false;
    // });
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function updateSize(w, h)
  {
    AR = w/h;

    tileW  = Math.round((w - 10)/screen_cols); // 10 px for scroll bar
    tileH  = Math.round(tileW / AR);

    frameW = tileW - (2 * borderW); // .. less border
    frameH = Math.round(frameW / AR);

    if(showSceneAlready)
    {
      showSceneAlready = false;
      sceneview.animate({ a: 0.0 }, 0.25, LINEAR, FFWD, 1);
    }

    createScene(w, h)

    // scrollable.root.h = hh * Math.ceil(appURLs.length / cols); //rows;
    // scrollable.update();
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  function ind2sub(idx)
  {
    var x = (idx  % cols);
    var y = ((idx - x)/cols) % rows;

    return {x: x, y: y}
  }

  function sub2ind(x,y)
  {
    return (y * cols) + x;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function moveTo(dx, dy)
  {
    var index = sub2ind(dx, dy);

    try
    {
      selected       = appTiles[index];
      selected.focus = true;
    }
    catch(e)
    {
      console.error("Exception:  Bad index ?? index: " + index)
    }
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  scene.on("onResize", function (e)
  {
    updateSize(e.w, e.h);
  });

  updateSize(scene.w, scene.h); // INITIAL

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}).catch(function importFailed(err) {
  console.error("Import for showcaseApp.js failed: " + err);
});

