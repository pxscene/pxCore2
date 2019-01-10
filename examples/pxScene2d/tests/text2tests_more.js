px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var shots = imports.shots;
var manual = imports.manual;

var doScreenshot = false;
var manualTest = manual.getManualTestValue();
var timeoutForScreenshot = 40;

var basePackageUri = px.getPackageBaseFilePath();

//var textA = "ÉéÈèÇçËëÒòÔôÖöÙùÀàÂâAaBbCcDdEeFfGgHhIiKkLlMmNnOoPpQqRrSsTtVvXxYyZz123456789";
//var longText = textA + "\n" + textA + "\n" + textA;
// "Hello!  How are you?";//
// Use fontUrl to load from web
var fontUrlStart = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/";
var XFinityMed = "XFINITYStandardTT-Medium.ttf";//"XFINITYSansTT-New-Med.ttf";
var DejaVu = "DejaVuSans.ttf";
var DejaVuSerif = "DejaVuSerif.ttf";
var XFinity = "XFINITYStandardTT-Light.ttf";//"XFINITYSansTT-New-Lgt.ttf";
var XFinityBold = "XFINITYStandardTT-Bold.ttf";//"XFINITYSansTT-New-Bold.ttf";
// Different text strings to test
var longText = "Here is a collection of a bunch of randomness, like words, phrases, and sentences that isn't supposed to make any kind of sense whatsoever. I want to test capital AV next to each other here. In generating this, I'm listening to someone talking, trying to make sense of what they are saying, and at the same time dictating to myself what I am going to type along with actually typing it out, recognizing when I make mistakes, and correcting myself when I do.";
var longText2 = "I don't think I'm doing a very good job listening to whoever it is that is doing the talking right now.  It probably would have been a lot easier to just copy and paste something from the net, but I'm a typist, not a person that hunts and pecks to find the appropriate key on the keyboard.  Though I do think I'm probably off of my 30 word per minute speed from way back when.  How much more text is appropriate?  Why do I use words like appropriate when I could just say will do instead?  These and other questions generally go on unanswered.  But looking at the output of this text, I realize that its simply not enough and that I need to add more text; which is making me wonder why I simply didn't copy and paste in the first place.  Ah, yes, the strange musings of a software engineer.";
var longText3 = longText + " " +longText2;
var shortText = "Hello!  How are you?";
var mediumText = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog."
var newlineText = "Paragraph\nParagraph longer\nParagraph more";
var continuousText = "ParagraphParagraphParagraphParagraphParagraphParagraphlongerParagraphmore";
var continuousLongText = "ParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmoreParagraphParagraphlongerParagraphlongestParagraphmore";
var multilinesText = "Pxscene is an application engine that has been added to the RDK.\n\
It is a scene graph API exposed to a Javascript engine in the ground.\n\
In other words, it allows for set top box applications to be authored in javascript.\n\
The authored javascript has access to the pxscene API for visual elements that are used for composition.";
var wordBoundaryCharsText = "Paragraph Paragraph Paragraph:Paragraph Paragraph&Paragraph Paragraph,Paragraph Paragraph;Paragraph Paragraph.Paragraph Paragraph?Paragraph Paragraph!Paragraph"; // \t/:&,;.?!
root.w=800;
//
// Use the font vars below to preload fonts so that they stay loaded. 

var fontXfinityMed = scene.create({t:"fontResource",url:fontUrlStart+XFinityMed});
var fontDejaVu = scene.create({t:"fontResource",url:fontUrlStart+DejaVu});
var fontDejaVuSerif = scene.create({t:"fontResource",url:fontUrlStart+DejaVuSerif});
var fontXFinity = scene.create({t:"fontResource",url:fontUrlStart+XFinity});
var fontXFinityBold = scene.create({t:"fontResource",url:fontUrlStart+XFinityBold});





var bg = scene.create({t:"object", parent:root, x:100, y:100, w:1000, h:1000, clip:false});
var rect = scene.create({t:"rect", parent:root, x:100, y:100, w:400, h:400, fillColor:0x00000000, lineColor:0xFF0000FF, lineWidth:1, clip:false});
var container = scene.create({t:"object", parent:root, x:100, y:100, w:800, h:600, clip:false});

