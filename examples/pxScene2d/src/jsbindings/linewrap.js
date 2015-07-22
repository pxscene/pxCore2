var root = scene.root;
var longText = "Here is a collection of a bunch of randomness, like words, phrases, and sentences that isn't supposed to make any kind of sense whatsoever. I want to test capital AV next to each other here. In generating this, I'm listening to someone talking, trying to make sense of what they are saying, and at the same time dictating to myself what I am going to type along with actually typing it out, recognizing when I make mistakes, and correcting myself when I do.  I don't think I'm doing a very good job listening to whoever it is that is doing the talking right now.  It probably would have been a lot easier to just copy and paste something from the net, but I'm not a pecker, meaning a person that hunts and pecks to find the appropriate key on the keyboard.  Though I do think I'm probably off of my 30 word per minute speed from way back when.  How much more text is appropriate?  Why do I use words like appropriate when I could just say will do instead?  These and other questions generally go on unanswered.  But looking at the output of this text, I realize that its simply not enough and that I need to add more text; which is making me wonder why I simply didn't copy and paste in the first place.  Ah, yes, the strange musings of a software engineer.";
var rect = scene.create({t:"rect", parent:root, w:400, h:400, lineColor:0x334433FF, lineWidth:1, clip:false});
var container = scene.create({t:"object", parent:root, w:400, h:400, clip:true});

var px = 0;
var py = 0;
var leading = 0;

// Use fontUrl to load from web
//var fontUrl = "http://localhost/XRE2/fonts/DancingScript-Bold.ttf";
var text2 = scene.create({t:"text2",   text:longText,  faceURL:"DancingScript-Bold.ttf", horizontalAlign:0, verticalAlign:0, xStartPos:25,xStopPos:0,
						wordWrap:true, ellipsis:true, truncation:2,leading:0, 
						clip:false, w:400, h:400, parent:container, textColor:0xFFDDFFFF, 
						pixelSize:20, x:px, y:py, rx:0, ry:1, rz:0});
            

//var text2 = scene.createText2({wordWrap:true, ellipsis:true, truncation:0,leading:10, clip:false, w:400, h:400, parent:container, textColor:0xFFDDFFFF, pixelSize:20, x:px, y:py, rx:0, ry:1, rz:0});
var metrics = null;
var measurements = null;

text2.ready.then(function(text) {
	console.log("inside text2.ready");

	metrics = text2.getFontMetrics();
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
  console.log("measurements firstCharX="+measurements.firstChar.x);
  console.log("measurements firstCharY="+measurements.firstChar.y);
  console.log("measurements lastCharX="+measurements.lastChar.x);
  console.log("measurements lastCharY="+measurements.lastChar.y);
});
