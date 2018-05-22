
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

  var bg = scene.create({t:"rect", parent: root, x: 0, y: 0, w: 100, h: 100, fillColor: "#CCC", id: "Background"});

  var title = scene.create({t:"textBox", text: "", parent: bg, pixelSize: 15, w: 800, h: 80, x: 0, y: 0,
                alignHorizontal: scene.alignHorizontal.LEFT, interactive: false,
                alignVertical:     scene.alignVertical.CENTER, textColor: 0x000000AF, a: 1.0});

  var buttonPLAY = null;

  var x0 = 10, y0 = 50, bw = 5, bh = 80, rx = 8, ry = 8, gap = 1, cx = 0;

  var buttonObjects = {};
  
  var buttonsPanel = scene.create({t:"path", parent: bg, interactive: false,
          d:"<g fill='#ddd' stroke='#666' stroke-width='1'><rect x='120' y='0' width='500' height='100' rx='20' ry='20'/></g>" });

  var buttonData = 
  [
    { func: "skipbwd()", id: "skipbwd",  key: "step-backward", buttonObj: null, svgObj: null, s: 0.2,  dx: 285, dy: 250,  x: 0, y: 0 },
    { func: "play()",    id: "play",     key: "play",          buttonObj: null, svgObj: null, s: 0.2,  dx: 350, dy: 250,  x: 0, y: 0 },
    { func: "stop()",    id: "stop",     key: "stop",          buttonObj: null, svgObj: null, s: 0.2,  dx: 285, dy: 250,  x: 0, y: 0 },
    { func: "pause()",   id: "pause",    key: "pause",         buttonObj: null, svgObj: null, s: 0.2,  dx: 285, dy: 250,  x: 0, y: 0 },
    { func: "skipfwd()", id: "skipfwd",  key: "step-forward",  buttonObj: null, svgObj: null, s: 0.2,  dx: 285, dy: 250,  x: 0, y: 0 },
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

        var icon_w = icon[0];
        var icon_h = icon[1];
        var icon_d = icon[4];

        var dx = data.dx - (512 - icon_w)/2;
        var dy = data.dy;
        
        var path =  "<svg>"
                  + "  <defs>"
                  + "    <linearGradient id='myLinearGradient1' x1='0%' y1='0%' x2='0%' y2='100%'"
                  + "                    spreadMethod='pad' gradientTransform='rotate(-45)'>"
                  + "      <stop offset='0%'   stop-color='#aaaaaa' stop-opacity='1'/>"
                  + "      <stop offset='100%' stop-color='#666666' stop-opacity='1'/>"
                  + "    </linearGradient>"
                  + "  </defs>"
                  + ""
                  + " <g>"
                  + "  <circle cx='100' cy='100' r='"+ 100 +"'  "
                  + "     style='fill:url(#myLinearGradient1);' />"
    //                 + "     style='fill-opacity:0.0; stroke: #eee; stroke-width: 8; stroke-alignment: inner' />"
                  + "    <path transform='scale(.2)  translate("+ dx +","+ dy +")' "
                  + "          style='fill: #000; fill-opacity:0.0; stroke: #eee; stroke-width: 28;'"
                  + " d='" + icon_d + "/>"
                    + " </g>" 
                  + "</svg>"

        console.log("Initial Create >> " + data.id);

        var button = scene.create( { t: "path", d: path, w: icon_w/8, h: icon_h/8, parent: root, x: x0, y: y0, 
                                        interactive: true, id: data.id, parent: buttonsPanel } );

        // RESPONDERS
        button.on("onMouseDown",   function (e)
        {
          var me = buttonObjects[e.target.id];
          me.buttonObj.y += 3;
        });
        button.on("onMouseUp",     function (e)
        {
          var me = buttonObjects[e.target.id];
          me.buttonObj.y -= 3;
        });

        data.buttonObj         = button;
        buttonObjects[data.id] = data;

        return button.ready.catch(e => { return null; });
      }); // MAP

      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Initial layout...

      Promise.all( allReady ).then(values =>
        {
          console.log("Initial Layout");
      
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
    console.log("Do Layout");

    var px = 0, py = 0;
    var totalW   = 0;
    var buttonW  = 0;
    var buttonH  = 0;

    Object.keys(buttonObjects).map(key =>
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

    Object.keys(buttonObjects).map(key =>
    {
      var o = buttonObjects[key];
      var b = o.buttonObj;

      if (b.x != px || b.y != py)
      {
        b.animateTo({x: px, y: py }, 0.5, scene.animation.TWEEN_STOP).catch(() => {});

        console.log("Do Layout - px = " + px + "   py =" + py);

        o.x = px; // target X
        o.y = py; // target Y
      }
      px += (buttonW + gap);

//       console.log("buttonsW: "+buttonsW+" bg: "+ bg.w + "  spare_x: "+ spare_x+ "  buttons: "+ buttonData.length);
//       console.log("gap: "+gap+" bw: "+ bw +"  px >> " + px);
    });
  };
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function updateSize(w,h)
  {
     bg.w = w;
     bg.h = h;

     console.log("Button Panel WxH >> " + buttonsPanel.w + " x " + buttonsPanel.h);

     buttonsPanel.x = (w - buttonsPanel.w)/2;
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
