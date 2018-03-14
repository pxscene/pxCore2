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



px.import({ scene:      'px:scene.1.js',
             keys:      'px:tools.keys.js',
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;

  var MM;

  var fontRes = scene.create({ t: "fontResource", url: "FreeSans.ttf" });

  var bg = scene.create({t:"rect", parent: root, x:  0, y: 0, w: 100, h: 100, fillColor: 0xffFFFFff, a: 1.0, id: "Background"});

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Create the BUTTONS
//
var strokeClr   = 0x555555ff;
var fillClr_OFF = 0x0000FF0f;
var fillClr_ON  = 0x00ff0028;
var textClr     = 0x000000e8;

var buttonData = [
    { func: "drawSVG_SparkPage()", id: "Spark Page",   text: "Spark Page",   buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_SparkLogo()", id: "Spark Logo",   text: "Spark Logo",   buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_LetterM()"  , id: "Letter M",     text: "Letter M",     buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_Cross()"    , id: "SVG Cross",    text: "SVG Cross",    buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_Circle()"   , id: "SVG Circle",   text: "SVG Circle",   buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_Polygon()"  , id: "Polygon",      text: "Polygon",      buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_Ellipse()"  , id: "Ellipse",      text: "Ellipse",      buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_RRect()"    , id: "Rounded Rect", text: "Rounded Rect", buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_ARC1()"     , id: "SVG Arc 1",    text: "SVG Arc 1",    buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_ARC2()"     , id: "SVG Arc 2",    text: "SVG Arc 2",    buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_SWEEP()"    , id: "SVG Sweep",    text: "SVG Sweep",    buttonObj: null, svgObj: null, x: 0, y: 0 },
    { func: "drawSVG_CURVES()"   , id: "SVG Curves",   text: "SVG Curves",   buttonObj: null, svgObj: null, x: 0, y: 0 }
];

var x0 = 10, y0 = 5, bw = 95, bh = 35, rx = 8, ry = 8, gap = 5, pts = 12 ,cx = 0;

var buttonObjects = {};

var buttonsPanel = scene.create({t:"rect",  parent: bg, x:0, y:0,   a: 1.0, w:1256, h:bh + 20, fillColor: 0xccccccFF, interactive: false });

var panelAnimate = null;
var panelVisible = true;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var buttonsReady = buttonData.map(data => 
    {
      x0 = -1;

      var button = scene.create( { t: "path", d:"rect x:0 y:0 width:"+bw+" height:"+bh+" rx:"+rx+" ry:"+ry+"", a: 1.0, x: x0, y: y0, 
                              strokeColor: strokeClr, strokeWidth: 2, strokeType: "inside", interactive: true, fillColor: fillClr_OFF, parent: buttonsPanel } );

      var text = scene.create({t:"textBox", text: data.text, parent: button, pixelSize: pts, w: bw, h: bh, x: 0, y: 0,
                  alignHorizontal: scene.alignHorizontal.CENTER, id: data.id, interactive: true,
                  alignVertical:     scene.alignVertical.CENTER, textColor: textClr, a: 1.0});

      Promise.all([button.ready, text.ready])
      .then((success, failure) =>
      {
         button.on("onMouseDown",   function (e)
         {
            var me = buttonObjects[e.target.id];
            me.buttonObj.y += 3;
         });
         button.on("onMouseUp",     function (e)
         {
            var me = buttonObjects[e.target.id];
            me.buttonObj.y -= 3; buttonToggle(me);
         });
      });

      data.buttonObj = button;
      buttonObjects[data.id] = data;

      x0 += (bw + gap);

      return button.ready.catch(e => { return null; });
    });

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Initial layout...

    Promise.all(buttonsReady).then(values =>
    {
      var targets = buttonObjects.map(box => { return {x: -1, y: -1}; });

      layout();
    });


  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bg.ready.then(
  function(o)
  {
    var show_SparkPage  = false;
    var show_SparkLogo  = false;
    var show_LetterM    = false;
    var show_Cross      = false;
    var show_Circle     = false;
    var show_Polygon    = false;
    var show_Ellipse    = false;
    var show_RRect      = false;
    var show_ARC1       = false;
    var show_ARC2       = false;
    var show_SWEEP      = false;
    var show_CURVES     = false;

    if( show_SparkPage )  drawSVG_SparkPage();
    if( show_SparkLogo )  drawSVG_SparkLogo();
    if( show_LetterM   )  drawSVG_LetterM();
    if( show_Cross     )  drawSVG_Cross();
    if( show_Circle    )  drawSVG_Circle();
    if( show_Polygon   )  drawSVG_Polygon();
    if( show_Ellipse   )  drawSVG_Ellipse();
    if( show_RRect     )  drawSVG_RRect();
    if( show_ARC1      )  drawSVG_ARC1();
    if( show_ARC2      )  drawSVG_ARC2();
    if( show_SWEEP     )  drawSVG_SWEEP();
    if( show_CURVES    )  drawSVG_CURVES();

  });

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function drawSVG_all()
  {
     //drawSVG_Spark();
     drawSVG_LetterM();
     drawSVG_Cross();
     drawSVG_Circle();
     drawSVG_Ellipse();
     drawSVG_RRect();
    //  drawSVG_ARC1();
    //  drawSVG_ARC2();
     drawSVG_SWEEP();
     drawSVG_CURVES();
  }

  function fake_translate(cc)
  {
    // translate(<x> [<y>]) = matrix(1 0 0 1 x y)
    //  scene.create( { t: "path",
    //      id: "fake translate(0,-229.26665)",
    //      d: "M0 -229.26665",
    //  parent: cc } );
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Spark Page ...
  function drawSVG_SparkLogo()
  {
    var svg    = scene.create({t:"object", parent: bg, x:350, y:30, a: 1.0, w:1256, h:800 });
    var layer1 = scene.create({t:"path", id: "layer1", /*transform: "translate(300,-229.26665)",*/ parent: svg });

    var sparkFill   = "#5D6D65";
    var sparkHiLite = "#E6E7E8";
    var sparkStroke = "#FF00FF";

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // polygon - TOP
    var t1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Top_2_", a:1,
                    d: "polygon points: 83.797,283.5 2.885,143 83.797,2.5 245.623,2.5 326.536,143 245.623,283.5",
            fillColor: sparkHiLite, strokeColor: sparkFill, strokeWidth: 5, strokeType: "center" });

    // alpha solid - RED
    var t2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Top_2_", a:0.75,
                    d: "polygon points: 101.463,252 38.201,143 101.463,34 227.958,34 291.22,143 227.958,252",
            fillColor: "#F16268"} );

    // path
    var t3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Top_1_", a:1.0,
                    d: "M223.755,41l59.044,102l-59.044,102H105.666L46.622,143l59.044-102H224.08 M232.161,26h-8.406H105.666" +
                       "H97.26l-4.21,7.454L34.005,135.57l-4.226,7.346l4.226,7.323l59.044,102.256L97.26,260h8.406h118.089h8.406l4.21-7.493" +
                       "l59.044-102.137l4.226-7.355l-4.226-7.328L236.372,33.468L232.161,26L232.161,26z",
            fillColor: sparkFill} );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // polygon - BOTTOM
    var b1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Bottom_2_", a:1,
                    d: "polygon points: 83.799,568.5 2.887,428 83.799,287.5 245.625,287.5 326.539,428 245.625,568.5",
            fillColor: sparkHiLite, strokeColor: sparkFill, strokeWidth: 5, strokeType: "center" });

    // alpha solid - YELLOW
    var b2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Bottom_2_", a:0.75,
                    d: "polygon points: 101.465,533 38.203,423.501 101.465,314 227.961,314 291.223,423.501 227.961,533",
            fillColor: "#FAEF5F"} );

    // path
    var b3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Bottom_2_", a:1.0,
                    d: "M223.758,321l59.044,102.001L223.758,525h-118.09L46.624,423.001L105.668,321H224.08 M232.164,307h-8.406" +
                       "h-118.09h-8.406l-4.21,7.181l-59.044,101.98l-4.226,7.278l4.226,7.289L93.051,532.74l4.21,7.26h8.406h118.09h8.406l4.21-7.267"+
                       "l59.044-102.021l4.226-7.299l-4.226-7.3l-59.044-101.933L232.164,307L232.164,307z",
            fillColor: sparkFill} );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // polygon - RIGHT
    var r1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Right_2_", a:1,
                    d: "polygon points: 330.181,425.5 249.268,285.5 330.181,145.5 492.005,145.5 572.918,285.5 492.005,425.5",
            fillColor: sparkHiLite, strokeColor: sparkFill, strokeWidth: 5, strokeType: "center" });

    // alpha solid - ORANGE
    var r2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Right_2_", a:0.75,
                    d: "polygon points: 347.846,394 284.583,284.5 347.846,175 474.341,175 537.603,284.5 474.341,394",
            fillColor: "#F89958"} );

    // path
    var r3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Right_2_", a:1.0,
                    d: "M470.138,182l59.044,102l-59.044,102H352.049l-59.044-102l59.044-102H470.08 M478.544,168h-8.406H352.049" +
                       "h-8.406l-4.21,7.205l-59.044,101.992l-4.226,7.283l4.226,7.292l59.044,101.99l4.21,7.237h8.406h118.089h8.406l4.21-7.242" +
                       "l59.044-102.011l4.226-7.292l-4.226-7.297l-59.044-101.954L478.544,168L478.544,168z",
            fillColor: sparkFill} );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // MIDDLE FILL
    var m1 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__BOTTOM_1_", a:1.0,
                    d: "polygon points: 285.83,428 263.396,389 285.83,350 330.697,350 353.131,389 330.697,428",
            fillColor: sparkFill} );

    var m2 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__BOTTOM", a:1.0,
                    d: "polygon points: 207.694,272.275 254.81,245.08 329.648,374.672 282.532,401.868",
            fillColor: sparkFill} );

    var m3 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__LEFT", a:1.0,
                    d: "polygon points: 97.708,312 81.884,285 97.708,258 129.354,258 145.178,285 129.354,312",
            fillColor: sparkFill} );

    var m4 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__LEFT_1_", a: 1.0,
                    d: "rect x:115.58 y:258.5 width:150 height:54",
            fillColor: sparkFill} );

    var m5 = scene.create( { t: "path", parent: layer1, id: "_x3C_Group_x3E__RIGHT", a:1.0,
                    d: "polygon points: 297.438,197 281.614,170 297.438,143 329.084,143 344.908,170 329.084,197",
            fillColor: sparkFill} );

    var m6 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__RIGHT_1_", a:1.0,
                    d: "polygon points: 254.324,325.59 207.208,298.395 282.046,168.802 329.162,195.998",
            fillColor: sparkFill} );

    svg.interactive = false; // ESSENTIAL

    return svg;
  } 

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Spark Page ...
  function drawSVG_SparkPage()
  {
    var svg         = scene.create({t:"object", parent: bg, x:10, y:-50, a: 1.0, id: "Spark Page", interactive: false});

    var sparkFill   = "#5D6D65";
    var sparkHiLite = "#E6E7E8";
    var sparkStroke = "#FF00FF";

    // polygon - TOP
    var t1 = scene.create( { t: "path", parent: svg, id: "Middle_-_Top_2_", a:1, interactive: false,
                 d: "polygon points: 421.288,326.5 373.926,245.5 421.288,164.5 516.011,164.5 563.373,245.5 516.011,326.5",
                 fillColor: sparkHiLite, strokeColor: sparkFill, strokeWidth: 5, strokeType: "center" });

    // alpha solid - RED
    var t2 = scene.create( { t: "path", parent: svg, id: "Color_-_Top_2_", a:0.75, interactive: false,
                 d: "polygon points: 431.649,309 394.608,245.5 431.649,182 505.649,182 542.69,245.5 505.649,309",
                 fillColor: "#F16268", strokeType: "center" });

    // path
    var t3 = scene.create( { t: "path", parent: svg, id: "Outline-_Top_1_", a:1.0, interactive: false,
                 d: "M503.21,186l34.562,59.5L503.21,305h-69.123l-34.561-59.5l34.561-59.5H503 M508.088,178h-4.878h-69.123" +
                 "h-4.878l-2.459,4.065l-34.562,59.134l-2.5,4.246l2.5,4.265l34.562,59.142L429.21,313h4.878h69.123h4.878l2.459-4.157" +
                 "l34.562-59.179l2.5-4.269l-2.5-4.276l-34.562-59.057L508.088,178L508.088,178z",
                 fillColor: sparkFill, strokeType: "center" });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // polygon - BOTTOM
    var b1 = scene.create( { t: "path", parent: svg, id: "Middle_-_Bottom_2_", a:1, interactive: false,
                 d: "polygon points: 421.289,491.5 373.927,410.5 421.289,329.5 516.012,329.5 563.374,410.5 516.012,491.5 ",
                 fillColor: sparkHiLite, strokeColor: sparkFill, strokeWidth: 5, strokeType: "center" });

    // alpha solid - YELLOW
    var b2 = scene.create( { t: "path", parent: svg, id: "Color_-_Bottom_2_", a:0.75, interactive: false,
                 d: "polygon points: 431.65,472 394.609,408.5 431.65,345 505.651,345 542.692,408.5 505.651,472",
                 fillColor: "#FAEF5F", strokeType: "center" });

    // path
    var b3 = scene.create( { t: "path", parent: svg, id: "Outline-_Bottom_2_", a:1.0, interactive: false,
                 d: "M503.212,349l34.562,59l-34.562,59h-69.123l-34.561-59l34.561-59H503 M508.09,340h-4.878h-69.123h-4.878" +
                 "l-2.459,4.439l-34.562,59.319l-2.5,4.34l2.5,4.312l34.562,59.291l2.459,4.299h4.878h69.123h4.878l2.459-4.284l34.562-59.241" +
                 "l2.5-4.302l-2.5-4.292l-34.562-59.438L508.09,340L508.09,340z",
                 fillColor: sparkFill, strokeType: "center" });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // polygon - RIGHT
    var r1 = scene.create( { t: "path", parent: svg, id: "Middle_-_Right_2_", a:1, interactive: false,
                 d: "polygon points: 565.506,409.5 518.145,328 565.506,246.5 660.229,246.5 707.592,328 660.229,409.5",
                 fillColor: sparkHiLite, strokeColor: sparkFill, strokeWidth: 5, strokeType: "center" });

    // alpha solid - ORANGE
    var r2 = scene.create( { t: "path", parent: svg, id: "Color_-_Right_2_", a:0.75, interactive: false,
                 d: "polygon points: 575.868,391 538.827,327.5 575.868,264 649.869,264 686.909,327.5 649.869,391",
                 fillColor: "#F89958", strokeType: "center" });
    
    // path
    var r3 = scene.create( { t: "path", parent: svg, id: "Outline-_Right_2_", a:1.0, interactive: false,
                 d: "M647.43,268l34.561,59.5L647.43,387h-69.123l-34.562-59.5l34.562-59.5H647 M652.308,260h-4.878h-69.123" +
                 "h-4.878l-2.459,4.122l-34.562,59.162l-2.5,4.261l2.5,4.272l34.562,59.089l2.459,4.095h4.878h69.123h4.878l2.459-4.101" +
                 "l34.561-59.151l2.5-4.254l-2.5-4.269l-34.561-59.11L652.308,260L652.308,260z",
                 fillColor: sparkFill, strokeType: "center" });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // MIDDLE FILL
    var m1 = scene.create( { t: "path", parent: svg, id: "_x3C_Path_x3E__BOTTOM_1_", a:1.0,
                 d: "polygon points: 539.545,411 526.414,388 539.545,365 565.808,365 578.94,388 565.808,411",
                 fillColor: sparkFill, interactive: false});

    var m2 = scene.create( { t: "path", parent: svg, id: "_x3C_Path_x3E__BOTTOM", a:1.0,
                 d: "polygon points: 493.81,320.449 521.388,304.668 565.194,379.869 537.615,395.65",
                 fillColor: sparkFill, interactive: false});

    var m3 = scene.create( { t: "path", parent: svg, id: "_x3C_Path_x3E__LEFT", a:1.0,
                 d: "polygon points: 429.43,344 420.167,328 429.43,312 447.954,312 457.216,328 447.954,344",
                 fillColor: sparkFill, interactive: false});

    var m4 = scene.create( { t: "path", parent: svg, id: "_x3C_Path_x3E__LEFT_1_", a: 1.0,
                 d: "rect x:439.5 y:311.5 width:88 height:32",
                 fillColor: sparkFill, interactive: false});

    var m5 = scene.create( { t: "path", parent: svg, id: "_x3C_Group_x3E__RIGHT", a:1.0,
                 d: "polygon points: 546.34,277 537.078,261 546.34,245 564.864,245 574.126,261 564.864,277",
                 fillColor: sparkFill, interactive: false});

    var m6 = scene.create( { t: "path", parent: svg, id: "_x3C_Path_x3E__RIGHT_1_", a:1.0,
                 d: "polygon points: 521.104,351.388 493.525,335.606 537.331,260.404 564.91,276.186",
                 fillColor: sparkFill, interactive: false});

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    var spartText = scene.create({t:"object", id: "Spart Text", a: 1.0, w:1256, h:1256, parent: svg });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // "S"
    var S = scene.create( { t: "path", parent: spartText, id: "Glyph S", a:1.0, fillColor: sparkFill,
                            d: "M652.102,504.59c0,4.872-0.888,9.182-2.662,12.924c-1.775,3.745-4.212,6.857-7.305,9.338"+
                               "c-3.096,2.482-6.714,4.377-10.855,5.684c-4.143,1.307-8.581,1.963-13.312,1.963c-4.096,0-7.943-0.338-11.538-1.016"+
                               "c-3.597-0.676-6.874-1.488-9.831-2.436c-2.959-0.947-5.53-1.939-7.714-2.977s-3.96-1.916-5.325-2.64v-25.036l12.289-3.924"+
                               "l6.144,17.051c1.366,1.174,3.322,2.459,5.871,3.857c2.547,1.4,5.506,2.098,8.875,2.098c3.823,0,6.735-0.744,8.739-2.232"+
                               "c2.001-1.489,3.004-3.451,3.004-5.888c0-1.532-0.273-2.886-0.819-4.06c-0.546-1.172-1.365-2.254-2.458-3.248"+
                               "c-1.092-0.992-2.481-1.939-4.164-2.842c-1.686-0.9-3.71-1.848-6.076-2.842c-5.553-2.436-10.219-4.849-13.995-7.24"+
                               "c-3.778-2.389-6.827-4.871-9.148-7.443c-2.321-2.57-3.983-5.299-4.984-8.188c-1.002-2.886-1.502-6.043-1.502-9.473"+
                               "c0-4.51,1.137-8.411,3.414-11.705c2.274-3.293,5.073-6.023,8.397-8.188c3.322-2.166,6.872-3.767,10.65-4.805"+
                               "c3.776-1.036,7.168-1.557,10.172-1.557c6.827,0,12.788,0.812,17.887,2.436c5.097,1.625,8.92,3.204,11.469,4.737v23.276"+
                               "l-11.059,3.654l-5.052-16.104c-0.729-0.45-1.958-1.06-3.687-1.827c-1.73-0.766-4.506-1.15-8.329-1.15"+
                               "c-2.731,0-4.939,0.588-6.622,1.76c-1.686,1.174-2.526,2.797-2.526,4.871c0,1.174,0.158,2.233,0.478,3.181"+
                               "c0.318,0.947,0.956,1.874,1.912,2.774c0.956,0.902,2.253,1.85,3.892,2.842c1.638,0.994,3.823,2.076,6.554,3.248"+
                               "c4.732,2.165,8.92,4.286,12.562,6.36c3.64,2.076,6.691,4.31,9.148,6.698c2.457,2.393,4.322,5.055,5.598,7.984"+
                               "C651.463,497.441,652.102,500.801,652.102,504.59z",
                               strokeColor: sparkStroke, strokeWidth: 0 });
    // "P"
    var P = scene.create( { t: "path", parent: spartText, id: "Glyph P", a:1.0, fillColor: sparkFill,
                            d: "M734.979,500.53c0,6.496-1.024,11.909-3.072,16.239s-4.71,7.828-7.987,10.488"+
                               "c-3.277,2.662-6.919,4.533-10.924,5.616c-4.006,1.083-7.966,1.624-11.879,1.624c-3.549,0-6.28-0.203-8.191-0.609" +
                                "c-1.912-0.406-3.551-1.194-4.916-2.368v16.104l6.145,3.112v11.097h-34.408v-11.097l6.008-3.112v-65.5h-6.008v-11.908l28.264-5.143" +
                               "v7.443c2.002-1.983,4.438-3.562,7.305-4.736c2.867-1.172,6.029-1.76,9.49-1.76c3.912,0,7.69,0.677,11.332,2.03" +
                               "c3.64,1.353,6.872,3.43,9.694,6.226c2.82,2.797,5.052,6.359,6.69,10.69S734.979,494.487,734.979,500.53z M711.768,502.425" +
                               "c0-6.134-1.229-11.05-3.687-14.751c-2.458-3.697-5.734-5.549-9.831-5.549c-1.729,0-3.549,0.406-5.461,1.219" +
                               "s-3.506,2.029-4.779,3.654v28.283c1.001,2.256,2.343,3.789,4.028,4.602c1.683,0.812,3.161,1.264,4.438,1.354" +
                               "c3.004,0.09,5.483-0.519,7.441-1.828c1.956-1.307,3.526-2.953,4.71-4.939c1.182-1.982,2.002-4.06,2.458-6.225" +
                               "C711.539,506.078,711.768,504.14,711.768,502.425z",
                               strokeColor: sparkStroke, strokeWidth: 0 });
    // "A"
    var A = scene.create( { t: "path", parent: spartText, id: "Glyph A", a:1.0, fillColor: sparkFill,
                            d: "M780.037,535.039v-8.391c-2.457,2.978-5.349,5.031-8.67,6.158c-3.324,1.127-6.851,1.691-10.582,1.691" +
                               "c-2.457,0-4.848-0.294-7.168-0.88c-2.321-0.586-4.369-1.51-6.145-2.774c-1.775-1.262-3.209-2.887-4.301-4.871" +
                               "s-1.639-4.331-1.639-7.037c0-3.969,1.045-7.512,3.141-10.624c2.093-3.112,4.892-5.819,8.396-8.12" +
                               "c3.504-2.3,7.578-4.148,12.221-5.548c4.643-1.397,9.558-2.368,14.746-2.909v-6.226c0-2.074-0.662-3.563-1.98-4.466" +
                               "c-1.32-0.9-3.027-1.354-5.119-1.354c-1.73,0-3.551,0.295-5.462,0.88c-1.911,0.588-3.755,1.376-5.53,2.368" +
                               "c-1.774,0.994-3.437,2.121-4.983,3.383c-1.549,1.266-2.777,2.618-3.687,4.061l-4.232-0.947l-1.912-16.104" +
                               "c0.909-0.722,2.435-1.51,4.574-2.369c2.139-0.856,4.619-1.645,7.441-2.368c2.82-0.722,5.871-1.33,9.148-1.827" +
                               "c3.277-0.494,6.507-0.744,9.693-0.744c3.367,0,6.531,0.227,9.49,0.677c2.957,0.452,5.551,1.421,7.783,2.909" +
                               "c2.229,1.489,3.98,3.609,5.256,6.361c1.273,2.752,1.912,6.475,1.912,11.164v30.855h6.008v11.908L780.037,535.039z M780.037,502.02" +
                               "c-2.096,0-4.074,0.361-5.939,1.082c-1.867,0.723-3.482,1.624-4.848,2.707c-1.365,1.082-2.457,2.301-3.276,3.653" +
                               "c-0.819,1.354-1.229,2.753-1.229,4.195c0,2.077,0.615,3.722,1.844,4.939s2.752,1.874,4.574,1.962" +
                               "c1.912,0.182,3.64-0.044,5.188-0.676c1.547-0.631,2.775-1.442,3.687-2.437V502.02z",
                               strokeColor: sparkStroke, strokeWidth: 0 });
    // "R"
    var R = scene.create( { t: "path", parent: spartText, id: "Glyph A", a:1.0, fillColor: sparkFill,
                            d: "M857.727,489.84c-0.73-0.901-1.686-1.669-2.867-2.301c-1.185-0.631-2.458-1.083-3.823-1.354" +
                               "c-1.366-0.271-2.731-0.338-4.097-0.203s-2.594,0.521-3.686,1.15v31.803l6.144,3.111v11.098H814.99v-11.098l6.008-3.111V481.99" +
                               "h-6.008v-11.908l28.264-5.008v10.556c1.454-2.165,3.024-3.992,4.71-5.481c1.683-1.488,3.366-2.659,5.052-3.518" +
                               "c1.684-0.856,3.299-1.422,4.848-1.691c1.547-0.271,2.912-0.225,4.096,0.135v23.412L857.727,489.84z",
                                strokeColor: sparkStroke, strokeWidth: 0 });
    // "K"
    var K = scene.create( { t: "path", parent: spartText, id: "Glyph A", a:1.0, fillColor: sparkFill,
                            d: "M907.699,533.145v-11.098l4.096-3.111l-7.1-15.157l-7.919,7.037v8.12l6.144,3.111v11.098h-34.407v-11.098"+
                               "l6.008-3.111v-62.658h-6.008v-11.908l28.264-5.143v56.432l16.521-14.48l-5.598-2.977V467.24h32.633v10.961l-9.831,3.789"+
                               "l-10.649,9.608l16.111,27.337l6.008,3.111v11.098H907.699z",
                               strokeColor: sparkStroke, strokeWidth: 0 });

//    S.animateTo({ strokeWidth: 10 }, 1.05, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, -1);
//    P.animateTo({ strokeWidth: 10 }, 1.15, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, -1);
//    A.animateTo({ strokeWidth: 10 }, 1.25, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, -1);
//    R.animateTo({ strokeWidth: 10 }, 1.35, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, -1);
//    K.animateTo({ strokeWidth: 10 }, 1.45, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, -1);

    spartText.interactive = false; // ESSENTIAL

    svg.interactive = false; // ESSENTIAL

    return svg;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Letter "M" ...
  function drawSVG_LetterM()
  {
     var letterM = scene.create({t:"object", parent: bg, x:800, y:100, a: 1.0 });

     MM = scene.create( { t: "path",
                          d: "M 68.8 49" +
                             "V 21.9" +
                             "c 0 -8.3 -2.8 -11.5 -8.2 -11.5" +
                             "c -6.6 0 -9.3 4.7 -9.3 11.4" +
                             "v 16.8" +
                             "h 6.4" +
                             "V 49" +
                             "H 37.6" +
                             "V 21.9" +
                             "c 0 -8.3 -2.8 -11.5 -8.2 -11.5" +
                             "c -6.6 0 -9.3 4.7 -9.3 11.4" +
                             "v 16.8" +
                             "h 9.2" +
                             "V 49" +
                             "H 0" +
                             "V 38.7" +
                             "h 6.4" +
                             "V 11.4" +
                             "H 0.1" +
                             "V 1" +
                             "h 20" +
                             "v 7.2" +
                             "C 23 3.1 28 0 34.7 0" +
                             "C 41.6 0 48 3.3 50.4 10.3" +
                             "C 53 3.9 58.5 0 66 0" +
                             "c 8.6 0 16.5 5.2 16.5 16.6" +
                             "v 22" +
                             "h 6.4" +
                             "V 49" +
                             "H 68.8" +
                             "Z",

        strokeColor: 0x000000ff, strokeWidth: 4, fillColor: 0x00FFFFff, parent: letterM, cx: 50, cy: 50} );

    return letterM;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Multi-color Cross ... (relative)
  function drawSVG_Cross()
  {
    var sw = 6;
  
    var cross = scene.create({t:"object", parent: bg, x:200, y:300, a: 1.0 });

    scene.create( { t: "path", d:"M100 100 v-50", strokeColor: 0xFF00FFff, strokeWidth: sw, parent: cross } );
    scene.create( { t: "path", d:"M100 100 v50",  strokeColor: 0x00FF00ff, strokeWidth: sw, parent: cross } );
    scene.create( { t: "path", d:"M100 100 h50",  strokeColor: 0x0000FFff, strokeWidth: sw, parent: cross } );
    scene.create( { t: "path", d:"M100 100 h-50", strokeColor: 0x00FFFFff, strokeWidth: sw, parent: cross } );

    return cross;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //     CIRCLE TESTING - see [test.svg]
  //
  //     <circle cx="50" cy="50" r="40" stroke="black" stroke-width="3" fill="red" />
  //
  //     <path d="M 191 140
  //              C 191 163   173 181  150  181
  //              C 127 181   109 163  109  140
  //              C 109 117   127  99  150   99
  //              C 173  99   191 117  191  140Z" stroke="black" stroke-width="3"  fill="cyan"/>
  function drawSVG_Circle()
  {
        var circle = scene.create( { t: "path", d:"circle cx:50 cy:50 r:50", x: 150, y: 250,
                           strokeColor: 0x000000ff, strokeWidth: 10, strokeType: "inside", fillColor: 0x00FFFFff, parent: bg} );

    // Part of the converted arcs
    //      scene.create( { t: "path", d:"M191 140 C 191 163 173 181  150  181", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );

    return circle;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //     ELLIPSE TESTING - see [test.svg]
  //
  //    <ellipse cx="60" cy="60" rx="50" ry="25" stroke="black" stroke-width="3" fill="red" />
  //
  //    <path d="M 200 340 C 200 354 178 365 150 365
  //             C 122 365 100 354 100 340
  //             C 100 326 122 315 150 315
  //             C 178 315 200 326 200 340Z" stroke="black" stroke-width="3"  fill="cyan"/>

  function drawSVG_Ellipse()
  {
    var ellipse = scene.create( { t: "path", d:"ellipse cx:50 cy:25 rx:50 ry: 25", x: 200, y: 200,
                        strokeColor: 0x000000ff, strokeWidth: 4, fillColor: 0xFFFF00ff, parent: bg} );

    return ellipse;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //     POLYGON TESTING - see [test.svg]
  //
  //    <polygon points="200,10 250,190 160,210" style="fill:lime;stroke:purple;stroke-width:1"/>
  //
  
  function drawSVG_Polygon()
  {
      var polygon = scene.create( { t: "path", d:"polygon  points:30,0 60,60 0,60",
                          strokeColor: 0x000000ff, strokeWidth: 2, fillColor: 0xFFFF00ff, parent: bg, x: 50, y: 50} );

     return polygon;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    ROUNDED RECT - see [test.svg]
  //
  //    <rect x="250" y="250" rx="50" ry="50" width="400" height="200"
  //         style="fill:red;stroke:black;stroke-width:5;opacity:0.5" />
  //
  function drawSVG_RRect()
  {
    var x0 = 400, y0 = 250, w = 100, h = 100, rx = 30, ry = 30, sw = 10;

    //var  referenceBG = scene.create( { t: "path", d:"rect x:"+x0+" y:"+y0+" width:"+w+" height:"+h, fillColor: 0x0000FFff, parent: bg} );
//     var  rrect = scene.create( { t: "path", d:"rect x:"+x0+" y:"+y0+" width:"+w+" height:"+h, strokeColor: 0x00000058, strokeWidth: sw, fillColor: 0x00FF00ff, parent: bg, a:1.0} );
    var rrect = scene.create( { t: "path", d:"rect x: 0, y: 0, width:"+w+" height:"+h+" rx:"+rx+" ry:"+ry+"", x: x0, y: y0,
                                    strokeColor: 0x00000058, strokeWidth: sw, strokeType: "inside", fillColor: 0x00FF00ff, parent: bg, a:1.0} );
    //scene.create( { t: "path", d:"rect x:"+x0+" y:"+(y0 + 150)+" width:"+w+" height:"+h+"", strokeColor: 0xFF0000ff, strokeWidth: 8, fillColor: 0x8888FFff, parent: bg} );

    rrect.cx = rrect.w/2;
    rrect.cy = rrect.h/2;
    
    rrect.ready.then( function(o)
    {
      rrect.animateTo({ x: 650, r: 180 }, 1.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, -1);
      console.log("\n########### rect2 WxH:  "+rect2.w+" x "+rect2.h);
    });

    return rrect;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    ARC: absolute
  //
  //    Example from: ARC section >>  https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
  //
  function drawSVG_ARC1()
  {
    var arc1 = scene.create({t:"object", parent: bg, x:300, y:100, a: 1.0 });
              
    scene.create( { t: "path", d:"M10 315 L 315 10", strokeColor: 0xFF0000ff, strokeWidth: 3,  parent: arc1} );
    scene.create( { t: "path", d:"M10 315 "
                                +"L 110 215 "
                                +"A 30 50 0 0 1 162.55 162.45 "
                                +"L 172.55 152.45 "
                                +"A 30 50 -45 0 1 215.1 109.9 "
                                +"L 315 10Z",strokeWidth: 2, strokeColor: 0x000000ff, fillColor: 0x00FF0080,  parent: arc1} );

    return arc1;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    ARC: relative
  //
  //    Example from: ARC section >>  https://www.w3.org/TR/SVG/paths.html#PathDataEllipticalArcCommands
  //
  function drawSVG_ARC2()
  {
    var arc2 = scene.create( { t: "path", d:"M600 350 l 50 -25 "
                                +"a25 25  -30 0 1 50 -25 l 50 -25 "
                                +"a25 50  -30 0 1 50 -25 l 50 -25 "
                                +"a25 75  -30 0 1 50 -25 l 50 -25 "
                                +"a25 100 -30 0 1 50 -25 l 50 -25Z",
                 fillColor: 0x00000000, strokeColor: 0xFF0000ff, strokeWidth: "5",  parent: bg } );

    return arc2;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    ARC SWEEP - 4 possible values
  //
  //    Example from: ARC section >>  https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
  //
  function drawSVG_SWEEP()
  {
    var obj = scene.create({t:"object", parent: bg, x:600, y:300, a: 1.0 });

    scene.create( { t: "path", d: "M80 80 "
                                 +"A 45 45  0  0  0  125 125 "
                                 +"L 125 80 Z",
                 fillColor: 0x00FF00ff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

    scene.create( { t: "path", d: "M230 80 "
                                 +"A 45 45  0  1  0  275 125 "
                                 +"L 275 80 Z",
                 fillColor: 0xFF0000ff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

    scene.create( { t: "path", d: "M80 230 "
                                 +"A 45 45  0  0  1  125 275 "
                                 +"L 125 230 Z",
                 fillColor: 0xFF00FFff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

    scene.create( { t: "path", d: "M230 230 "
                                 +"A 45 45  0  1  1  275 275 "
                                 +"L 275 230 Z",
                 fillColor: 0x0000FFff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

    return obj;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    Curves
  //
  // These curves should overlap, Black Below, Red Above
  //
  function drawSVG_CURVES()
  {
    var p = scene.create({t:"object", parent: bg, x: 0, y:0, a: 1.0 });

    var  curveBLACK = scene.create( { id: "CurveBG", t: "path", d:"M100,250 Q250,100 400,250",  strokeColor: 0x000000ff, strokeWidth: 13, parent: p, strokeType: "center"} );
    var  curveGREEN = scene.create( { id: "CurveFG", t: "path", d:"M100,250 q150,-150 300,0",   strokeColor: 0x00FF00ff, strokeWidth: 3,  parent: p, strokeType: "center" } );

    scene.create( { id: "CurveBG", t: "path", d:"M100 180 q50 10, 95 80",     strokeColor: 0x000000ff, strokeWidth: 13, parent: p, strokeType: "outside"} );
    scene.create( { id: "CurveFG", t: "path", d:"M100 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 3,  parent: p, strokeType: "outside" } );

    var    strokeClr = 0x888888ff;
    var  rectInside  = scene.create( { id: "rectInside",  t: "path", d:"rect x:0 y:0 width:100 height: 100", x:20 , y:200 + 150,  strokeColor: strokeClr, strokeWidth: 6, strokeType: "inside",  fillColor: 0x00FF00ff, parent: p, a:1.0} );
    var  rectOutside = scene.create( { id: "rectOutside", t: "path", d:"rect x:0 y:0 width:100 height: 100", x:220, y:200 + 150,  strokeColor: strokeClr, strokeWidth: 6, strokeType: "outside", fillColor: 0x00FF00ff, parent: p, a:1.0} );
    var  rectCenter  = scene.create( { id: "rectCenter",  t: "path", d:"rect x:0 y:0 width:100 height: 100", x:420, y:200 + 150,  strokeColor: strokeClr, strokeWidth: 6, strokeType: "center",  fillColor: 0x00FF00ff, parent: p, a:1.0} );
    var         line = scene.create( { id: "Line",        t: "path", d:"M10 200 H530", x:0 , y:150, strokeColor: 0xFF0000ff, strokeWidth: 1, strokeType: "center", parent: p} );

    Promise.all([rectInside.ready, rectOutside.ready, rectCenter.ready, line.ready])
    .then( function(o)
    {
      var textData = [ { rect: rectInside},  { rect: rectOutside},  { rect: rectCenter} ];

      var textReady = textData.map(data =>
            {
               var rr = data.rect;
               var tt = ("WxH: " + rr.w + " x " +  rr.h);

               var txt1 = scene.create({t:"textBox", parent: rr, pixelSize: pts, w: rr.w, h: 50, x: 0, y: rr.y/3,
                                 alignHorizontal: scene.alignHorizontal.CENTER, interactive: false, text: rr.id,
                                   alignVertical:     scene.alignVertical.CENTER, textColor: textClr, a: 1.0});

               var txt2 = scene.create({t:"textBox", parent: txt1, pixelSize: pts, w: rr.w, h: 50, x: 0, y: 15,
                                 alignHorizontal: scene.alignHorizontal.CENTER, interactive: false, text: tt,
                                   alignVertical:     scene.alignVertical.CENTER, textColor: textClr, a: 1.0});
            });//MAP
    }); // PROMISE

    return p;
  }

//##################################################################################################################################

  bg.on("onMouseUp", function(e)
  {
    if(false)
    {
//##################################################################################################################################
//
//------ LINE/MOVE
//
        var ss = 2;
        
//      var shape1 = scene.create( { id: "Path (0,0)",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                x: 0, y: 0, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
//      var shape2 = scene.create( { id: "Path (50,50)",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                x: 50, y: 50, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
//      var shape3 = scene.create( { id: "Path (100,100)",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                x: 100, y: 100, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
      
//       scene.create( { t: "path", d:"M100 100 L200 200",                         fillColor: 0x00FF88ff, parent: bg} );
//    var shape = scene.create( { t: "path", d:"M100 100 L200 100 200 200 100 200 100 100", strokeColor: 0xFF000Fff, strokeWidth: 6, parent: bg} );

//        var shape = scene.create( { id: "Path ("+e.x+","+e.y+")",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                 x: e.x-50, y: e.y-50, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
        
//        var shape = scene.create( { id: "Path ("+e.x+","+e.y+")",  t: "path", d:"M" +(e.x-50)+ " " +(e.y-50)+ " l100 0 0 100 -100 0 0 -100", interactive: false,
//                                  w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
        
      var shape = scene.create( { id: "Path - triangle ("+(e.x-50)+","+(e.y-50)+")",  t: "path", d:"M50 0 L100 100  0 100  50 0", interactive: false,
                             x: e.x-50, y: e.y-50, fillColor: 0xFF000088, strokeColor: 0x000000ff, strokeWidth: 6, parent: bg, cx: 50, cy: 50} );

        console.log("Click >>  " + shape.id );
        
//        var shape = scene.create( { t: "path", d:"circle cx:150 cy:140 r:50", x: e.x-50, y: e.y-50,cx: 50, cy: 50, strokeColor: 0x000000ff, strokeWidth: 4,  fillColor: 0x00FF00ff,  parent: bg} );
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //  Animation
        //
        if(true)
        {
          shape.ready.then
          (
              function(o)
              {
           MM.r = 0;
                MM.animateTo({ r: 360 }, 1.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
                o.animateTo({ r: 360 }, 1.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
               // var px = o.x + 100;
               // o.animateTo({ x: px }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);

               // var ww = o.w + 100;
               // o.animateTo({ w: ww }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
              } );
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
//        scene.create( { t: "path", d:"M100 100 v-50", x: 100, y:  50, w: 50, h: 50, strokeColor: 0xFF00FFff, strokeWidth: 4,  parent: bg} );
//        scene.create( { t: "path", d:"M100 100 v50",  x: 100, y: 150, w: 50, h: 50, strokeColor: 0x00FF00ff, strokeWidth: 4,  parent: bg} );
//        scene.create( { t: "path", d:"M100 100 h50",  x: 150, y: 100, w: 50, h: 50, strokeColor: 0x0000FFff, strokeWidth: 4,  parent: bg} );
//        scene.create( { t: "path", d:"M100 100 h-50", x:  50, y: 100, w: 50, h: 50, strokeColor: 0x00FFFFff, strokeWidth: 4,  parent: bg} );

//        scene.create( { t: "path", d:"M100 100 h-50 M100 100 v50 M100 100 h50 M100 100 v-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative

//##################################################################################################################################
//
//------ VERTICAL
//
//       scene.create( { t: "path", d:"M100 100 v-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative
//       scene.create( { t: "path", d:"V200", strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // absolute
             
//##################################################################################################################################
//
//------ HORIZONTAL
//
//       scene.create( { t: "path", d:"M100 100 h-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative
//       scene.create( { t: "path", d:"H200", strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // absolute
             
//##################################################################################################################################
//
//------ QUADRATIC CURVE
        
//        scene.create( { t: "path", d:"M10 80 Q 50 10, 95 80 T 180 80 270 110", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );  // relative
//        scene.create( { t: "path", d:"M10 80 Q 50 10, 95 80 T 180 80", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );  // relative

//         These curves should overlap, Black Below, Red Above
//        scene.create( { id: "CurveBG", t: "path", d:"M100 180 q50 10, 95 80",     strokeColor: 0x0000001f, strokeWidth: 9, parent: bg} );  // relative
//        scene.create( { id: "CurveFG", t: "path", d:"M100 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 3, parent: bg} );  // absolute

     //   scene.create( { id: "CurveFG1", t: "path", d:"M110 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 13, parent: bg} );  // absolute
//        scene.create( { id: "CurveFG1", t: "path", d:"M110 180 Q150 190, 195 260", strokeColor: 0x000000ff, strokeWidth: 3,  parent: bg} );  // absolute

//##################################################################################################################################
//
//------ CUBIC CURVE
//
//             var curve = scene.create( { t: "path", d:"M100 180 c100,100 250,100 250,200", strokeColor: 0x00FF88ff, strokeWidth: 3, parent: bg} );  // relative
//             var curve = scene.create( { t: "path", d:"M0 0 c100,100 250,100 250,200", strokeColor: 0x00FF88ff, strokeWidth: 3, parent: bg, x: 200, y: 400, w:10, h: 10 } );  // relative
//         scene.create( { t: "path", d:"M100,200 C100,100 250,100 250,200", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );  // absolute
//         var curve = scene.create( { t: "path", d:"M100,200 C100,100 250,100 250,200 S400,300 400,200", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );  // absolute
 
//         curve.ready.then
//         (
//          function(o)
//          {
//             curve.w = 310; curve.h = 210;
//
//             curve.animateTo({ x: 400.0  }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1).then
//             (
//              function(o) {
//              
//              }
//              );
//         }
//        );
             
//             scene.create( { t: "path", d:"M0.330689 0 c -0.182634,0 -0.330689,0.148055 -0.330689,0.330689 0,0.182638 0.148055,0.330689 0.330689,0.330689 0.182634,0 0.330689,-0.148051 0.330689,-0.330689 0,-0.182634 -0.148055,-0.330689 -0.330689,-0.330689z", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );
             
//          scene.create( { t: "path", d:"M89.171,155.172c2.668-0.742,4.23-3.531,3.475-6.238c-0.76-2.678-3.549-4.266-6.265-3.492 c-2.669,0.76-4.235,3.568-3.476,6.24C83.673,154.365,86.454,155.948,89.171,155.172z", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );
     
//             scene.create( { t: "path", d:"rect( rx:'15', ry:'15' )", strokeColor: 0x00FF00ff, strokeWidth: 3, parent: bg, x: 600, y: 800, w: 300, h: 150;} );
//         scene.create( { t: "path", d:"M50 200 L350 200", strokeColor: 0x00FF00ff, strokeWidth: 3, parent: bg} );
             
//       scene.create( { t: "path", d:"M10 80 l100 0   0 100   -100 0   0 -100", fillColor: 0x00FF88ff, parent: bg} );
             
//       scene.create( { t: "path", d:"M10 80 Q50 10, 95 80 T 180 80 270 110", fillColor: 0x00FF88ff, parent: bg} );
//       scene.create( { t: "path", d:"M10 80 Q50 10, 95 80 T 180 80 270 110 M100 100 L200 200", fillColor: 0x00FF88ff, parent: bg} );
        
//##################################################################################################################################
        
// JUNK
//
//    if(false)
//    {
//      scene.create( { t: "path", d:"M300 600 l44.629 -10.01 42.188 -10.01 37.036 -8.618 35.498 -5.029 37.036 -1.392 42.188 0 55.664 0 60.791 -1.392 "
//                                 + "62.305 -4.98 60.059 -9.229 55.42 -10.01 35.498 -59.18 13.867 -59.229 8.716 -57.837 7.178 -54.199 8.716 -50.562 "
//                                 + "13.867 -49.17 20.41 -49.17 4.688 -10.01 -0.635 -8.691 -2.246 -5.029 -0.708 -1.392 4.443 0 17.92 0 23.071 -1.392 "
//                                 + "24.609 -4.98 23.071 -8.618 17.92 -10.01 15.479 -10.01 17.92 -16.113 42.236, -16.113Z",
//                      strokeColor: 0x000000ff,  strokeWidth: 7, strokeWidth: 3, fillColor: 0x00FF88ff, parent: bg} );
//    }
        
//        scene.create( { t: "path", d:"M 285.25596,665.33856 C 654.07555 712.27292 58.724179,415.02235 405.53052 458.53782",
//                     strokeColor: 0x000000ff,  x: e.x, y: e.y - 200, strokeWidth: 7,fillColor: 0x00FF88ff, parent: bg} );
    }
  });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var panelShowing = false;

function showPanel()
{
  if(panelVisible == true) return; // Already SHOWN

  if(panelShowing) return;
  panelShowing = true; // one shot

//  console.log("Show PANEL !");

  if(panelAnimate != null)
  {
     console.log("Cancel Animation PANEL ! (gonna SHOW)");
     panelAnimate.cancel();
  }

  buttonsPanel.interactive = false;

  var py = (bg.h - buttonsPanel.h);      // Final Position
  var dy = (py   - buttonsPanel.y);      // Distance to move
  var tt = Math.abs((dy) * panelSpeed);  // Time to get there

  //console.log("SHOW >> Panel Time: "+ tt + "   panelSpeed: "+ panelSpeed);
  //console.log("SHOW >> Panel DY: "+ dy + "   PY: "+ py+ "   tt: "+ tt);

  panelAnimate = buttonsPanel.animate({ a: 1.0, y: py }, tt, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
  panelAnimate.done.then( function(o)
  {
     panelAnimate = null;
     panelVisible = true;

     panelShowing = false;
     panelHiding  = false;
  });
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //var panelSpeed  = 1.25 / buttonsPanel.h ; // px per second 
  var panelSpeed  = 0.55 / buttonsPanel.h ; // px per second 
  var panelHiding = false;

  function hidePanel()
  {
    if(panelVisible == false) return; // Already HIDDEN

    if(panelHiding) return;
    panelHiding = true; // one shot

    if(panelAnimate != null)
    {
       console.log("Cancel Animation PANEL ! (gonna HIDE)");
       panelAnimate.cancel();
    }

    buttonsPanel.interactive = false;

    var py = (bg.h               );        // Final Position
    var dy = (py - buttonsPanel.y);        // Distance to move
    var tt = Math.abs((dy) * panelSpeed);  // Time to get there

  //  console.log("HIDE >> Panel Time: "+ tt + "   panelSpeed: "+ panelSpeed);
  //  console.log("HIDE >> Panel DY: "+ dy + "   PY: "+ py+ "   tt: "+ tt);

    panelAnimate = buttonsPanel.animate({ a: 1.0, y: py }, tt, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
    panelAnimate.done.then( function(o)
    {
        panelAnimate = null;
        panelVisible = false;
  
        panelShowing = false;
        panelHiding  = false; 
    });
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bg.on("onMouseMove",  function (e)
  {
     //console.log("Mouse Y: " + e.y);
  
     var threshold_y = (bg.h - buttonsPanel.h);
  
     if(e.y > threshold_y)
     {
       showPanel();
     }
     else
     {
       hidePanel();
     }
  });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  buttonsPanel.on("onMouseMove",  function (e)
  {
     e.stopPropagation(); // Eat MouseMove
  });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  var layout = () => 
  {
    var px = 0, py = 0;

    var buttonsW = (buttonData.length * bw);
    var spare_x  = (bg.w - buttonsW);

    // Compute GAP and Initial Position
    gap = spare_x / (buttonData.length + 1);
    px  = gap;
    py  = (buttonsPanel.h - bh)/2;

    for(var key in buttonObjects)
    {
      var o = buttonObjects[key];

      if (!o) continue;
      if (px + bw > bg.w)   { px = gap;  /*py += side;*/  } // NEXT ROW

      // TODO should we have an option to not cancel
      // animations if targets haven't changed
      if (o.buttonObj.x != px || o.buttonObj.y != py)
      {
        o.buttonObj.animateTo({x: px, y: py }, 0.5, scene.animation.TWEEN_STOP).catch(() => {});
            // .ready.then( function(o) { 
            //       hidePanel(); });

        o.x = px; // target X
        o.y = py; // target Y
      }
      px += (bw + gap);

//       console.log("buttonsW: "+buttonsW+" bg: "+ bg.w + "  spare_x: "+ spare_x+ "  buttons: "+ buttonData.length);
//       console.log("gap: "+gap+" bw: "+ bw +"  px >> " + px);
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function buttonToggle(b)
  {
        if(b.svgObj != null)  b.svgObj.remove();   // Turning OFF

                                                  //   Turn ON      :  Turn OFF
        b.buttonObj.fillColor    = (b.svgObj == null) ?   fillClr_ON : fillClr_OFF;
        b.buttonObj.strokeWidth *= (b.svgObj == null) ?          2.0 : 0.5;
        b.svgObj                 = (b.svgObj == null) ? eval(b.func) : null;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function updateSize(w,h)
  {
     bg.w = w;
     bg.h = h;

    buttonsPanel.w = bg.w;
    buttonsPanel.x = 0;
    buttonsPanel.y = bg.h - buttonsPanel.h;

    layout();

    hidePanel();
  }

  scene.on("onResize",    function(e) { updateSize(e.w, e.h); });

  Promise.all([ bg.ready])
      .catch( (err) =>
      {
          console.log("SVG >> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
          console.log("\n START SVG >> Children of [bg] = " + bg.children.length);

          updateSize(scene.w, scene.h);
      });

}).catch( function importFailed(err){
  console.error("SVG >> Import failed for svgtest.js: " + err);
});
