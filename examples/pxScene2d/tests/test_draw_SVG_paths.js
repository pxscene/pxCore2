"use strict";

// Import frameworks
var frameworks = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/frameworks/";
px.configImport({"frameworks:":frameworks});

px.import({  scene:"px:scene.1.js",
             check:'frameworks:checks.js',
            assert:"../test-run/assert.js",
             shots:"../test-run/tools_screenshot.js",
            manual:"../test-run/tools_manualTests.js"

}).then( function importsAreReady(imports)
{
  var scene  = imports.scene;
  var checks = imports.check;
  var root   = imports.scene.root;
  var assert = imports.assert.assert;
  var shots  = imports.shots;
  var manual = imports.manual;

  var results = [];

  var doScreenshot   = false;
  var manualTest     = manual.getManualTestValue();
  var basePackageUri = px.getPackageBaseFilePath();

  var bg = scene.create({t:"rect", parent: root, x:  0, y:   0, w: 100, h: 100, fillColor: 0xFFFFFFff, a: 1.0 });

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  var doScreenshotComparison = function(name, expectedScreenshotImage, results, printScreenshotToConsole,resolve, reject) 
  {
        shots.validateScreenshot(expectedScreenshotImage, printScreenshotToConsole)
        .then(function(match)
        {
          console.log("Image Comparison result is match: " + match);
          results.push(assert(match == true, "screenshot comparison for [ " + name + " ] failed"));
          resolve(results);

        }).catch(function(err)
        {
          results.push(assert(false, "screenshot comparison for [ " + name + " ] failed due to error: " + err));
          resolve(results);
      });
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  var tests =
  {
    drawSVG_all_test: function()
    {
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
      if( checks.checkType("path") == false) // SVG supported ?
      {
          return new Promise (function(resolve, reject)
          {
            console.log("ERROR: SVG is NOT Supported in this build.");
            results.push(assert(true, "SVG is NOT Supported in this build."));
            /*reject*/resolve(results);  // "GOOD" Fail
          });
      }
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

      return new Promise(function(resolve, reject)
      {
        var assets = drawSVG_all();

        Promise.all(assets)
        .then( function() // resolve
        {
            //shots.takeScreenshot(true); // Gold Master
            if( doScreenshot )
            {
              setTimeout(function()
              {
                var expectedImage = basePackageUri+"/images/screenshot_results/test_draw_SVG_paths.png";

                doScreenshotComparison("draw_svg_paths", expectedImage, results, false, resolve, reject);

              }, 1000);
            } else
            {
              resolve(results);  // success
            }
        }, 
        function() // reject
        {
          console.log("ERROR: drawSVG_all_test:  >>  create() failed ! results: " + results);

          /*reject*/resolve(results); // fail
        });
      });
    }//drawSVG_all_test
  }//tests[]

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  module.exports.tests = tests;

  if(manualTest === true)
  {
    manual.runTestsManually(tests);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function drawSVG_all()
  {
      var promises = [];

      promises.push( drawSVG_SparkPage() );
      promises.push( drawSVG_SparkLogo() );
      promises.push( drawSVG_LetterM() );
      promises.push( drawSVG_Cross() );
      promises.push( drawSVG_Circle() );
      promises.push( drawSVG_Ellipse() );
      promises.push( drawSVG_RRect() );
    // promises.push(  drawSVG_ARC1() );
    // promises.push(  drawSVG_ARC2() );
      promises.push( drawSVG_SWEEP() );
      promises.push( drawSVG_CURVES() );

      return promises;
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Spark Page ...
  function drawSVG_SparkLogo()
  {
    try
    {
      var svg    = scene.create({t:"object", parent: bg, x:850, y:30, a: 1.0, w:1256, h:800, sx: 0.5, sy: 0.5});
      var layer1 = scene.create({t:"path", id: "layer1", parent: svg });

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // polygon - TOP
      var t1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Top_2_", a:1,
                      d: "polygon points: 83.797,283.5 2.885,143 83.797,2.5 245.623,2.5 326.536,143 245.623,283.5",
              fillColor: "#E6E7E8", strokeColor: "#5D6D65", strokeWidth: 5} );
                      
      // alpha solid - RED
      var t2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Top_2_", a:0.75,
                      d: "polygon points: 101.463,252 38.201,143 101.463,34 227.958,34 291.22,143 227.958,252",
              fillColor: "#F16268"} );

      // path
      var t3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Top_1_", a:1.0,
                      d: "M223.755,41l59.044,102l-59.044,102H105.666L46.622,143l59.044-102H224.08 M232.161,26h-8.406H105.666" +
                              "H97.26l-4.21,7.454L34.005,135.57l-4.226,7.346l4.226,7.323l59.044,102.256L97.26,260h8.406h118.089h8.406l4.21-7.493" +
                              "l59.044-102.137l4.226-7.355l-4.226-7.328L236.372,33.468L232.161,26L232.161,26z",
              fillColor: "#5D6D65"} );

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // polygon - BOTTOM
      var b1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Bottom_2_", a:1,
                      d: "polygon points: 83.799,568.5 2.887,428 83.799,287.5 245.625,287.5 326.539,428 245.625,568.5",
              fillColor: "#E6E7E8", strokeColor: "#5D6D65", strokeWidth: 5} );

      // alpha solid - YELLOW
      var b2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Bottom_2_", a:0.75,
                      d: "polygon points: 101.465,533 38.203,423.501 101.465,314 227.961,314 291.223,423.501 227.961,533",
              fillColor: "#FAEF5F"} );

      // path
      var b3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Bottom_2_", a:1.0,
                      d: "M223.758,321l59.044,102.001L223.758,525h-118.09L46.624,423.001L105.668,321H224.08 M232.164,307h-8.406" +
                              "h-118.09h-8.406l-4.21,7.181l-59.044,101.98l-4.226,7.278l4.226,7.289L93.051,532.74l4.21,7.26h8.406h118.09h8.406l4.21-7.267"+
                              "l59.044-102.021l4.226-7.299l-4.226-7.3l-59.044-101.933L232.164,307L232.164,307z",
              fillColor: "#5D6D65"} );

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // polygon - RIGHT
      var r1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Right_2_", a:1,
                      d: "polygon points: 330.181,425.5 249.268,285.5 330.181,145.5 492.005,145.5 572.918,285.5 492.005,425.5",
              fillColor: "#E6E7E8", strokeColor: "#5D6D65", strokeWidth: 5} );

      // alpha solid - ORANGE
      var r2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Right_2_", a:0.75,
                      d: "polygon points: 347.846,394 284.583,284.5 347.846,175 474.341,175 537.603,284.5 474.341,394",
              fillColor: "#F89958"} );

      // path
      var r3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Right_2_", a:1.0,
                      d: "M470.138,182l59.044,102l-59.044,102H352.049l-59.044-102l59.044-102H470.08 M478.544,168h-8.406H352.049" +
                              "h-8.406l-4.21,7.205l-59.044,101.992l-4.226,7.283l4.226,7.292l59.044,101.99l4.21,7.237h8.406h118.089h8.406l4.21-7.242" +
                              "l59.044-102.011l4.226-7.292l-4.226-7.297l-59.044-101.954L478.544,168L478.544,168z",
              fillColor: "#5D6D65"} );

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // MIDDLE FILL
      var m1 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__BOTTOM_1_", a:1.0,
                      d: "polygon points: 285.83,428 263.396,389 285.83,350 330.697,350 353.131,389 330.697,428",
              fillColor: "#5D6D66"} );

      var m2 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__BOTTOM", a:1.0,
                      d: "polygon points: 207.694,272.275 254.81,245.08 329.648,374.672 282.532,401.868",
              fillColor: "#5C6D66"} );

      var m3 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__LEFT", a:1.0,
                      d: "polygon points: 97.708,312 81.884,285 97.708,258 129.354,258 145.178,285 129.354,312",
              fillColor: "#5C6D66"} );

      var m4 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__LEFT_1_", a: 1.0,
                      d: "rect x:115.58 y:258.5 width:150 height:54",
              fillColor: "#5C6D65"} );

      var m5 = scene.create( { t: "path", parent: layer1, id: "_x3C_Group_x3E__RIGHT", a:1.0,
                      d: "polygon points: 297.438,197 281.614,170 297.438,143 329.084,143 344.908,170 329.084,197",
              fillColor: "#5C6D65"} );

      var m6 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__RIGHT_1_", a:1.0,
                      d: "polygon points: 254.324,325.59 207.208,298.395 282.046,168.802 329.162,195.998",
              fillColor: "#5C6D65"} );

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      return new Promise(function(resolve, reject)
      {
        var assets = [t1.ready,t2.ready,t3.ready, 
                      b1.ready,b2.ready,b3.ready, 
                      r1.ready,r2.ready,r3.ready, 
                      m1.ready,m2.ready,m3.ready,m4.ready,m5.ready,m6.ready];

        Promise.all(assets)
        .then( function()
        {
          resolve(); // success
        },
        function()
        {
          console.log("ERROR:  test_drawSVG_paths.js >>  drawSVG_SparkPage()  creations failed !");
          reject(); // fail
        })
        .catch(function(error)
        {
          console.log("ERROR:  test_drawSVG_paths.js >>  drawSVG_SparkPage()  creations exception !");
          reject(); // fail
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject) {
        reject("ERROR:  test_drawSVG_paths.js >>  drawSVG_SparkPage()  >>> SVG not supported");
      });
    }
  } // drawSVG_SparkPage()

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Spark Page ...
  function drawSVG_SparkPage()
  {
    try
    {
      var svg    = scene.create({t:"object", parent: bg, x:10, y:-30, a: 1.0, w:1256, h:800, sx: 0.45, sy: 0.45});
      var layer1 = scene.create({t:"path", id: "layer1", parent: svg });

      // polygon - TOP
      var t1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Top_2_", a:1,
                  d: "polygon points: 421.288,326.5 373.926,245.5 421.288,164.5 516.011,164.5 563.373,245.5 516.011,326.5",
                  fillColor: "#E6E7E8", strokeColor: "#5D6D65", strokeWidth: 5} );

      // alpha solid - RED
      var t2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Top_2_", a:0.75,
                  d: "polygon points: 431.649,309 394.608,245.5 431.649,182 505.649,182 542.69,245.5 505.649,309",
                  fillColor: "#F16268"} );

      // path
      var t3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Top_1_", a:1.0,
                  d: "M503.21,186l34.562,59.5L503.21,305h-69.123l-34.561-59.5l34.561-59.5H503 M508.088,178h-4.878h-69.123" +
                  "h-4.878l-2.459,4.065l-34.562,59.134l-2.5,4.246l2.5,4.265l34.562,59.142L429.21,313h4.878h69.123h4.878l2.459-4.157" +
                  "l34.562-59.179l2.5-4.269l-2.5-4.276l-34.562-59.057L508.088,178L508.088,178z",
                  fillColor: "#5D6D65"} );

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      // polygon - BOTTOM
      var b1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Bottom_2_", a:1,
                  d: "polygon points: 421.289,491.5 373.927,410.5 421.289,329.5 516.012,329.5 563.374,410.5 516.012,491.5 ",
                  fillColor: "#E6E7E8", strokeColor: "#5D6D65", strokeWidth: 5} );

      // alpha solid - YELLOW
      var b2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Bottom_2_", a:0.75,
                  d: "polygon points: 431.65,472 394.609,408.5 431.65,345 505.651,345 542.692,408.5 505.651,472",
                  fillColor: "#FAEF5F"} );

      // path
      var b3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Bottom_2_", a:1.0,
                  d: "M503.212,349l34.562,59l-34.562,59h-69.123l-34.561-59l34.561-59H503 M508.09,340h-4.878h-69.123h-4.878" +
                  "l-2.459,4.439l-34.562,59.319l-2.5,4.34l2.5,4.312l34.562,59.291l2.459,4.299h4.878h69.123h4.878l2.459-4.284l34.562-59.241" +
                  "l2.5-4.302l-2.5-4.292l-34.562-59.438L508.09,340L508.09,340z",
                  fillColor: "#5D6D65"} );

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // polygon - RIGHT
      var r1 = scene.create( { t: "path", parent: layer1, id: "Middle_-_Right_2_", a:1,
                  d: "polygon points: 565.506,409.5 518.145,328 565.506,246.5 660.229,246.5 707.592,328 660.229,409.5",
                  fillColor: "#E6E7E8", strokeColor: "#5D6D65", strokeWidth: 5} );

      // alpha solid - ORANGE
      var r2 = scene.create( { t: "path", parent: layer1, id: "Color_-_Right_2_", a:0.75,
                  d: "polygon points: 575.868,391 538.827,327.5 575.868,264 649.869,264 686.909,327.5 649.869,391",
                  fillColor: "#F89958"} );
      
      // path
      var r3 = scene.create( { t: "path", parent: layer1, id: "Outline-_Right_2_", a:1.0,
                  d: "M647.43,268l34.561,59.5L647.43,387h-69.123l-34.562-59.5l34.562-59.5H647 M652.308,260h-4.878h-69.123" +
                  "h-4.878l-2.459,4.122l-34.562,59.162l-2.5,4.261l2.5,4.272l34.562,59.089l2.459,4.095h4.878h69.123h4.878l2.459-4.101" +
                  "l34.561-59.151l2.5-4.254l-2.5-4.269l-34.561-59.11L652.308,260L652.308,260z",
                  fillColor: "#5D6D65"} );

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      // MIDDLE FILL
      var m1 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__BOTTOM_1_", a:1.0,
                  d: "polygon points: 539.545,411 526.414,388 539.545,365 565.808,365 578.94,388 565.808,411",
                  fillColor: "#5D6D66"} );

      var m2 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__BOTTOM", a:1.0,
                  d: "polygon points: 493.81,320.449 521.388,304.668 565.194,379.869 537.615,395.65",
                  fillColor: "#5C6D66"} );

      var m3 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__LEFT", a:1.0,
                  d: "polygon points: 429.43,344 420.167,328 429.43,312 447.954,312 457.216,328 447.954,344",
                  fillColor: "#5C6D66"} );

      var m4 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__LEFT_1_", a: 1.0,
                  d: "rect x:439.5 y:311.5 width:88 height:32",
                  fillColor: "#5C6D65"} );

      var m5 = scene.create( { t: "path", parent: layer1, id: "_x3C_Group_x3E__RIGHT", a:1.0,
                  d: "polygon points: 546.34,277 537.078,261 546.34,245 564.864,245 574.126,261 564.864,277",
                  fillColor: "#5C6D65"} );

      var m6 = scene.create( { t: "path", parent: layer1, id: "_x3C_Path_x3E__RIGHT_1_", a:1.0,
                  d: "polygon points: 521.104,351.388 493.525,335.606 537.331,260.404 564.91,276.186",
                  fillColor: "#5C6D65"} );

      layer1.paint = false;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      var layer2 = scene.create({t:"object", id: "layer2", a: 1.0, w:1256, h:1256, parent: svg }); //

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      var sparkFill   = "#5D6D65";
      var sparkStroke = "#FF00FF";

      // "S"
      var S = scene.create( { t: "path", parent: layer2, id: "Glyph S", a:1.0, fillColor: sparkFill,
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
                        strokeColor: sparkStroke, strokeWidth: 0 } );
      // "P"
      var P = scene.create( { t: "path", parent: layer2, id: "Glyph P", a:1.0, fillColor: sparkFill,
                      d: "M734.979,500.53c0,6.496-1.024,11.909-3.072,16.239s-4.71,7.828-7.987,10.488"+
                        "c-3.277,2.662-6.919,4.533-10.924,5.616c-4.006,1.083-7.966,1.624-11.879,1.624c-3.549,0-6.28-0.203-8.191-0.609" +
                        "c-1.912-0.406-3.551-1.194-4.916-2.368v16.104l6.145,3.112v11.097h-34.408v-11.097l6.008-3.112v-65.5h-6.008v-11.908l28.264-5.143" +
                        "v7.443c2.002-1.983,4.438-3.562,7.305-4.736c2.867-1.172,6.029-1.76,9.49-1.76c3.912,0,7.69,0.677,11.332,2.03" +
                        "c3.64,1.353,6.872,3.43,9.694,6.226c2.82,2.797,5.052,6.359,6.69,10.69S734.979,494.487,734.979,500.53z M711.768,502.425" +
                        "c0-6.134-1.229-11.05-3.687-14.751c-2.458-3.697-5.734-5.549-9.831-5.549c-1.729,0-3.549,0.406-5.461,1.219" +
                        "s-3.506,2.029-4.779,3.654v28.283c1.001,2.256,2.343,3.789,4.028,4.602c1.683,0.812,3.161,1.264,4.438,1.354" +
                        "c3.004,0.09,5.483-0.519,7.441-1.828c1.956-1.307,3.526-2.953,4.71-4.939c1.182-1.982,2.002-4.06,2.458-6.225" +
                        "C711.539,506.078,711.768,504.14,711.768,502.425z",
                        strokeColor: sparkStroke, strokeWidth: 0 } );
      // "A"
      var A = scene.create( { t: "path", parent: layer2, id: "Glyph A", a:1.0, fillColor: sparkFill,
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
                        strokeColor: sparkStroke, strokeWidth: 0 } );
      // "R"
      var R = scene.create( { t: "path", parent: layer2, id: "Glyph A", a:1.0, fillColor: sparkFill,
                      d: "M857.727,489.84c-0.73-0.901-1.686-1.669-2.867-2.301c-1.185-0.631-2.458-1.083-3.823-1.354" +
                        "c-1.366-0.271-2.731-0.338-4.097-0.203s-2.594,0.521-3.686,1.15v31.803l6.144,3.111v11.098H814.99v-11.098l6.008-3.111V481.99" +
                        "h-6.008v-11.908l28.264-5.008v10.556c1.454-2.165,3.024-3.992,4.71-5.481c1.683-1.488,3.366-2.659,5.052-3.518" +
                        "c1.684-0.856,3.299-1.422,4.848-1.691c1.547-0.271,2.912-0.225,4.096,0.135v23.412L857.727,489.84z",
                        strokeColor: sparkStroke, strokeWidth: 0 } );
      // "K"
      var K = scene.create( { t: "path", parent: layer2, id: "Glyph A", a:1.0, fillColor: sparkFill,
                      d: "M907.699,533.145v-11.098l4.096-3.111l-7.1-15.157l-7.919,7.037v8.12l6.144,3.111v11.098h-34.407v-11.098"+
                        "l6.008-3.111v-62.658h-6.008v-11.908l28.264-5.143v56.432l16.521-14.48l-5.598-2.977V467.24h32.633v10.961l-9.831,3.789"+
                        "l-10.649,9.608l16.111,27.337l6.008,3.111v11.098H907.699z",
                        strokeColor: sparkStroke, strokeWidth: 0 } );

      return new Promise(function(resolve, reject)
      {
        var assets = [t1.ready,t2.ready,t3.ready, 
                      b1.ready,b2.ready,b3.ready, 
                      r1.ready,r2.ready,r3.ready, 
                      m1.ready,m2.ready,m3.ready,m4.ready,m5.ready,m6.ready, 
                      S.ready,P.ready,A.ready,R.ready,K.ready ];

        Promise.all(assets)
        .then( function()
        {
          resolve(); // success
        },
        function()
        {
          console.log("ERROR:  test_drawSVG_paths.js >>  drawSVG_SparkPage()  creations failed !");
          reject(); // fail
        })
        .catch(function(error)
        {
          console.log("ERROR:  test_drawSVG_paths.js >>  drawSVG_SparkPage()  creations exception !");
          reject(); // fail
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject) {
        reject("ERROR:  test_drawSVG_paths.js >>  drawSVG_SparkPage()  >>> SVG not supported");
      });
    }
  } // drawSVG_SparkPage()

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Letter "M" ...
  function drawSVG_LetterM()
  {
    try
    {
      var letterM = scene.create({t:"object", parent: bg, x:700, y:100, a: 1.0 });

      var  p1 = scene.create( { t: "path",
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

      var assets = [p1];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_LetterM() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_LetterM() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_LetterM()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_LetterM()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Multi-color Cross ... (relative)
  function drawSVG_Cross()
  {
    try
    {
      var sw = 6;
      var cross = scene.create({t:"object", parent: bg, x:200, y:300, a: 1.0 });

      var p1 = scene.create( { t: "path", d:"M100 100 v-50", strokeColor: 0xFF00FFff, strokeWidth: sw, parent: cross} );
      var p2 = scene.create( { t: "path", d:"M100 100 v50",  strokeColor: 0x00FF00ff, strokeWidth: sw, parent: cross} );
      var p3 = scene.create( { t: "path", d:"M100 100 h50",  strokeColor: 0x0000FFff, strokeWidth: sw, parent: cross} );
      var p4 = scene.create( { t: "path", d:"M100 100 h-50", strokeColor: 0x00FFFFff, strokeWidth: sw, parent: cross} );

      var assets = [p1.ready, p2.ready, p3.ready, p4.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_Cross() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_Cross() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_Cross()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_Cross()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
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
    try
    {
      var p1 = scene.create( { t: "path", d:"circle cx:150 cy:250 r:41", strokeColor: 0x000000ff, strokeWidth: 4, fillColor: 0x00FFFFff, parent: bg} );
      // Part of the converted arcs
      // scene.create( { t: "path", d:"M191 140 C 191 163 173 181  150  181", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );

      var assets = [p1.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_Circle() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_Circle() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_Circle()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_Circle()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
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
    try
    {
      var p1 = scene.create( { t: "path", d:"ellipse cx:150 cy:340 rx:50 ry: 25", strokeColor: 0x000000ff, strokeWidth: 4, fillColor: 0xFFFF00ff, parent: bg} );

      var assets = [p1.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_Ellipse() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_Ellipse() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_Ellipse()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_Ellipse()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
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
    try
    {
      var x0 = 400, y0 = 250, w = 200, h = 100, rx = 30, ry = 30;
      var p1 = scene.create( { t: "path", d:"rect x:"+x0+" y:"+y0+" width:"+w+" height:"+h+" rx:"+rx+" ry:"+ry+"", strokeColor: 0x000000ff, strokeWidth: 8, fillColor: 0x8888FFff, parent: bg} );
      var p2 = scene.create( { t: "path", d:"rect x:"+x0+" y:"+(y0 + 150)+" width:"+w+" height:"+h+"", strokeColor: 0xFF0000ff, strokeWidth: 8, fillColor: 0x8888FFff, parent: bg} );

      var assets = [p1.ready, p2.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_RRect() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_RRect() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_RRect()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_RRect()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    ARC: absolute
  //
  //    Example from: ARC section >>  https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
  //
  function drawSVG_ARC1()
  {
    try
    {
      var arc1 = scene.create({t:"object", parent: bg, x:300, y:100, a: 1.0 });
                
      var p1 = scene.create( { t: "path", d:"M10 315 L 315 10", strokeColor: 0xFF0000ff, strokeWidth: 3,  parent: arc1} );
      var p2 = scene.create( { t: "path", d:"M10 315 "
                                          +"L 110 215 "
                                          +"A 30 50 0 0 1 162.55 162.45 "
                                          +"L 172.55 152.45 "
                                          +"A 30 50 -45 0 1 215.1 109.9 "
                                          +"L 315 10Z",strokeWidth: 2, strokeColor: 0x000000ff, fillColor: 0x00FF0080,  parent: arc1} );

      var assets = [p1.ready, p2.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_ARC1() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_ARC1() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_ARC1()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_ARC1()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    ARC: relative
  //
  //    Example from: ARC section >>  https://www.w3.org/TR/SVG/paths.html#PathDataEllipticalArcCommands
  //
  function drawSVG_ARC2()
  {
    try
    {
      var p1 = scene.create( { t: "path", d:"M600 350 l 50 -25 "
                                          +"a25 25  -30 0 1 50 -25 l 50 -25 "
                                          +"a25 50  -30 0 1 50 -25 l 50 -25 "
                                          +"a25 75  -30 0 1 50 -25 l 50 -25 "
                                          +"a25 100 -30 0 1 50 -25 l 50 -25Z",
                            fillColor: 0x00000000, strokeColor: 0xFF0000ff, strokeWidth: "5",  parent: bg } );

      var assets = [p1.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_ARC1() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_ARC2() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_ARC2()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_ARC2()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    ARC SWEEP - 4 possible values
  //
  //    Example from: ARC section >>  https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
  //
  function drawSVG_SWEEP()
  {
    try
    {
      var obj = scene.create({t:"object", parent: bg, x:600, y:300, a: 1.0 });

      var p1 = scene.create( { t: "path", d: "M80 80 "
                                            +"A 45 45  0  0  0  125 125 "
                                            +"L 125 80 Z",
                            fillColor: 0x00FF00ff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

      var p2 = scene.create( { t: "path", d: "M230 80 "
                                            +"A 45 45  0  1  0  275 125 "
                                            +"L 275 80 Z",
                            fillColor: 0xFF0000ff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

      var p3 = scene.create( { t: "path", d: "M80 230 "
                                            +"A 45 45  0  0  1  125 275 "
                                            +"L 125 230 Z",
                            fillColor: 0xFF00FFff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

      var p4 = scene.create( { t: "path", d: "M230 230 "
                                            +"A 45 45  0  1  1  275 275 "
                                            +"L 275 230 Z",
                            fillColor: 0x0000FFff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: obj } );

      var assets = [p1.ready, p2.ready, p3.ready, p4.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_SWEEP() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_SWEEP() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_SWEEP()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_SWEEP()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    Curves
  //
  // These curves should overlap, Black Below, Red Above
  //
  function drawSVG_CURVES()
  {
    try
    {
      var p = scene.create({t:"object", parent: bg, x:-50, y:-150, a: 1.0 });

      var p1 = scene.create( { id: "CurveBG", t: "path", d:"M100,250 Q250,100 400,250",   strokeColor: 0x000000ff, strokeWidth: 13, parent: p} );
      var p2 = scene.create( { id: "CurveFG", t: "path", d:"M100,250 q150,-150 300,0",    strokeColor: 0xFF0000ff, strokeWidth: 3,  parent: p} );

      var p3 = scene.create( { id: "CurveBG", t: "path", d:"M100 180 q50 10, 95 80",      strokeColor: 0x000000ff, strokeWidth: 13, parent: p} );
      var p4 = scene.create( { id: "CurveFG", t: "path", d:"M100 180 Q150 190, 195 260",  strokeColor: 0xFF0000ff, strokeWidth: 3,  parent: p} );

//     scene.create( {  id: "Marker", t: "path", d:"rect x:50 y:50 width:20 height:20", fillColor: 0x8888FFff, parent: p} ); // JUNK
//     scene.create( {  id: "Marker", t: "path", d:"rect x:91 y:171 width:18 height:18", fillColor: 0x8888FFff, parent: p} );

      var assets = [p1.ready, p2.ready, p3.ready, p4.ready];

      return new Promise(function(resolve, reject)
      {
        Promise.all(assets)
        .then( function()
        {
          results.push(assert(true, "drawSVG_CURVES() - create() success"));
          resolve(results); // success
        },
        function()
        {
          results.push(assert(false, "drawSVG_CURVES() - create() failed"));
          /*reject*/resolve(results); // fail
        })
        .catch(function(error)
        {
          results.push(assert(false, "drawSVG_CURVES()  >>  create() exception ! ["+error+"]"));
          /*reject*/resolve(results); // fatal
        });
      });
    }
    catch(error) {
      return new Promise (function(resolve, reject)
      {
        results.push(assert(false, "drawSVG_CURVES()  >>  SVG is NOT supported"));
        /*reject*/resolve(results); // fatal
      });
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function updateSize(w,h)
  {
    bg.w = w;
    bg.h = h;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  scene.on("onResize", function(e) { updateSize(e.w, e.h); });

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Promise.all([ bg.ready ])
      .catch( (err) =>
      {
          console.log("SVG >> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
          updateSize(scene.w, scene.h);
      });

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}).catch( function importFailed(err){
  console.log("Import for test_draw_SVG_paths.js failed: " + err)
});
