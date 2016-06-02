var root = scene.root;

/*
// TEST LOADING OF JSON FILE
 
var fs = require('fs');
var gridFile = process.cwd() + "/bob/z_15h_grid.json";
var gridData = null;
fs.readFile(gridFile, 'utf8', function(err, data) {
	if (err) {
		console.log("Error loading grid file: " + err);
		return;
	}
	gridData = JSON.parse(data);
	console.log("Grid data loaded and parsed!!!");
	console.log("Did object load?  " + gridData.getGridResponse.channels[0].channelUrn);
	
});
*/

/*
// BEGINNING OF GRID

var gridScene = null;
var highlighter = null;
var currentRow = 0;
var currentShow = 0;
var lastXMovement = 0;


function doGrid() {
	gridScene = scene.create({t:"rect", parent:root, y:150, w:root.w, h:250, fillColor:0x001144FF});
	var rows = 5;
	var indent = 10;
	for (r=0; r<rows; r++) {
		var ypos = r*50;
		var rowScene = scene.create({t:"object", parent:gridScene, y:ypos, w:root.w, h:50});
		var xpos=0;
		var rect = null;
		while (xpos < 800) {
			var width = randomInt(1, 4) * 100; // each half hour segment is 100 pixels
			//var width = 200;
			var program = scene.create({t:"object",  parent:rowScene, x:xpos, y:0, w:width, h:50, clip:true});
			// Create a object, that has a rectangle, text object and a image object
			var textWidth = width - indent;
			var programText = scene.create({t:"text", parent:program, x:indent, textColor:0xAAAAFFFF, pixelSize:28, w:textWidth, text:"really long program title"});
			var fadeX = width - 70; // width of image
			var imgUrl = process.cwd() + "/bob/blue_fade_right.png";
			var programImage = scene.create({t:"image", parent:program, x:fadeX, url:imgUrl});
			console.log("row: " + r + "  width: " + width + "  xpos: " + xpos + "  fadeX: " + fadeX);
			var programRect = scene.create({t:"rect", parent:program, x:0, y:0, w:width, h:50, fillColor:0x00000000, lineColor:0xCCCCCCFF, lineWidth:1});
			xpos += width;
		}
	}
	//highlighter = scene.create({t:"rect", parent:root, x:gridScene.x, y:gridScene.y, h:gridScene.children[0].h, w:gridScene.children[0].children[0].w,
	//				fillColor:0xCCCCFF33, lineColor:0x00FFFFFF, lineWidth:4});
	var imgUrl = process.cwd() + "/bob/selector.png";
	highlighter = scene.create({t:"image9", parent:root, x:gridScene.x, y:gridScene.y, 
								h:gridScene.children[0].h, w:gridScene.children[0].children[0].w, 
								url:imgUrl, x1:3, y1:3, x2:3, y2:3});
	
}

function randomInt(from, to) {
	var range = to-from;
	return Math.round(Math.random()*range + from);
}

function findIndex(newRow, xpos) {
	var row = gridScene.children[newRow];
	var index = 0
	console.log("**** row.length: " + row.children.length + " " + xpos);	
	for (i=0; i<row.children.length; i++) {
		console.log("*** i, cell.x, xpos " + i + ", " + row.children[i].x + ", " + xpos);
		if (row.children[i].x > xpos) {
			return index;
		} else if (row.children[i].x == xpos) {
			return i;
		} else if (i+1 == row.children.length) {
			return i;
		}
		index = i;
	}
	return 0;
}
	

scene.root.on('onKeyDown', function(e) {
	// change text color of old show.
	gridScene.children[currentRow].children[currentShow].children[0].textColor=0xAAAAFFFF;
	
  	var nextX;
  	var nextY;
  	var nextW;
  	var nextH;
	console.log("keydown:" + e.keyCode);
	var currentX = gridScene.children[currentRow].children[currentShow].x;
	if (e.keyCode == 40) {				// down
		if (currentRow + 1 >= gridScene.children.length) {
			currentRow = 0;
		} else {
			currentRow++;
		}
		currentShow = findIndex(currentRow, lastXMovement);
	} else if (e.keyCode == 38) {		// up
		if (currentRow -1 < 0) {
			currentRow = gridScene.children.length - 1;
		} else {
			currentRow--;
		}
		currentShow = findIndex(currentRow, lastXMovement);
	} else if (e.keyCode == 39) {		// right
		if (currentShow + 1 >= gridScene.children[currentRow].children.length) {
			currentShow = 0;
		} else {
			currentShow++;
		}
		lastXMovement = gridScene.children[currentRow].children[currentShow].x;
	} else if (e.keyCode == 37) {		// left
		if (currentShow - 1 < 0) {
			currentShow = gridScene.children[currentRow].children.length - 1;
		} else {
			currentShow--;
		}
		lastXMovement = gridScene.children[currentRow].children[currentShow].x;
	}
	
	nextY = gridScene.y + gridScene.children[currentRow].y;
	var moveTo = {	x:gridScene.children[currentRow].children[currentShow].x,
					y:nextY, 
					w:gridScene.children[currentRow].children[currentShow].w, 
					h:gridScene.children[currentRow].h};
	
	console.log(moveTo);
	
	highlighter.animateTo(moveTo, .2, 0, 0);
	gridScene.children[currentRow].children[currentShow].children[0].textColor=0xFFFFFFFF;
});


doGrid();
console.log("Current folder: " + process.cwd());
 
// end of GRID
*/

