var root = scene.root;
// "Hello!  How are you?";//
var longText = "Here is a collection of a bunch of randomness, like words, phrases, and sentences that isn't supposed to make any kind of sense whatsoever. I want to test capital AV next to each other here. In generating this, I'm listening to someone talking, trying to make sense of what they are saying, and at the same time dictating to myself what I am going to type along with actually typing it out, recognizing when I make mistakes, and correcting myself when I do.  I don't think I'm doing a very good job listening to whoever it is that is doing the talking right now.  It probably would have been a lot easier to just copy and paste something from the net, but I'm a typist, not a person that hunts and pecks to find the appropriate key on the keyboard.  Though I do think I'm probably off of my 30 word per minute speed from way back when.  How much more text is appropriate?  Why do I use words like appropriate when I could just say will do instead?  These and other questions generally go on unanswered.  But looking at the output of this text, I realize that its simply not enough and that I need to add more text; which is making me wonder why I simply didn't copy and paste in the first place.  Ah, yes, the strange musings of a software engineer.";
//var longText2 = " when I could just say will do instead?  These and other questions generally go on unanswered.  But looking at the output of this text, I realize that its simply not enough and that I need to add more text; which is making me wonder why I simply didn't copy and paste in the first place.  Ah, yes, the strange musings of a software engineer.";
root.w=800;
var rect = scene.create({t:"rect", parent:root, x:100, y:100, w:400, h:400, lineColor:0xFF0000FF, lineWidth:1, clip:false});
var container = scene.create({t:"object", parent:root, x:100, y:100, w:800, h:600, clip:false});

// widgets for tracking current property settings
var truncationStatus = scene.create({t:"text", parent:container, x:20, y:420, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"truncation=truncate"});
var wrapStatus = scene.create({t:"text", parent:container, x:20, y:440, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"wordWrap=true"});
var hAlignStatus = scene.create({t:"text", parent:container, x:20, y:460, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"hAlign=left"});
var vAlignStatus = scene.create({t:"text", parent:container, x:20, y:480, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"vAlign=top"});
var ellipsisStatus = scene.create({t:"text", parent:container, x:20, y:500, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"ellipsis=true"});
var clipStatus = scene.create({t:"text", parent:container, x:20, y:520, textColor:0xFFDDFFFF, pixelSize:20,clip:false,text:"clip=true"});
var px = 0;
var py = 0;
var leading = 0;

// Use fontUrl to load from web
//var fontUrl = "http://localhost/XRE2/fonts/DancingScript-Bold.ttf";
var text2 = scene.create({t:"text2", clip:true, parent:container, x:px, y:py, rx:0, ry:0, rz:0});
   text2.h=400;
   text2.w=400;
   text2.textColor=0xFFDDFFFF;
   text2.pixelSize=20;
   text2.leading=0;
   text2.faceURL="DancingScript-Bold.ttf";
   text2.horizontalAlign=0;
   text2.verticalAlign=0;
   text2.xStartPos=0;
   text2.xStopPos=0;
	 text2.wordWrap=true;
   text2.ellipsis=true;
   text2.truncation=1;

   
           
   text2.text=longText; 
//   text2.text+=longText2;

                 

//var text2 = scene.createText2({wordWrap:true, ellipsis:true, truncation:0,leading:10, clip:false, w:400, h:400, parent:container, textColor:0xFFDDFFFF, pixelSize:20, x:px, y:py, rx:0, ry:1, rz:0});
var metrics = null;
var measurements = null;


function textready(text) {
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

function cycleValues(v) {
    console.log("v is "+v);
    if( v >= 2) {
      v = 0;
    } else {
      v++;
    }
    console.log("v is now"+v);
    return v;
}
scene.root.on("onChar", function(e) {
  var v; 
  if (e.charCode == 119) { // w for wordWrap
    text2.wordWrap = !text2.wordWrap;
    if( text2.wordWrap) {
      wrapStatus.text ="wordWrap=true";
    } else {
      wrapStatus.text ="wordWrap=false";
    }
  } else if(e.charCode == 116) { // t for truncation

    v = cycleValues(text2.truncation);
    text2.truncation = v //t;
    if(v == 0) {
      truncationStatus.text="truncation=none";
    } else if(v ==1) {
      truncationStatus.text="truncation=truncate";
    } else {
      truncationStatus.text="truncation=truncate at word boundary";
    }
  } else if(e.charCode == 101) { // e for ellipsis
    text2.ellipsis = !text2.ellipsis;
    if( text2.ellipsis) {
      ellipsisStatus.text ="ellipsis=true";
    } else {
      ellipsisStatus.text ="ellipsis=false";
    }    
  } else if(e.charCode == 104) { // h for horizontalAlign
    v = cycleValues(text2.horizontalAlign);
    text2.horizontalAlign = v;
    if(v == 0) {
      hAlignStatus.text="hAlign=left";
    } else if(v ==1) {
      hAlignStatus.text="hAlign=center";
    } else {
      hAlignStatus.text="hAlign=right";
    }   
  } else if(e.charCode == 118) { // v for verticalAlign
    v = cycleValues(text2.verticalAlign);
    text2.verticalAlign = v;
    if(v == 0) {
      vAlignStatus.text="vAlign=top";
    } else if(v ==1) {
      vAlignStatus.text="vAlign=center";
    } else {
      vAlignStatus.text="vAlign=bottom";
    }    
  } else if(e.charCode == 99) { // c for clip
    text2.clip  = !text2.clip;
    if( text2.clip) {
      clipStatus.text ="clip=true";
    } else {
      clipStatus.text ="clip=false";
    }    
  }
  text2.ready.then(function(text) {
    console.log("!CLF: Promise received");
    textready(text);
  });
});
