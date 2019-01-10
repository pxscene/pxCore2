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
  "components:" : px.getPackageBaseFilePath() + "/components/"
});

px.import({       scene: 'px:scene.1.js',
                   keys: 'px:tools.keys.js',
                   apps: 'appconfig.js',
             scrollable: 'components:scrollable.js',
              sceneview: 'components:sceneview.js'
}).then( function importsAreReady(imports)
{
  // module.exports.wantsClearscreen = function()
  // {
  //   return false; // skip clearscreen by framework... using opaque bg.
  // };

  var scene = imports.scene;
  var root  = imports.scene.root;
  var keys  = imports.keys;
  var apps  = imports.apps;

  var Scrollable = imports.scrollable.Scrollable;
  var SceneView  = imports.sceneview;

  var AR = scene.w / scene.h;

  const SHADOW_COLOR = "#444";
  const STRETCH      = scene.stretch.STRETCH;
  const LINEAR       = scene.animation.TWEEN_LINEAR;
  const FFWD         = scene.animation.OPTION_FASTFORWARD;
  const HL           = scene.alignHorizontal.LEFT;
  const HC           = scene.alignHorizontal.CENTER;
  const VC           = scene.alignVertical.CENTER;

  var appTiles = [];
  var appURLs = apps.rdkItems;

  var cols = 3;
  var rows = Math.ceil(appURLs.length / cols);
  
  var screen_cols = cols;
  var screen_rows = 3;

  var last_page = 0;
  var this_page = 0;

  var tileW = Math.round((scene.w - 10)/screen_cols); // 10 px for scroll bar
  var tileH = Math.round(tileW / AR);

  var borderW = 25; // .. 10 px border

  var frameW = tileW - (2 * borderW); // .. less border
  var frameH = Math.round(frameW / AR);

  var SS       = 1.08;

  // Animations
  var frameFocusAnim  = null;
  var shadowFocusAnim = null;
  var titleFocusAnim  = null;
  var titleBlurAnim   = null;

  var frameBlurAnim   = null;
  var shadowBlurAnim  = null;

  var scrollable      = null;
  var sceneview       = null;
  var selected        = null;

  var pos_x  = 0;
  var pos_y  = 0;

  let container      = scene.create({ t: "object", parent: root,      w: scene.w, h: scene.h, focus: true, draw: true, clip: true });
  var shade          = scene.create({ t: "rect",   parent: container, w: scene.w, h: scene.h, fillColor: "#000", a: 0 });

  let placeholderSVG = 'data:image/svg, <svg width="'+frameW+'" height="'+frameH+'"><linearGradient id="PHL" gradientUnits="userSpaceOnUse" x1="32.0229" y1="142.4004" x2="171.9757" y2="61.5986" gradientTransform="matrix(-4.371139e-08 -1 1 -4.371139e-08 -2 201.9995)"><stop  offset="0.0056" style="stop-color:#375270"/><stop  offset="1.0000" style="stop-color:#7DA7D9"/></linearGradient><rect x="0" y="0" fill="url(#PHL)" width="'+frameW+'" height="'+frameH+'"/></svg>';
  let placeholderRes = scene.create({ t: "imageResource", url: placeholderSVG, w: frameW, h: frameH });
  let font           = scene.create({ t: "fontResource",  url: "FreeSans.ttf" });

  let assets  = [container.ready, shade.ready, placeholderRes.ready, font.ready ];

  // container.on('onMouseEnter', function() { console.log(" >>>> onMouseEnter"); } );
  // container.on('onMouseLeave', function() { console.log(" >>>> onMouseLeave"); } );

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function doFocus(o)
  {
   console.log("doFocus()  >>> " + o.info.title);

    let frame  = o.frame;
    let shadow = o.shadow;
    let title  = o.title;

    const TT = 0.25;

    frameFocusAnim  =  frame.animate({ sx: SS, sy: SS,       y: 3         }, TT, LINEAR, FFWD, 1);
    shadowFocusAnim = shadow.animate({                 x: 11,y: 9, a: 0.5 }, TT, LINEAR, FFWD, 1);
    titleFocusAnim  =  title.animate({                             a: 0.8 }, TT, LINEAR, FFWD, 1);

    return Promise.all([frameFocusAnim.done, shadowFocusAnim.done]);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  function doMouseDown(o)
  {
    var index = appTiles.indexOf(o)
    var pos   = ind2sub(index);

    pos_x = pos.x;
    pos_y = pos.y;

    moveTo(pos_x, pos_y);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function doBlur(o)
  {
    console.log("doBlur()  >>> " + o.info.title);

    let frame  = o.frame;
    let shadow = o.shadow;
    let title  = o.title;

    const TT = 0.125;
    const ar = 1/AR;

    frameBlurAnim   =  frame.animate({ sx: 1.0, sy: 1.0, x: borderW, y: borderW/2      },   TT, LINEAR, FFWD, 1);
    shadowFocusAnim = shadow.animate({                   x: 0, y: 0, a: 0.15 },   TT, LINEAR, FFWD, 1);
    titleBlurAnim   =  title.animate({ a: 0.0                                }, 2*TT, LINEAR, FFWD, 1);

    return Promise.all([frameBlurAnim.done, shadowBlurAnim.done]);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  container.on('onPreKeyDown', function (e)
  {
    console.log(">>>> ROOT KEY")

    if ( keys.is_CTRL(e.flags) ) // ENTER
    {
        if(e.keyCode == keys.L)
        {
          hideInfo();

          selected.focus = true; // restore

          e.stopPropagation(); 
        } 
    }
    else
    if ( keys.is_CTRL_ALT(e.flags) ) // ENTER
    {
        if(e.keyCode == keys.ENTER)
        {
          hideInfo();

          selected.focus = true; // restore

          e.stopPropagation(); 
        } 
    }
    else
    if(selected.focus == true)
    {
      var dx = 0, dy = 0;
      var target = e.target;

      switch(e.keyCode)
      {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.ENTER: showInfo(target.info); e.stopPropagation(); return; // SELECT App
        case keys.UP:    dx =  0; dy = -1;      e.stopPropagation(); break;  //    UP a row
        case keys.DOWN:  dx =  0; dy =  1;      e.stopPropagation(); break;  //  DOWN a row
        case keys.LEFT:  dx = -1; dy =  0;      e.stopPropagation(); break;  //  LEFT a square
        case keys.RIGHT: dx =  1; dy =  0;      e.stopPropagation(); break;  // RIGHT a square
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
          if(dy > 0)  pos_y = 0;
          if(dy < 0)  pos_y = rows - 2; // 2nd last row index
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
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  var showInfoAlready = false;

  function showInfo(info)
  {
    if(showInfoAlready) return;
    showInfoAlready = true;

    shade.animateTo({ a: 0.5 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);

    sceneview.showInfo(info)
    .then( (o) =>
    {
      sceneview.focus = true;
    });
  }

  function hideInfo()
  {
    shade.animateTo({ a: 0.0 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);

    sceneview.hideInfo()
    .then( (o) =>
    {
      showInfoAlready = false;
    });
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function updateSize(w, h)
  {
    var sx = w/scene.w;
    var sy = h/scene.h;

    container.sx = sx;
    container.sy = sy;

    // scrollable.root.h = hh * Math.ceil(appURLs.length / cols); //rows;
    // scrollable.update();
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function createTiles(urls)
  {
    const tw = tileW - borderW;

    urls.map( (app, n) =>
    {
      let appUrl = app.attributes.url;
      let imgUrl = app.image;

      var pos = ind2sub(n);

      var tileX = (pos.x * tileW);
      var tileY = (pos.y * tileH);

      var titleY = (frameH - 26);

      var pp = scrollable.root;

      let tile     = scene.create({ t: "rect",    parent: pp,       x: tileX,   y: tileY,     w: tileW,    h: tileH,  interactive: true,  fillColor: 0x00000000, lineColor: "#000", lineWidth: 0  });
      let frame    = scene.create({ t: "rect",    parent: tile,     x: borderW, y: borderW/2, w: frameW,   h: frameH, interactive: false, fillColor: "#777", lineColor: "#ddd", lineWidth: 1 });
      let shadow   = scene.create({ t: "rect",    parent: frame,    x: 0,       y: 0,         w: frameW,   h: frameH, interactive: false, fillColor: SHADOW_COLOR, a: 0.8 });
      let image    = scene.create({ t: "image",   parent: frame,    x: 0,       y: 0,         w: frameW,   h: frameH, interactive: false, stretchX: STRETCH, stretchY: STRETCH,  resource: placeholderRes});
      let titleBox = scene.create({ t: "rect",    parent: image,    x: 0,       y: titleY,    w: frameW,   h: 26,     interactive: false, cx: tw, cy: 13, fillColor: "#222", a: 0  });
      let title    = scene.create({ t: "textBox", parent: titleBox, x: 0,       y: 0,         w: frameW,   h: 26,     interactive: false, cx: tw, cy: 13, textColor: "#fff", pixelSize: 16,
                                                                                                            text: app.title, font: font, alignHorizontal: HC, alignVertical: VC});
      shadow.moveBackward();

      frame.cx    = frame.w/2;
      frame.cy    = frame.h/2;

      tile.frame  = frame;
      tile.image  = image;
      tile.title  = titleBox;
      tile.shadow = shadow;
      tile.appUrl = appUrl;

      tile.info   = app;

      appTiles.push(tile);

      // appTiles.push( tile );
      // appTiles.push( frame );
      // // appTiles.push( image );
      // appTiles.push( shadow );
      // appTiles.push( titleBox );
      // appTiles.push( title );

      tile.on("onFocus",  function(o) {  doFocus(o.target); });
      tile.on("onBlur",   function(o) {   doBlur(o.target); });
      tile.on("onMouseDown", function(o) { doMouseDown(o.target); });
//        imgUrl = imgUrl.replace("https", "http");

      scene.create({ t: "imageResource", url: imgUrl }).ready
      .then( function(o)
      {
        image.resource = o;
      })
      .catch( (err) => {
        console.error("Exception: >> " + imgUrl + " << load failed.  Err:" + err);
      });

      console.log(" APP["+n+"]   Title: " + app.title);
    });//map

    var tiles_ready = appTiles.map( o => o.ready); // collect promises

    Promise.all( tiles_ready ).then( function(o)
    {
        pos_x = 1; pos_y = 1; // middle-middle
        moveTo(pos_x, pos_y); // Initial position
    });
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

    try{
      selected = appTiles[index];

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

    sceneview.updateSize(e.w, e.h);
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   Promise.all(assets).then( (o) =>
  {
    shade.moveForward();

    scrollable = new Scrollable(scene, container, {rowScrollHeight: tileH});
    sceneview = new  SceneView(scene, container);

    scrollable.root.h = tileH * Math.ceil(appURLs.length / cols); //rows;
    scrollable.update();
    
    createTiles(appURLs);
    updateSize(scene.getWidth(), scene.getHeight());
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}).catch(function importFailed(err) {
  console.error("Import for showcaseApp.js failed: " + err);
});

