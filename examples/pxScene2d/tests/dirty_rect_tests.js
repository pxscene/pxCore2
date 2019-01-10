px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var shots = imports.shots;
var manual = imports.manual;

var doScreenshot = shots.getScreenshotEnabledValue();
var testPlatform=scene.info.build.os;

var manualTest = manual.getManualTestValue();
var timeoutForScreenshot = 40;

var basePackageUri = px.getPackageBaseFilePath();

//var textA = "ÉéÈèÇçËëÒòÔôÖöÙùÀàÂâAaBbCcDdEeFfGgHhIiKkLlMmNnOoPpQqRrSsTtVvXxYyZz123456789";
//var longText = textA + "\n" + textA + "\n" + textA;
// "Hello!  How are you?";//
// Use fontUrl to load from web
var fontUrlStart = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/";
root.w=800;

scene.showDirtyRect=true;
//scene.showMeasurements=true;
var bg = scene.create({t:"object", parent:root, x:100, y:100, w:1000, h:1000, clip:false});
//var rect = scene.create({t:"rect", parent:root, x:100, y:100, w:700, h:700, fillColor:0x00000000, lineColor:0xFF00FF77, lineWidth:1, clip:false});
var container = scene.create({t:"object", parent:root, x:100, y:100, w:800, h:600, clip:false});

