var root = scene.root;

var urlFocus = process.cwd() + "/../../images/focusMenuLine.png";
var urlLine = process.cwd() + "/../../images/grayMenuLine.png";
var text = ["TV", "Movies", "Kids", "Network", "Music", "Latino", "Streampix", "Other Stuff"];
var textObjs = [];
var focusIndx = 0;
var focusColor = 0x0080c0ff;
var xPos=25;
var pad = 25;
var winWidth = root.w;


var lineTop = scene.createImage9({url:urlLine,x:25,y:25,h:2,parent:root});
var lineTopFocus = scene.createImage9({url:urlFocus,x:25,y:25,w:50,h:5,parent:root});
var lineBtm	= scene.createImage9({url:urlLine,x:25,y:75,h:2,parent:root});
var lineBtmFocus = scene.createImage9({url:urlFocus,x:25,y:72,w:50,h:5,parent:root});


for( var i = 0; i < text.length; i++){
	var t = scene.createText({text:text[i],
			parent:root,
			x:xPos,
			y:25,
			textColor:0xffffffff,
			pixelSize:30});

	xPos+=t.w + pad;
	textObjs[i] = t;


}

lineTopFocus.w = textObjs[focusIndx].w;
lineTopFocus.x = textObjs[focusIndx].x;
lineBtmFocus.w = textObjs[focusIndx].w;
lineBtmFocus.x = textObjs[focusIndx].x;
textObjs[focusIndx].textColor = focusColor;
lineTop.w = xPos-(pad*2);
lineBtm.w = xPos-(pad*2);


scene.root.on('onKeyDown', function(e) {
	console.log("lineTop.w is "+lineTop.w);

	textObjs[focusIndx].textColor = 0xffffffff;
	  if (e.keyCode == 39) {
		if( focusIndx < (textObjs.length-1)) {
			focusIndx++;
		} else {
			focusIndx = 0;
		}		  

	  } else if(e.keyCode == 37 ) {
		  if( focusIndx > 0) {
			  focusIndx--;
		  } else {
			  focusIndx = textObjs.length -1;
		  }
	  }
	  lineTopFocus.w = textObjs[focusIndx].w;
	  lineBtmFocus.w = textObjs[focusIndx].w;
      lineTopFocus.animateTo({x:textObjs[focusIndx].x},0.3, scene.PX_STOP, scene.PX_END);
      lineBtmFocus.animateTo({x:textObjs[focusIndx].x},0.3, scene.PX_STOP, scene.PX_END);
      textObjs[focusIndx].textColor = focusColor;
	});