/*
// 9 SLICE IMAGE
	var imgUrl = process.cwd() + "/bob/selector.png";
	var highlighter = scene.create({t:"image9", parent:root, x:gridScene.x, y:gridScene.y, 
								h:gridScene.children[0].h, w:gridScene.children[0].children[0].w, 
								url:imgUrl, x1:2, y1:2, x2:38, y2:13});
*/



/*

// ANIMATION
 
var myrect = scene.createRectangle({parent:root, x:250, y:100, cx:150, cy:100, w:300, h:200, fillColor:0xDD0033FF, lineColor:0xFF0000FF, lineWidth:4});
var rotate = 0;

function f1() {
	rotate+=360;
	myrect.rx=1;
	myrect.rz=0;
	myrect.animateTo({r:rotate}, 2, 0, 0, f2);	
}

function f2() {
	rotate+=360;
	myrect.rx=0;
	myrect.ry=1;
	myrect.animateTo({r:rotate}, 2, 0, 0, f3);
}

function f3() {
	rotate+=90;
	myrect.ry=0;
	myrect.rz=1;
	myrect.animateTo({r:rotate}, 2, 0, 0, f1);
}

f1();
*/

/*	

// ANIMATION

function animateMe() {
	myrect.rx=1;
	myrect.rz=0;
	myrect.animateTo({r:360}, 2, 0, 0, function() {
		myrect.rx=0;
		myrect.ry=1;
		myrect.animateTo({r:0}, 2, 0, 0, function () {
			myrect.ry=0;
			myrect.rz=1;
			myrect.animateTo({r:90}, 2, 0, 0, animateMe());
		});
	});
};

animateMe();
*/


/*
// LOAD IMAGE

var url = process.cwd() + "/bob/ql.jpg";
var bg = scene.create({t:"image", url:url,parent:root});
bg.on("ready", function(){ console.log("Image successfully loaded: ", bg.w)});
bg.sx = 150.0/bg.w;
bg.sy = 150.0/bg.h;



function updateSize(w, h) {
    bg.w = w;
    bg.h = h;
    //bgShade.w = w;
    //bgShade.h = h;
    //txt1.y = h-txt1.h;
}


//scene.on("onResize", function(e) {console.log("fancy resize", e.w, e.h); updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);
*/


/*
// BASIC TEXT

var text = scene.create({t:"text", textColor:0xFF0000FF, 
	pixelSize:20, parent:root, x:50, y:50 });
//text.text = "SCRUM CAN SEE MAN";
text.text = "scrum can see man";
console.log("x, y, w, h " + text.x + " " + text.y + " " + text.w + " " + text.h + " " + text.clip);
//text.animateTo({textColor:0xFF0000FF}, 15, 5, 1);
//var rect = scene.createRectangle({parent:root, x:50, y:50, cx:150, cy:100, w:100, h:20, fillColor:0xDD003300, lineColor:0xFFFFFFFF, lineWidth:1});
*/

/*
// CHARACTER BY CHARACTER RENDERING
var longText = "Here is a collection of a bunch of random words, phrases, and sentences that isn't supposed to make any kind of sense whatsoever.  In generating this, I'm listening to someone talking, trying to make sense of what they are saying, and at the same time dictating to myself what I am going to type along with actually typing it out, recognizing when I make mistakes, and correcting myself when I do."; // I don't think I'm doing a very good job listening to whoever it is that is doing the talking right now.  It probably would have been a lot easier to just copy and paste something from the net, but I'm not a pecker, meaning a person that hunts and pecks to find the appropriate key on the keyboard.  Though I do think I'm probably off of my 30 word per minute speed from way back when.  How much more text is appropriate?  Why do I use words like appropriate when I could just say will do instead?  These and other questions generally go on unanswered.  But looking at the output of this text, I realize that its simply not enough and that I need to add more text; which is making me wonder why I simply didn't copy and paste in the first place.  Ah, yes, the strange musings of a software engineer.";
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
/*
function animate(index) {
	container.children[index].animateTo({r:360}, 2, 0, 0);
}

for (i=0; i<length; i++) {
	//animate(i);
	setTimeout(animate(i), i*100);
}
*/



// IMAGE TEST

//var imgurl = process.cwd() + "/../../images/banana.png";
//var imgurl = "http://www.auduboninstitute.org/sites/default/files/imagecache/highlight_large/photos/elephant_show_0.jpg";
var imgurl = "http://54.146.54.142/images/dvd_tile.187.jpg";
console.log(imgurl);
var image = scene.createImage({url:imgurl, parent:root});
image.w = 200;
image.h = scene.h;


/*
// MOVE RECTANGLE TEST
var container = scene.create({t:"rect", parent:root, x:50, y:50, w:250, h:50, fillColor:0xDD0033FF, lineColor:0xFFFFFFFF, lineWidth:1});
var child = scene.create({t:"text", parent:container, x:10, y:10, w:150, h:30, textColor:0xFFDDFFFF, pixelSize:20, text:"Hello way out there!"});
container.children[0].x = 250;
*/