// Widgets for displaying metrics values 
var height = scene.create({t:"text", parent:root, x:50, y:0, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"Height="});
var width = scene.create({t:"text", parent:root, x:50, y:20, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"Width="});
var boundsX1 = scene.create({t:"text", parent:root, x:200, y:0, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsX1="});
var boundsY1 = scene.create({t:"text", parent:root, x:200, y:20, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsY1="});
var boundsX2 = scene.create({t:"text", parent:root, x:200, y:40, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsX2="});
var boundsY2 = scene.create({t:"text", parent:root, x:200, y:60, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsY2="});
var drboundsX1 = scene.create({t:"text", parent:root, x:400, y:0, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"DrBoundsX1="});
var drboundsY1 = scene.create({t:"text", parent:root, x:400, y:20, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"DrBoundsY1="});
var drboundsX2 = scene.create({t:"text", parent:root, x:400, y:40, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"DrBoundsX2="});
var drboundsY2 = scene.create({t:"text", parent:root, x:400, y:60, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"DrBoundsY2="});
var testText = scene.create({t:"text", parent:root, x:600, y:60, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"Test="});


var testrect = scene.create({t:"rect",parent:container,fillColor:0xffffffff,x:0,y:100,w:300,h:300,cx:150,cy:150});
                 
var metrics = null;
var measurements = null;
var showMeasurements = function() {
    var bounds = measurements.bounds;
    var drbounds = measurements.drbounds;
    var green = 0x00FF0077;
    var blue = 0x0000FF77;
    var red = 0xFF000077;
    var yellow = 0xFFFF0077;
    var orange = 0xFF8C0077;
    var pink = 0xFF00FF77;
    scene.create({t:"rect", fillColor:0x00000000, parent:bg, lineColor:yellow, lineWidth:1, x:bounds.x1, y:bounds.y1, w:bounds.x2 - bounds.x1, h:bounds.y2 - bounds.y1});
    scene.create({t:"rect", fillColor:0x00000000, parent:bg, lineColor:red, lineWidth:1, x:drbounds.x1, y:drbounds.y1, w:drbounds.x2 - drbounds.x1, h:drbounds.y2 - drbounds.y1});
    }


var rectready = function(obj) {
	console.log("inside rect.ready");
  //console.log("text2.h="+text2.h+" and text2.w="+text2.w);

  var dirtyRect=scene.getDirtyRect;
  measurements=expectedTextDesc;
  measurements.bounds=dirtyRect;
  measurements.drbounds=dirtyRect;

  measurements.bounds.x1=testrect.x;
  measurements.bounds.y1=testrect.y;
  measurements.bounds.x2=(testrect.w+testrect.x);
  measurements.bounds.y2=(testrect.h+testrect.y);
  
  console.log("measurements boundsX1="+measurements.bounds.x1);
  console.log("measurements boundsY1="+measurements.bounds.y1);
  console.log("measurements boundsX2="+measurements.bounds.x2);
  console.log("measurements boundsY2="+measurements.bounds.y2);
  console.log("measurements boundsW="+measurements.drbounds.x1);
  console.log("measurements boundsH="+measurements.drbounds.y1);
  height.text="Height="+testrect.h;
  width.text="Width="+testrect.w;
  boundsX1.text="BoundsX1="+measurements.bounds.x1;
  boundsY1.text="BoundsY1="+measurements.bounds.y1;
  boundsX2.text="BoundsX2="+measurements.bounds.x2;
  boundsY2.text="BoundsY2="+measurements.bounds.y2;
  drboundsX1.text="DrBoundsX1="+measurements.drbounds.x1;
  drboundsY1.text="DrBoundsY1="+measurements.drbounds.y1;
  drboundsX2.text="DrBoundsX2="+measurements.drbounds.x2;
  drboundsY2.text="DrBoundsY2="+measurements.drbounds.y2;
  

  showMeasurements();
}

/** HELPER FUNCTIONS FOR CHANGING TEXT2 PROPERTIES **/
var cycleValues = function(v) {
    console.log("v is "+v);
    if( v >= 2) {
      v = 0;
    } else {
      v++;
    }
    console.log("v is now"+v);
    return v;
}

/**********************************************************************/
/**                                                                   */
/**            pxscene tests go in this section                       */
/**                                                                   */
/**********************************************************************/
var expectedTextDesc = [
  ["bounds", "x1"], 
  ["bounds", "y1"], 
  ["bounds", "x2"], 
  ["bounds", "y2"],
  ["drbounds", "x1"], 
  ["drbounds", "y1"], 
  ["drbounds", "x2"], 
  ["drbounds", "y2"],
  
];
var expectedValuesMeasure = {
  // bounds.x1, bounds.y1, bounds.x2, bounds.y2, dirbounds.x1, dirbounds.y1, dirbounds.x2, dirbounds.y2
  "rectangleTestx0y0":[0, 0, 300, 300, 0, 0, 300, 300], // rectangleTestx0y0
  "rectangleTestx100y0":[100, 0, 400, 300, 100, 0, 400, 300], // rectangleTestx100y0
  "rectangleTestx100y100":[100, 100, 400, 400, 100, 100, 400, 400], // rectangleTestx100y100
  "rectangleTestx200y200":[200, 200, 500, 500, 200, 200, 500, 500], // rectangleTestx200y200
  "rectangleTestx300y500":[300, 500, 600, 800, 300, 500, 600, 800], // rectangleTestx300y500
};

var rectMeasurementResults = function(values) {
  var results = [];
  var numResults = values.length;
  for( var i = 0; i < numResults; i++) {
    results[i] = assert(measurements[expectedTextDesc[i][0]][expectedTextDesc[i][1]] === values[i], "measurements "+expectedTextDesc[i][0]+"."+expectedTextDesc[i][1]+" should be "+values[i]+" but is "+measurements[expectedTextDesc[i][0]][expectedTextDesc[i][1]]);
  }
  return results;
}

var beforeStart = function() {
  return new Promise(function(resolve, reject) {

    // Setup all properties as assumed for start of tests
    // set to short text, wordWrap=false, pixelSize, hAlign=left 
  //  testrect.animateTo({r:0,x:0, y:0},2,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
    testrect.x=0;
    testrect.y=0;
    
    resolve("dirty_rect_tests.js beforeStart");
  });
}

var doScreenshotComparison = function(name, resolve, reject) 
{
    var results = rectMeasurementResults(expectedValuesMeasure[name]);

    shots.takeScreenshot(false).then(function(link){
      shots.validateScreenshot(basePackageUri+"/images/screenshot_results/"+testPlatform+"/dirty_rect_tests_"+name+".png", false).then(function(match){
        console.log("test result is match: "+match);
        results.push(assert(match == true, "screenshot comparison for "+name+" failed"));
        resolve(results);
      });
    }).catch(function(err) {
        results.push(assert(false, "screenshot comparison for "+name+" failed due to error: "+err));
        resolve(results);
    });
 
}

function fancy(o) {
  var startX = 450;
  var startY = 100;
  // animate x and restart the overall animation at end
  o.x = startX;
  o.animateTo({x:50}, 1.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1)
    .then(function(o){
      o.animateTo({x:startX}, 3.0, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1)
        .then(function(o){
          fancy(o);
      })
  });

  // animate y
  o.y = startY;
  o.animateTo({y:350}, 1.0, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_LOOP, 1)
    .then(function(o) {
      o.animateTo({y:startY}, 1.0, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1);
  });

  // animate r
  o.r = 0;
  o.animateTo({r:-360}, 2.5, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1);

  // animate sx, sy
  o.animateTo({sx:0.2,sy:0.2}, 1, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_LOOP, 1)
    .then(function(o){
      o.animateTo({sx:2.0,sy:2.0}, 1.0, scene.animation.TWEEN_EXP1, scene.animation.OPTION_LOOP, 1)
        .then(function(o) {
          o.animateTo({sx:1.0,sy:1.0}, 1.0, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1);
      })
  });
}
var tests = {

  rectangleTestx0y0: function() {
  
  console.log("dirty_rect_tests.js rectangleTestx0y0");
  //testrect.animateTo({x:0, y:0},10,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
  testrect.x=0;
  testrect.y=0;
  console.log("rectangleTestx0y0 is "+expectedValuesMeasure.rectangleTestx0y0);
   return new Promise(function(resolve, reject) {
    testText.text="Test=rectangleTestx0y0";
    rectready(testrect);
    if(doScreenshot) 
    {
        setTimeout( function() {
          doScreenshotComparison("rectangleTestx0y0", resolve)
        }, timeoutForScreenshot);
    } 
    else 
      resolve( rectMeasurementResults(expectedValuesMeasure.rectangleTestx0y0));
   });
 },
 rectangleTestx100y0: function() {
  
  console.log("dirty_rect_tests.js rectangleTestx100y0");
  //testrect.animateTo({x:100, y:0},10,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
  testrect.x=100;
  testrect.y=0;
  console.log("rectangleTestx100y0 is "+expectedValuesMeasure.rectangleTestx100y0);
   return new Promise(function(resolve, reject) {
    rectready(testrect);
    testText.text="Test=rectangleTestx100y0";
    if(doScreenshot) 
    {
        setTimeout( function() {
          doScreenshotComparison("rectangleTestx100y0", resolve)
        }, timeoutForScreenshot);
    } 
    else 
      resolve( rectMeasurementResults(expectedValuesMeasure.rectangleTestx100y0));
   });
 },
 rectangleTestx100y100: function() {
  
  console.log("dirty_rect_tests.js rectangleTestx100y100");
  //testrect.animateTo({x:100, y:100},12,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
  testrect.x=100;
  testrect.y=100;
  console.log("rectangleTestx100y100 is "+expectedValuesMeasure.rectangleTestx100y100);

   return new Promise(function(resolve, reject) {
    rectready(testrect);
    testText.text="Test=rectangleTestx100y100";
    if(doScreenshot) 
    {
        setTimeout( function() {
          doScreenshotComparison("rectangleTestx100y100", resolve)
        }, timeoutForScreenshot);
    } 
    else 
      resolve( rectMeasurementResults(expectedValuesMeasure.rectangleTestx100y100));
   });
 },
 rectangleTestx200y200: function() {
  
  console.log("dirty_rect_tests.js rectangleTestx200y200");
  //testrect.animateTo({x:200, y:200},12,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
  testrect.x=200;
  testrect.y=200;
  console.log("rectangleTestx200y200 is "+expectedValuesMeasure.rectangleTestx200y200);
   return new Promise(function(resolve, reject) {
    rectready(testrect);
    testText.text="Test=rectangleTestx200y200";
    if(doScreenshot) 
    {
        setTimeout( function() {
          doScreenshotComparison("rectangleTestx200y200", resolve)
        }, timeoutForScreenshot);
    } 
    else 
      resolve( rectMeasurementResults(expectedValuesMeasure.rectangleTestx200y200));
   });
 },
 rectangleTestx300y500: function() {
  
  console.log("dirty_rect_tests.js rectangleTestx300y500");
  //testrect.animateTo({x:200, y:200},12,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
  testrect.x=300;
  testrect.y=500;
  console.log("rectangleTestx300y500 is "+expectedValuesMeasure.rectangleTestx300y500);
   return new Promise(function(resolve, reject) {
    rectready(testrect);
    testText.text="Test=rectangleTestx300y500";
    if(doScreenshot) 
    {
        setTimeout( function() {
          doScreenshotComparison("rectangleTestx300y500", resolve)
        }, timeoutForScreenshot);
    } 
    else 
      resolve( rectMeasurementResults(expectedValuesMeasure.rectangleTestx300y500));
   });
 },
 }

module.exports.beforeStart = beforeStart;
module.exports.tests = tests;


if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}


}).catch( function importFailed(err){
  console.error("Import failed for text2tests.js: " + err)
});