// Widgets for displaying metrics values 
var height = scene.create({t:"text", parent:root, x:50, y:0, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"Height="});
var ascent = scene.create({t:"text", parent:root, x:50, y:20, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"Ascent="});
var descent = scene.create({t:"text", parent:root, x:50, y:40, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"Descent="});
var naturalLeading = scene.create({t:"text", parent:root, x:50, y:60, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"NatLead="});
var baseline  = scene.create({t:"text", parent:root, x:50, y:80, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"Baseline="});
var boundsX1 = scene.create({t:"text", parent:root, x:200, y:0, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsX1="});
var boundsY1 = scene.create({t:"text", parent:root, x:200, y:20, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsY1="});
var boundsX2 = scene.create({t:"text", parent:root, x:200, y:40, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsX2="});
var boundsY2 = scene.create({t:"text", parent:root, x:200, y:60, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"BoundsY2="});
var charFirstX = scene.create({t:"text", parent:root, x:400, y:0, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"charFirstX="});
var charFirstY = scene.create({t:"text", parent:root, x:400, y:20, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"charFirstY="});
var charLastX = scene.create({t:"text", parent:root, x:400, y:40, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"charLastX="});
var charLastY = scene.create({t:"text", parent:root, x:400, y:60, textColor:0xFFDDFFFF, pixelSize:15,clip:false,text:"charLastY="});
 
// widgets for tracking current property settings
var truncationStatus = scene.create({t:"text", parent:root, x:20, y:container.y+420, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"truncation=truncate"});
var wrapStatus = scene.create({t:"text", parent:root, x:20, y:container.y+440, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"wordWrap=true"});
var hAlignStatus = scene.create({t:"text", parent:root, x:20, y:container.y+460, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"hAlign=left"});
var vAlignStatus = scene.create({t:"text", parent:root, x:20, y:container.y+480, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"vAlign=top"});
var ellipsisStatus = scene.create({t:"text", parent:root, x:20, y:container.y+500, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"ellipsis=true"});
var pixelSizeStatus = scene.create({t:"text", parent:root, x:20, y:container.y+520, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"pixelSize=20"});
var pixelSizeHint = scene.create({t:"text", parent:root, x:140, y:container.y+520, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"(use p and P)"});
var textStatus = scene.create({t:"text", parent:root, x:350, y:container.y+420, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"text=longest"});
var textHint = scene.create({t:"text", parent:root, x:665, y:container.y+420, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"(use small s)"});
var clipStatus = scene.create({t:"text", parent:root, x:350, y:container.y+440, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"clip=true"});
var xStartPosStatus = scene.create({t:"text", parent:root, x:350, y:container.y+460, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"xStartPos=0"});
var xStopPosStatus = scene.create({t:"text", parent:root, x:350, y:container.y+480, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"xStopPos=0"});
var xStopPosHint = scene.create({t:"text", parent:root, x:465, y:container.y+480, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"(use small L)"});
var leadingStatus = scene.create({t:"text", parent:root, x:350, y:container.y+500, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"leading=0"});
var leadingHint = scene.create({t:"text", parent:root, x:465, y:container.y+500, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"(use + -)"});
var fontStatus = scene.create({t:"text", parent:root, x:350, y:container.y+520, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"font="+XFinityMed+" (http)"});
var leading = 0;


var text2 = scene.create({t:"textBox", clip:true, parent:container, x:0, y:0, rx:0, ry:0, rz:0});
   text2.h=400;
   text2.w=400;
   text2.textColor=0xFFDDFFFF;
   text2.pixelSize=20;
   text2.leading=0;
   text2.font=fontXfinityMed;
   text2.alignHorizontal=0;
   text2.alignVertical=0;
   text2.xStartPos=0;
   text2.xStopPos=0;
	 text2.wordWrap=true;
   text2.ellipsis=true;
   text2.truncation=1;
  
   text2.text=longText3;

                 
var metrics = null;
var measurements = null;
var showMeasurements = function() {
    var bounds = measurements.bounds;
    var charFirst = measurements.charFirst;
    var charLast = measurements.charLast;
    var w = bounds.x2 - bounds.x1;
    var spacing = metrics.height + text2.leading;
    var x = bounds.x1;//0;
    var y = bounds.y1;//0;
    var green = 0x00FF0077;
    var blue = 0x0000FF77;
    var red = 0xFF000077;
    var yellow = 0xFFFF0077;
    var orange = 0xFF8C0077;
    var pink = 0xFF00FF77;
    do {
        scene.create({t:"rect", parent:bg, fillColor:green, x:x, y:y + metrics.baseline - metrics.ascent, w:w, h:metrics.ascent});
        scene.create({t:"rect", parent:bg, fillColor:blue, x:x, y:y + metrics.baseline, w:w, h:metrics.descent});
        scene.create({t:"rect", fillColor:0x00000000, parent:bg, lineColor:red, lineWidth:1, x:x, y:y, w:w, h:metrics.height});
        y += spacing;
    } while (y < bounds.y2);
    scene.create({t:"rect", fillColor:0x00000000, parent:bg, lineColor:yellow, lineWidth:1, x:bounds.x1, y:bounds.y1, w:w, h:bounds.y2 - bounds.y1});
    scene.create({t:"rect", fillColor:0x00000000, parent:bg, lineColor:pink, lineWidth:1, x:charFirst.x, y:charFirst.y, w:charLast.x - charFirst.x, h:(charLast.y - charFirst.y)==0?1:(charLast.y - charFirst.y)});
}


var textready = function(text) {
	console.log("inside text2.ready");
  console.log("text2.h="+text2.h+" and text2.w="+text2.w);

	metrics = text2.font.getFontMetrics(text2.pixelSize);
	console.log("metrics h="+metrics.height);
	console.log("metrics a="+metrics.ascent);
	console.log("metrics d="+metrics.descent);
  console.log("metrics naturalLeading="+metrics.naturalLeading);
  console.log("metrics baseline="+metrics.baseline);

  measurements = text2.measureText();
  console.log("measurements boundsX1="+measurements.bounds.x1);
  console.log("measurements boundsY1="+measurements.bounds.y1);
  console.log("measurements boundsX2="+measurements.bounds.x2);
  console.log("measurements boundsY2="+measurements.bounds.y2);
  console.log("measurements charFirstX="+measurements.charFirst.x);
  console.log("measurements charFirstY="+measurements.charFirst.y);
  console.log("measurements charLastX="+measurements.charLast.x);
  console.log("measurements charLastY="+measurements.charLast.y);

  height.text="Height="+metrics.height;
  ascent.text="Ascent="+metrics.ascent;
  descent.text="Descent="+metrics.descent;
  naturalLeading.text="NatLead="+metrics.naturalLeading;
  baseline.text="Baseline="+metrics.baseline;
  boundsX1.text="BoundsX1="+measurements.bounds.x1;
  boundsY1.text="BoundsY1="+measurements.bounds.y1;
  boundsX2.text="BoundsX2="+measurements.bounds.x2;
  boundsY2.text="BoundsY2="+measurements.bounds.y2;
  charFirstX.text="charFirstX="+measurements.charFirst.x;
  charFirstY.text="charFirstY="+measurements.charFirst.y;
  charLastX.text="charLastX="+measurements.charLast.x;
  charLastY.text="charLastY="+measurements.charLast.y;

  
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
var setText = function(textValue,hintText) {
     text2.text = textValue; 
     textStatus.text = hintText;
}
var toggleWordWrap = function() {
    text2.wordWrap = !text2.wordWrap;
    if( text2.wordWrap) {
      wrapStatus.text ="wordWrap=true";
    } else {
      wrapStatus.text ="wordWrap=false";
    }
}
var toggleEllipsis = function() {
  text2.ellipsis = !text2.ellipsis;
  if( text2.ellipsis) {
    ellipsisStatus.text ="ellipsis=true";
  } else {
    ellipsisStatus.text ="ellipsis=false";
  }
}

var toggleClip = function() {
    text2.clip  = !text2.clip;
    if( text2.clip) {
      clipStatus.text ="clip=true";
    } else {
      clipStatus.text ="clip=false";
    }  
}
var setFont = function(fontName, hintText) {
    text2.font = fontName;
    fontStatus.text = hintText;
}

var setAlignH = function(ha) {

  text2.alignHorizontal = ha;

  if(ha == 0) {
    hAlignStatus.text="hAlign=left";
  } else if(ha == 1) {
    hAlignStatus.text="hAlign=center";
  } else {
    hAlignStatus.text="hAlign=right";
  }   
}
var setAlignV = function(va){
  text2.alignVertical = va;
  if(va == 0) {
    vAlignStatus.text="vAlign=top";
  } else if(va == 1) {
    vAlignStatus.text="vAlign=center";
  } else {
    vAlignStatus.text="vAlign=bottom";
  }
}
var setTruncation = function(t) {
  text2.truncation = t;
  if(t == 0) {
    truncationStatus.text="truncation=none";
  } else if(t == 1) {
    truncationStatus.text="truncation=truncate";
  } else {
    truncationStatus.text="truncation=truncate at word boundary";
  }
}

var setPixelSize = function(p) {
  text2.pixelSize = p;
  pixelSizeStatus.text="pixelSize="+p;
}
var setLeading = function(l) {
  text2.leading = l;
  leadingStatus.text="leading="+l;
}

var setXStartPos = function(s) {
  text2.xStartPos = s; 
  xStartPosStatus.text="xStartPos="+s;
}

var setXStopPos = function(s) {
  text2.xStopPos = s; 
  xStopPosStatus.text="xStopPos="+s;
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
  ["charFirst", "x"], 
  ["charFirst", "y"], 
  ["charLast", "x"], 
  ["charLast", "y"]
  
];
var expectedValuesMeasure = {
  // bounds.x1, bounds.y1, bounds.x2, bounds.y2, charFirst.x, charFirst.y, charLast.x, charLast.y
  "longestTextNoWrapNoTruncateNoClipNoEllipsisNoHeightNoWidthH0V0":[0,0,2029,24,0,20,2029,20], //longestTextNoWrapNoTruncateNoClipNoEllipsisNoHeightNoWidthH0V0
};

var textMeasurementResults = function(values) {
  var results = [];
  var numResults = values.length;
  for( var i = 0; i < numResults; i++) {

    results[i] = assert(measurements[expectedTextDesc[i][0]][expectedTextDesc[i][1]] === values[i], "measurements "+expectedTextDesc[i][0]+"."+expectedTextDesc[i][1]+" should be "+values[i]+" but is "+measurements[expectedTextDesc[i][0]][expectedTextDesc[i][1]]);
  }
  return results;
}
var cleanup = function () {
  text2.parent=container;
  bg.x=container.x;
  bg.y=container.y;
  text2.h=400;
  text2.w=400;
}
var setParentToRoot = function () {

  text2.parent=root;
  bg.x=root.x;
  bg.y=root.y;
}

var beforeStart = function() {

  return new Promise(function(resolve, reject) {

    // Setup all properties as assumed for start of tests
    // set to short text, wordWrap=false, pixelSize, hAlign=left 
    setFont(fontXfinityMed,"font="+XFinityMed+" (http)");
    if( text2.wordWrap) {
      toggleWordWrap();
    }
    setPixelSize(20);
    setLeading(0);
    setAlignH(0);
    setAlignV(0);
    if( text2.clip) {
      toggleClip();
    }
    setTruncation(0);
    if( text2.ellipsis) {
      toggleEllipsis();
    }
    setXStartPos(0);
    setXStopPos(0);
    resolve("text2tests.js beforeStart");
  });
}

var doScreenshotComparison = function(name, resolve, reject) 
{
    var results =  textMeasurementResults(expectedValuesMeasure[name]);

    //shots.takeScreenshot(false).then(function(link){
      shots.validateScreenshot(basePackageUri+"/images/screenshot_results/text2tests_"+name+".png", false).then(function(match){
        console.log("test result is match: "+match);
        results.push(assert(match == true, "screenshot comparison for "+name+" failed"));
        resolve(results);
      //});
    }).catch(function(err) {
        results.push(assert(false, "screenshot comparison for "+name+" failed due to error: "+err));
        resolve(results);
    });
 
}

var tests = {
   longestTextNoWrapNoTruncateNoClipNoEllipsisNoHeightNoWidthH0V0: function() {
     console.log("text2tests.js longestTextNoWrapNoTruncateNoClipNoEllipsisNoHeightNoWidthH0V0");
     // set to longest text
     setText( longText3,"text=longest");
     setAlignH(0);
     setAlignV(0);
     setTruncation(0);
     text2.leading=0;
     setParentToRoot();
     text2.clip=false;
     text2.wordWrap=false;
     text2.h=0;
     text2.w=0;
 
     return new Promise(function(resolve, reject) {
       text2.ready.then(function(myText) {
         bg.removeAll();
         textready(myText);
         if( doScreenshot) 
         {
             setTimeout( function() {
               doScreenshotComparison("longestTextNoWrapNoTruncateNoClipNoEllipsisNoHeightNoWidthH0V0", resolve)
             }, timeoutForScreenshot);
         } 
         else {
          resolve(textMeasurementResults(expectedValuesMeasure.longestTextNoWrapNoTruncateNoClipNoEllipsisNoHeightNoWidthH0V0));
         }
         cleanup();
       }, function(o) {
         resolve(assert(false,'longestTextNoWrapNoTruncateNoClipNoEllipsisNoHeightNoWidthH0V0 Promise rejection received'));
       });
     });
   }
 }

module.exports.beforeStart = beforeStart;
module.exports.tests = tests;


if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}


}).catch( function importFailed(err){
  console.error("Import failed for text2tests.js: " + err)
});
