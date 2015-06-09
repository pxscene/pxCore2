var text = scene.createText({text:'Hello from context1!',x:50,y:100,rx:1,rz:0,parent:scene.root});
text.animateToF({"r": 360}, 1.0, 0, 2);
scene.createScene({url:"childapp2.js",parent:scene.root});

console.log("afterload childapp2");

// Unfortunately this hangs the other child js context
/*
while(true)
{
    console.log("childapp1.js");
}
*/
