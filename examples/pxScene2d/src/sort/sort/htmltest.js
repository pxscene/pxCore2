var root = scene.root;
var viewObj = scene.create({
	t: "object",
	fillColor : 0xff0000ff,
	w:1140,h:10,parent:root,
	x:10,clip:false
});
console.log("root w="+root.w+" and h="+root.h);
console.log("viewObj w="+viewObj.w+" and h="+viewObj.h);
var textObj2 = scene.create({
        t: "text2",
		text: "6:17a / 43°",
		faceURL:"/home/comcast/comcast_files/pxCore/examples/pxScene2d/src/jsbindings/fonts/XFINITYSansTT-New-Bold.ttf",
		pixelSize: 20,
		parent: viewObj,
		w :viewObj.w,
		h:viewObj.h,
		x:0,clip:false
    });
console.log("textObj2 w="+textObj2.w+" and h="+textObj2.h);
