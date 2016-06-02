px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;
  
  // CHARACTER BY CHARACTER RENDERING
  var longText = "Here is a collection of a bunch of random words, phrases, and sentences that isn't supposed to make any kind of sense whatsoever.  In generating this, I'm listening to someone talking, trying to make sense of what they are saying, and at the same time dictating to myself what I am going to type along with actually typing it out, recognizing when I make mistakes, and correcting myself when I do.";
  var rect = scene.create({t:"rect", parent:root, w:400, h:400, lineColor:0x334433FF, lineWidth:1});
  var container = scene.create({t:"object", parent:root, w:400, h:400});
  var length = longText.length;
  var px = 0;
  var py = 0;
  var leading = 0;
  var kearning = 0;
  
  for (i=0; i<length; i++) {
	  var temp = scene.create({t:"text", parent:container, textColor:0xFFDDFFFF, pixelSize:20, x:px, y:py, rx:0, ry:1, rz:0});
	  temp.text = longText.charAt(i);
	  temp.cx = temp.w/2;
	  temp.cy = temp.h/2;
	  if (px + temp.w > container.w) {
		  py += temp.h + leading;
		  px = 0;
		  temp.x=px;
		  temp.y=py;
	  }
	  px += temp.w + kearning;
  }
  
  function animate(index) {
	  container.children[index].animateTo({r:360}, 2, 0, 0);
  }
  
  for (var i=0; i<length; i++) {
	  setTimeout(function(n){return function(){animate(n);}}(i), i*20);
  }
});
