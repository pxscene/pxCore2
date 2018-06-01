
var fa_icons = // https://fontawesome.com/icons?d=gallery&c=audio-video
{
  "pause":         [448, 512, [], "f04c", "M144 479H48c-26.5 0-48-21.5-48-48V79c0-26.5 21.5-48 48-48h96c26.5 0 48 21.5 48 48v352c0 26.5-21.5 48-48 48zm304-48V79c0-26.5-21.5-48-48-48h-96c-26.5 0-48 21.5-48 48v352c0 26.5 21.5 48 48 48h96c26.5 0 48-21.5 48-48z"],
  "pause-circle":  [512, 512, [], "f28b", "M256 8C119 8 8 119 8 256s111 248 248 248 248-111 248-248S393 8 256 8zm-16 328c0 8.8-7.2 16-16 16h-48c-8.8 0-16-7.2-16-16V176c0-8.8 7.2-16 16-16h48c8.8 0 16 7.2 16 16v160zm112 0c0 8.8-7.2 16-16 16h-48c-8.8 0-16-7.2-16-16V176c0-8.8 7.2-16 16-16h48c8.8 0 16 7.2 16 16v160z"],
  "play":          [448, 512, [], "f04b", "M424.4 214.7L72.4 6.6C43.8-10.3 0 6.1 0 47.9V464c0 37.5 40.7 60.1 72.4 41.3l352-208c31.4-18.5 31.5-64.1 0-82.6z"],
  "play-circle":   [512, 512, [], "f144", "M256 8C119 8 8 119 8 256s111 248 248 248 248-111 248-248S393 8 256 8zm115.7 272l-176 101c-15.8 8.8-35.7-2.5-35.7-21V152c0-18.4 19.8-29.8 35.7-21l176 107c16.4 9.2 16.4 32.9 0 42z"],
  "step-backward": [448, 512, [], "f048", "M64 468V44c0-6.6 5.4-12 12-12h48c6.6 0 12 5.4 12 12v176.4l195.5-181C352.1 22.3 384 36.6 384 64v384c0 27.4-31.9 41.7-52.5 24.6L136 292.7V468c0 6.6-5.4 12-12 12H76c-6.6 0-12-5.4-12-12z"],
  "step-forward":  [448, 512, [], "f051", "M384 44v424c0 6.6-5.4 12-12 12h-48c-6.6 0-12-5.4-12-12V291.6l-195.5 181C95.9 489.7 64 475.4 64 448V64c0-27.4 31.9-41.7 52.5-24.6L312 219.3V44c0-6.6 5.4-12 12-12h48c6.6 0 12 5.4 12 12z"],
  "stop":          [448, 512, [], "f04d", "M400 32H48C21.5 32 0 53.5 0 80v352c0 26.5 21.5 48 48 48h352c26.5 0 48-21.5 48-48V80c0-26.5-21.5-48-48-48z"],
  "stop-circle":   [512, 512, [], "f28d", "M256 8C119 8 8 119 8 256s111 248 248 248 248-111 248-248S393 8 256 8zm96 328c0 8.8-7.2 16-16 16H176c-8.8 0-16-7.2-16-16V176c0-8.8 7.2-16 16-16h160c8.8 0 16 7.2 16 16v160z"],
};

