px.import("px:scene.1.js").then( function ready(scene) {

var text4063 = scene.create({
    t:"textBox",
    text:"PRESS ANY KEY FOR GETTING 7 LINES.ONLY 4 LINES WILL BE SHOWN.THIS IS A BUG.Star Trek Into Darkness is a 2013 American science fiction action film. It is the twelfth installment in the Star Trek film franchise, and the sequel to 2009's Star Trek. The film was directed by J. J. Abrams from a screenplay by Roberto Orci, Alex Kurtzman and Damon Lindelof based on the series of the same name created by Gene Roddenberry. Lindelof, Orci, Kurtzman and Abrams are also producers, with Bryan Burk. Chris Pine reprises his role as Captain James T. Kirk, with Zachary Quinto, Karl Urban, Zoe Saldana, Anton Yelchin, Simon Pegg, Leonard Nimoy, John Cho and Bruce Greenwood reprising their roles from the previous film. Benedict Cumberbatch, Peter Weller and Alice Eve round out the film's principal cast.",
    fontUrl:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Lgt.ttf",
    pixelSize:24,
    leading:24,
    wordWrap:true,
    alignVertical:0,
    alignHorizontal:0,
    ellipsis:true,
    truncation:1
});


var view4067 = scene.create({
    t:"object",
    w:764,
    h:324,
    parent:scene.root,
    clip:false
});

text4063.parent = view4067;
text4063.h = view4067.h;
text4063.w = view4067.w;

//Code with issue
text4063.ready.then(function(textObj){
    var fm = textObj.font.getFontMetrics(textObj.pixelSize);
    console.log("fm.height",fm.height);                 //Line spacing calculated only using fm.height
    view4067.h = fm.height*7;		    //height of 7 lines. But only 4 lines are shown
    text4063.h = view4067.h;		    
    }
);
scene.root.on("onKeyDown", function(e) {
//  console.log("charCode is "+e.charCode);
  console.log("keyCode is "+e.keyCode);

});
scene.root.on("onChar", function(e) {
  console.log("Got onChar event for "+e.charCode);
  //Working Code. 
  text4063.ready.then(function(textObj){
      console.log("textBox is ready after onChar");
      var fm = textObj.font.getFontMetrics();
      console.log("fm.height",fm.height);                 //Line spacing calculated using fm.height + textObj.leading
      view4067.h = (fm.height+textObj.leading)*7;		    //height of 7 lines. 
      text4063.h = view4067.h;		    
      }
  );
});

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-57.js: " + err)
});
