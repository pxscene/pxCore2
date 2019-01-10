"use strict";
px.import({scene:"px:scene.1.js",
           keys:"px:tools.keys.js" 
          }).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var keys = imports.keys;

var container = scene.create({t:"object", parent:root, w:400, h:400, clip:false});
var text2 = scene.create({t:"textBox", text:"I am the RECEIVER....",  x:0, y:0, w:400, h:400, parent:container, textColor:0xFF7000FF, pixelSize:20});

var textLog = scene.create({t:"textBox", x:0, y:50, w:400, h:400, parent:container, textColor:0xFF7000FF, pixelSize:20});

root.on("onPreChar", function(e) {
  console.log("receiverApp got onPreChar: "+e);
  textLog.text = "Got onPreChar '"+e.charCode+"'";
})
module.exports.onPreKeyDown = 
//root.on("onPreKeyDown", 
function (e) {
  console.log("receiverApp got onPreKeyDown: "+e);
  textLog.text = "Got onPreKeyDown '"+e.keyCode+"'";
  if(e.keyCode == keys.HOME) {
    console.log("Receiver is stopping propagation");
    return true;
  }
  if(e.keyCode == keys.ZERO) { // ZERO
    var appToResume = scene.getService(".optimus").getApplicationById("fonts2");
    scene.getService(".optimus").getApplicationById("answers").a = 0;
    //appToResume.moveToFront();
    appToResume.resume();
    appToResume.setFocus();
    scene.getService(".optimus").getApplicationById("answers").suspend();
    return true; // stop propagation!
  }
  else if(e.keyCode == keys.ONE) {// ONE
    var appToResume = scene.getService(".optimus").getApplicationById("answers");
    appToResume.a = 1;
    appToResume.moveToFront();
    appToResume.resume();
    appToResume.setFocus();
    scene.getService(".optimus").getApplicationById("fonts2").suspend();
    return true;
  }
  
  return false;
};

module.exports.onPreChar = 

function (e) {
  console.log("receiverApp got onPreChar: "+e);
  textLog.text = "Got onPreChar '"+e.charCode+"'";
  // Everything should already have been handled in onPreKeyDown, so 
  // just stop propagation here for handled keys
  if(e.charCode == 48) { // ZERO
    return true; // stop propagation!
  }
  else if(e.charCode == 49) {// COMMA
    return true;
  }
  
  return false;
};

root.on("onPreKeyDown", function(e) {
  console.log("receiverApp got onPreKeyDown: "+e);
  textLog.text = "Got onPreKeyDown '"+e.keyCode+"'";

})
root.on("onKeyDown", function(e) {
  console.log("receiverApp got onKeyDown: "+e);
  textLog.text = "Got onKeyDown '"+e.keyCode+"'";
})

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for receiverApp.js failed: " + err)
});