/*
   pxCore Copyright 2005-2017 John Robinson

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

px.import({
  scene: 'px:scene.1.js'
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var root  = imports.scene.root;

  var base    = px.getPackageBaseFilePath();
  var fontRes = scene.create({ t: "fontResource", url: "FreeSans.ttf" });

  var COLOR_BG   =  "#CCC";
  var COLOR_TEXT =  "#222";

  var bg = scene.create({t:"rect", parent: root, x: 0, y: 0, w: 100, h: 100, fillColor: COLOR_BG });

  var title = scene.create({t:"textBox", text: "", parent: bg, pixelSize: 15, w: 800, h: 80, x: 0, y: 0,
                alignHorizontal: scene.alignHorizontal.LEFT, interactive: false,
                alignVertical:     scene.alignVertical.CENTER, textColor: COLOR_TEXT, a: 1.0});

  var buttonPLAY = null;

  var x0 = 10, y0 = 50, bw = 5, bh = 80, rx = 8, ry = 8, gap = 1, cx = 0;

  var buttonObjects = {};
  
  var buttonsPanel = scene.create({t:"path", parent: bg, interactive: false,
          d:"<g fill='#ddd' stroke='#666' stroke-width='1'><rect x='120' y='0' width='500' height='100' rx='20' ry='20'/></g>" });

  var buttonData = 
  [
    { func: "skipbwd()", id: "skipbwd",  key: "step-backward", buttonObj: null, clickObj: null, s: 0.2,  dx: 25, dy: 0,  x: 0, y: 0 },
    { func: "play()",    id: "play",     key: "play",          buttonObj: null, clickObj: null, s: 0.2,  dx: 50, dy: 0,  x: 0, y: 0 },
    { func: "stop()",    id: "stop",     key: "stop",          buttonObj: null, clickObj: null, s: 0.2,  dx: 50, dy: 0,  x: 0, y: 0 },
    { func: "pause()",   id: "pause",    key: "pause",         buttonObj: null, clickObj: null, s: 0.2,  dx: 50, dy: 0,  x: 0, y: 0 },
    { func: "skipfwd()", id: "skipfwd",  key: "step-forward",  buttonObj: null, clickObj: null, s: 0.2,  dx: 25, dy: 0,  x: 0, y: 0 },
  ];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function createButtons()
{
  console.log("Call  createButtons()");

   var allReady =  buttonData.map(data => 
    {
       x0 = -1;

       var key    = data.key;
       var icon   = fa_icons[key];

       var svg_w  = 1024;
       var svg_h  = 1024;

       var circle_r  = svg_h / 2;

       var icon_w = 512; //icon[0];
       var icon_h = 512; //icon[1];

       var icon_d = icon[4];

       var button_w = 512 / 8;
       var button_h = 512 / 8;

       var init_y = 0;
       var nudge_x = 32;

       var defineGradient =   " <defs>"
                            + "   <linearGradient id='myLinearGradient1' x1='0%' y1='0%' x2='0%' y2='100%'"
                            + "                   spreadMethod='pad' gradientTransform='rotate(-45)'>"
                            + "     <stop offset='0%'    stop-color='#aaaaaa'  stop-opacity='1' />"
                            + "     <stop offset='100%'  stop-color='#666666'  stop-opacity='1' />"
                            + "   </linearGradient>"
                            + " </defs>";

        var styleGradient   = " style='fill: url(#myLinearGradient1);' ";
        var styleNormal     = " style='fill: #000; fill-opacity:0.0; stroke: #eee; stroke-width: 28;' ";
        var styleClick      = " style='fill: #000; fill-opacity:0.0; stroke: #ff0; stroke-width: 32;' ";

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        this.modeX = false; // for calibration

        var pathMarkerX  = ( !modeX ? "" : "<path d='M0 0, L1024,1024 M1024,0 L0,1024, M512,0, L512,1024, M0,512 L1024,512' stroke='#f00' stroke-width='4'/>");
 
        if(modeX)
        {
          button_w = 512;  button_h = 512;
          init_y   = -450;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        var pathNormal =  "<svg width='"+ svg_w +"' height='"+ svg_h +"'>"
                        + defineGradient
                        + " <g>"
                        + "  <circle cx='"+ circle_r +"' cy='"+ circle_r +"' r='"+ circle_r +"' " + styleGradient + "/>"
                        + pathMarkerX // MARKER
                        + "  <path transform=' translate("+ (icon_w/2 + nudge_x) +","+ icon_h/2 +")' " + styleNormal 
                        + " d='" + icon_d + "' />" + " </g>" + "</svg>"; // ICON

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        var pathClick =   "<svg width='"+ svg_w +"' height='"+ svg_h +"'>"
                        + defineGradient
                        + " <g>"
                        + "  <circle cx='"+ circle_r +"' cy='"+ circle_r +"' r='"+ circle_r +"' " + styleGradient +"/>"
                        + pathMarkerX // MARKER
                        + "  <path transform=' translate("+ (icon_w/2 + nudge_x) +","+ icon_h/2 +")' " + styleClick 
                        + " d='" + icon_d + "' />" + " </g>"  + "</svg>"; // ICON

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        console.log("Initial Create >> " + data.id);

        var imageNormal = scene.create( { t: "path", d: pathNormal, w: button_w, h: button_h, parent: root, x: 0, y: init_y,
                                        interactive: true, id: data.id, parent: buttonsPanel} );

        var imageClick  = scene.create( { t: "path", d: pathClick,  w: button_w, h: button_h, parent: root, x: 0, y: 0,
                                        interactive: false, id: data.id, parent: imageNormal, a: 0} );

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        // RESPONDERS
        imageNormal.on("onMouseDown", function (e)
        {
          var me = buttonObjects[e.target.id];
          me.buttonObj.y += 3;

          me.clickObj.a = 1.0;
        });
        imageNormal.on("onMouseUp",   function (e)
        {
          var me = buttonObjects[e.target.id];
          me.buttonObj.y -= 3;

          if(this.modeX)
          {
            me.buttonObj.moveToBack();
          }

          me.clickObj.a = 0.0;
        });

        data.buttonObj         = imageNormal;
        data.clickObj          = imageClick;
        buttonObjects[data.id] = data;

        return imageNormal.ready.catch(e => { return null; });
      }); // MAP

      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Initial layout...

      Promise.all( allReady ).then(values =>
      {
        var targets = Object.keys(buttonObjects).map(button => { return {x: -1, y: -1}; });

        layout();
      });

      }// FUNCTION

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  buttonsPanel.ready.then(function(o)
  {
    createButtons();
  });
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function layout()
  {
    if(this.modeX == true)
    {
      return; // Skip Layout  >> calibration
    }

    var px = 0, py = 0;
    var totalW   = 0;
    var buttonW  = 0;
    var buttonH  = 0;

    var keys = Object.keys(buttonObjects);

    // Survey dimensions...
    keys.map(key =>
    {
      var o = buttonObjects[key];
      var b = o.buttonObj;

      totalW += b.w;

      buttonW = b.w;
      buttonH = b.h;
    });

    var spare_x = (buttonsPanel.w - totalW);

    // Compute GAP and Initial Position
    gap = spare_x / (buttonData.length + 1);

    px  = gap;
    py  = (buttonsPanel.h - buttonH)/2;

    keys.map(key =>
    {
      var o = buttonObjects[key];
      var b = o.buttonObj;

      if (b.x != px || b.y != py)
      {
        b.animateTo({x: px, y: py }, 0.5, scene.animation.TWEEN_STOP).catch(() => {});

        o.x = px; // target X
        o.y = py; // target Y
      }
      px += (buttonW + gap);
    });
  };
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function updateSize(w,h)
  {
     bg.w = w;
     bg.h = h;

     console.log("Button Panel WxH >> " + buttonsPanel.w + " x " + buttonsPanel.h);

     buttonsPanel.x = (w - buttonsPanel.w) / 2;
     buttonsPanel.y = (h - buttonsPanel.h) - 20;
     
     title.x = 10;
     title.y = bg.h - title.h;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  scene.on("onResize", function(e) { updateSize(e.w, e.h); });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Promise.all([ bg.ready, title.ready ])
      .catch( (err) =>
      {
          console.log("SVG >> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
          updateSize(scene.w, scene.h);

          bg.focus = true;
      });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}).catch( function importFailed(err){
  console.error("SVG >> Import failed: " + err);
});
