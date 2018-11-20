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


px.import({ scene: 'px:scene.1.js' }).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;

  var LIGHT_BLUE  = "#2B9CD8";
  var DARK_GRAY   = "#DADADA";
  var LIGHT_GRAY  = "#BABABA";
  var LIGHT_GRAY2 = "#B0B0B0";

  var autoDismiss  = null;
  var textRows     = {};
  var textRowBG    = null;

  var label_w   = 175;
  var label_h   = 26;
  var value_w   = 355;
  var value_h   = 26;

  var max_w     = label_w + value_w;
  var max_h     = 175;
  var title_dy  = 30;

  var fontRes   = scene.create({ t: "fontResource",  url: "FreeSans.ttf" });
  var panel     = scene.create({ t: "object",  parent: root, x: 40, y: -720, w: (max_w + 4), h: max_h, a: 0.0 });
  var panel_bg  = scene.create({ t: "rect",    parent: panel, fillColor: LIGHT_GRAY2, w: panel.w + 16, h: panel.h + 8, x: -8, y: -8});
  var rows      = scene.create({ t: "object",  parent: panel_bg, a: 1.0 });

  // Use embedded SVG URI for logo
  var logoURI = "data:image/svg,"+ '<svg height="571" viewBox="0 0 575.806 571" width="575.806" xmlns="http://www.w3.org/2000/svg"><path d="m83.797 283.5-80.912-140.5 80.912-140.5h161.826l80.913 140.5-80.913 140.5z" fill="#e6e7e8" stroke="#5d6d65" stroke-width="5"/><path d="m101.463 252-63.262-109 63.262-109h126.495l63.262 109-63.262 109z" fill="#f16268" opacity=".75"/><path d="m223.755 41 59.044 102-59.044 102h-118.089l-59.044-102 59.044-102h118.414m8.081-15h-8.406-118.089-8.406l-4.21 7.454-59.045 102.116-4.226 7.346 4.226 7.323 59.044 102.256 4.211 7.505h8.406 118.089 8.406l4.21-7.493 59.044-102.137 4.226-7.355-4.226-7.328-59.043-102.219z" fill="#5d6d65"/><path d="m83.799 568.5-80.912-140.5 80.912-140.5h161.826l80.914 140.5-80.914 140.5z" fill="#e6e7e8" stroke="#5d6d65" stroke-width="5"/><path d="m101.465 533-63.262-109.499 63.262-109.501h126.496l63.262 109.501-63.262 109.499z" fill="#faef5f" opacity=".75"/><path d="m223.758 321 59.044 102.001-59.044 101.999h-118.09l-59.044-101.999 59.044-102.001h118.412m8.084-14h-8.406-118.09-8.406l-4.21 7.181-59.044 101.98-4.226 7.278 4.226 7.289 59.043 102.012 4.21 7.26h8.406 118.09 8.406l4.21-7.267 59.044-102.021 4.226-7.299-4.226-7.3-59.044-101.933z" fill="#5d6d65"/><path d="m330.181 425.5-80.913-140 80.913-140h161.824l80.913 140-80.913 140z" fill="#e6e7e8" stroke="#5d6d65" stroke-width="5"/><path d="m347.846 394-63.263-109.5 63.263-109.5h126.495l63.262 109.5-63.262 109.5z" fill="#f89958" opacity=".75"/><path d="m470.138 182 59.044 102-59.044 102h-118.089l-59.044-102 59.044-102h118.031m8.464-14h-8.406-118.089-8.406l-4.21 7.205-59.044 101.992-4.226 7.283 4.226 7.292 59.044 101.99 4.21 7.237h8.406 118.089 8.406l4.21-7.242 59.044-102.011 4.226-7.292-4.226-7.297-59.044-101.954z" fill="#5d6d65"/><path d="m285.83 428-22.434-39 22.434-39h44.867l22.434 39-22.434 39z" fill="#5d6d66"/><path d="m207.694 272.275 47.116-27.195 74.838 129.592-47.116 27.196z" fill="#5c6d65"/><path d="m97.708 312-15.824-27 15.824-27h31.646l15.824 27-15.824 27z" fill="#5d6d66"/><path d="m115.58 258.5h150v54h-150z" fill="#5c6d65"/><path d="m297.438 197-15.824-27 15.824-27h31.646l15.824 27-15.824 27z" fill="#5d6d66"/><path d="m254.324 325.59-47.116-27.195 74.838-129.593 47.116 27.196z" fill="#5c6d65"/></svg>';

  var logoRes = scene.create({ t: "imageResource", w: 128, h: 128, url: logoURI });

  var logo = scene.create({ t: "image",  parent: panel, resource: logoRes, x: 0, y: 5});

  var dismissTXT = scene.create({ t: "textBox", parent: panel, textColor: "#000",
                            w: max_w, h: 20, x: 8, y: 0,
                            font: fontRes, pixelSize: 12, wordWrap: true,
                            text: "Press SPACE to dismiss",
                            alignHorizontal: scene.alignHorizontal.CENTER,
                            alignVertical:   scene.alignVertical.CENTER})
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
                                    font: fontRes, pixelSize: textPts, wordWrap: true, truncation: scene.truncation.TRUNCATE,
                                    xStartPos: ix, xStopPos: ix,
                                    text: value, // ### VALUE
                                    alignHorizontal: scene.alignHorizontal.LEFT,
                                    alignVertical:   scene.alignVertical.CENTER})
    textRows[key] = valueTxt;

    max_h += (value_h + 4);
                                           

    return Promise.all([row.ready, labelRect.ready, labelTxt.ready, valueRect.ready, valueTxt.ready] )
      .catch( function (err)
    {
       console.log(">>> Loading Assets ... err = " + err);
                                                               
    }).then( function ()
    {
       // resolve
    });
  }

  function createPanel()
  {
    var gfx = parseInt( parseInt( scene.info.gfxmemory ) / 1024);

    var promises = [];
                        
    promises.push( addRow(rows, "InfoBuildVersion",  "Version: ",         scene.info.version) );
    promises.push( addRow(rows, "InfoBuildEngine",   "Engine: ",          scene.info.engine) );
    promises.push( addRow(rows, "InfoBuildDate",     "Build Date: ",      scene.info.build.date.replace(/"/g, '') )); // global RegEx
    promises.push( addRow(rows, "InfoBuildTime",     "Build Time: ",      scene.info.build.time.replace(/"/g, '') )); // global RegEx
    promises.push( addRow(rows, "InfoBuildRevision", "Build Revision: ",  scene.info.build.revision.replace(/"\s*/g, '') )); // global RegEx
    promises.push( addRow(rows, "InfoGfxMemory",     "Base GFX memory: ", gfx.toLocaleString()  + " KB") );

    return Promise.all( promises )
             .catch( function (err)
           {
              console.log(">>> Loading rows ... err = " + err);
           }).then( function ()
           {
              // resolve
           });
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
                                       scene.animation.OPTION_FASTFORWARD, 1)
      .then(
		function() 
		{
		  panel_bg.focus = true;
		});


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
  panel_bg.on("onKeyDown", function(e) { if(e.keyCode) { hidePanel();  }  });

  Promise.all([fontRes.ready, logo.ready, title_bg.ready, panel.ready]).catch( function (err)
  {
    console.log(">>> Loading Assets ... err = " + err);

  }).then( function ()
  {
    title = scene.create({ t: "text", text: "About Spark", font: fontRes, parent: panel, pixelSize: titlePts, textColor: 0xFFFFFFff });

    title.ready.then( function ()
    {
        var titleM = fontRes.measureText(titlePts, title.text);

        title.x    = (panel.w - titleM.w)/2;
        title.y    = logo.y + logo.resource.h + title_dy;
        title.h    = titleM.h;
        title_bg.y = title.y;
        title_bg.h = title.h;

        max_h += title.h;
        max_h += dismissTXT.h + value_h;

        var ready = createPanel();

        ready.then(function () 
        {
          dismissTXT.y = max_h - dismissTXT.h - 10;

          panel.h    = max_h;
          panel_bg.h = max_h;

          updateSize(scene.w, scene.h);
        });
    });
  }); // PROMISE.ALL

}).catch( function importFailed(err){
  console.error("Import failed for about.js: " + err);
});
