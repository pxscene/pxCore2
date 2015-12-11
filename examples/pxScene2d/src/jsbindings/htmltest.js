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
        t: "textBox",
		text: "6:17a / 43°",
		fontUrl:"http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-Bold.ttf",
		pixelSize: 20,
		parent: viewObj,
		w :viewObj.w,
		h:viewObj.h,
		x:0,clip:false
    });
console.log("textObj2 w="+textObj2.w+" and h="+textObj2.h);
