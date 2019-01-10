px.import("px:scene.1.js").then( function ready(scene) {

var root = scene.root;

var view4065 = scene.create({
    t:"object",
    w:636,
    h:283,
    clip:true,
    parent:scene.root
});

var view4070 = scene.create({
    t:"object",
    w:636,
    h:616,		//Calculated from Evaluate Expression
    parent:view4065
});

var view4071 = scene.create({
    t:"object",
    w:636,
    h:324,
    clip:false,
    parent:view4070
});

var text4067 = scene.create({
    t:"textBox",
    w:view4071.w,
    h:view4071.h,
    clip:false,
    text:"A health inspector comes to the Krusty Krab unannounced. Mr. Krabs and SpongeBob have to give one of everything on the menu to the inspector. After the inspector finishes his food he wants a simple Krabby Patty so he can pass the inspection. While SpongeBob and Mr. Krabs are celebrating, a report on the news comes on about a health inspector passing himself off for free food. Thinking it's their inspector that's phoney, they give him the most disgusting Krabby Patty ever. Then the news comes on again saying they caught the culprit. Mr. Krabs and SpongeBob look out the kitchen window they find he is dead. They bury him on a hill and the police coincidentally drive by. Because of the rain, the body slides down the hill from where it is buried. The police officers offer to ride them back.",
   parent:view4071,
   fontUrl:"http://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Lgt.ttf",
   leading:12,
   pixelSize:24,
   alignVertical:0,
   alignHorizontal:0,
   wordWrap:true,
});
var m42; 
text4067.ready.then(function(textObj){
            var textMeasurement = textObj.measureText();
            var height = textMeasurement.bounds.y2 - textMeasurement.bounds.y1;
            console.log("height of total text " +height);
            var fm = textObj.font.getFontMetrics(textObj.pixelSize);
            console.log("Line Spacing " + (fm.height + textObj.leading));
            m42 = -(fm.height + textObj.leading);

});
//var m42 = -41;   
var fancy = function(){
  setTimeout(function(){
  console.log("Going to animate.......");
    view4070.animateTo({"m41":0,"m42":m42}, 1, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1).then(function() {
        console.log("animation done " + "m41 " + view4070.m41 + " m42 " + view4070.m42);
        text4067.x = view4070.m41;
        text4067.y = view4070.m42;
       console.log("new y is "+text4067.y); 
          });
          
    m42 += -41;
    fancy();
  },2000);
};

fancy();

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-57_59.js: " + err)
});
