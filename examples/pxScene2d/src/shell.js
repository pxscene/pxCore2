px.import("px:scene.1.js").then(function ready(scene) {

  // JRJR TODO had to add more modules
  var url = queryStringModule.parse(urlModule.parse(module.appSceneContext.packageUrl).query).url;
  var originalURL = (!url || url=="")?"browser.js":url;
  console.log("url:",originalURL);

  var blackBg = scene.create({t:"rect", fillColor:0x000000ff,lineColor:0xffff0080,lineWidth:0,x:0,y:0,w:1280,h:720,a:1,parent:scene.root});
  var childScene = scene.create({t:"scene", url:originalURL,parent:scene.root});
  childScene.focus = true;
  var showFPS = false;
  var fpsBg = scene.create({t:"rect", fillColor:0x00000080,lineColor:0xffff0080,lineWidth:3,x:10,y:10,a:showFPS?1:0,parent:scene.root});
  var fpsCounter = scene.create({t:"text", x:5,textColor:0xffffffff,pixelSize:24,text:"0fps",parent:fpsBg});
  fpsBg.w = fpsCounter.w+16;
  fpsBg.h = fpsCounter.h;
  
  function updateSize(w, h) {
    childScene.w = w;
    childScene.h = h;
    blackBg.w = w;
    blackBg.h = h;
  }
  
  // TODO if I log out event object e... there is extra stuff??
  scene.on("onResize", function(e) { updateSize(e.w, e.h);});
  updateSize(scene.w, scene.h);
  
  
  scene.on("onFPS", function(e) { 
    if(showFPS) {
      fpsCounter.text = ""+Math.floor(e.fps)+"fps"; 
      fpsBg.w = fpsCounter.w+16;
      fpsBg.h = fpsCounter.h;
    }
  });
                                
////
if (false) {
  // TODO Cursor emulation mostly for egl targets right now.

  // TODO hacky raspberry pi detection
  var os = require("os");
  var hostname = os.hostname();
  
  if (hostname == "raspberrypi") {
    var cursor = scene.create({t:"image", url:"cursor.png",parent:scene.root,
				                            interactive:false});
    
    scene.on("onMouseMove", function(e) {
	    cursor.x = e.x-23;
	    cursor.y = e.y-10;
    });
  }
}
////
                                
  scene.root.on("onPreKeyDown", function(e) {
	  var code = e.keyCode; var flags = e.flags;
    console.log("onPreKeyDown:", code, ", ", flags);
       
    if (code == 89 && ((flags & 48)==48))   // ctrl-alt-y
    {
      showFPS = !showFPS
      fpsBg.a = (showFPS)?1.0:0;
      e.stopPropagation();
    }
    if (code == 79 && ((flags & 48) == 48))   // ctrl-alt-o
    {
      scene.showOutlines = !scene.showOutlines;
      e.stopPropagation();
    }
    else if (code == 82 && ((flags & 56) == 56))   // ctrl-alt-r
    {
      console.log("Reloading url: ", originalURL);
      childScene.url = originalURL;
      e.stopPropagation();
    }
    else if (code == 72 && ((flags & 56)==56))   // ctrl-alt-h
    {
      var homeURL = "browser.js";
      console.log("Loading home url: ", homeURL);
      childScene.url = homeURL;
      e.stopPropagation();
    }
    else if (code == 83 && ((flags & 48)==48))  // ctrl-s
    {
      // This returns a data URI string with the image data
      var dataURI = scene.screenshot('image/png;base64');
      // convert the data URI by stripping off the scheme and type information
      // to a base64 encoded string with just the PNG image data
      var base64PNGData = dataURI.slice(dataURI.indexOf(',')+1);
      // decode the base64 data and write it to a file
      fs.writeFile("screenshot.png", new Buffer(base64PNGData, 'base64'), function(err)
      {
        if (err)
          console.log("Error creating screenshot.png");
        else
          console.log("Created screenshot.png");
      });
    }
    else if (code == 67 && ((flags & 16)==16))  // ctrl-c
    {
        // On COPY ... access the Native CLIPBOARD and ADD to top!
        //
        var foo = scene.clipboardSet('clipString SET to Top');
       // console.log("#######  CTRL C >>> COPY !!   foo: " + foo);
        console.log("#######  CTRL C >>> COPY !!   ");
    }
//    else if (code == 86 && ((flags & 16)==16))  // ctrl-v
//    {
//        // On PASTE ... access the Native CLIPBOARD and GET the top!
//        //
//        var myString = "empty";
//                
//        var foo = scene.clipboardGet('clipString GET from Top', myString);
//        console.log("SHELL #######  CTRL V >>> PASTE !!  myString: " + myString);
//    }
  });

  scene.root.on("onPreKeyUp", function(e) {
    console.log("in onPreKeyUp", e.keyCode, e.flags);
	  var code = e.keyCode; var flags = e.flags;
    console.log("onKeyUp:", code, ", ", flags);
                
    // eat the ones we handle here
         if (code == 89 && ((flags & 48)==48)) e.stopPropagation(); // ctrl-alt-y
    else if (code == 79 && ((flags & 48)==48)) e.stopPropagation(); // ctrl-alt-o
    else if (code == 82 && ((flags & 56)==56)) e.stopPropagation(); // ctrl-alt-shift-r
    else if (code == 72 && ((flags & 56)==56)) e.stopPropagation(); // ctrl-alt-shift-h
    else if (code == 83 && ((flags & 48)==48)) e.stopPropagation(); // ctrl-alt-s
  });

  if (true) {
  scene.root.on("onKeyDown", function(e) {
	  var code = e.keyCode; var flags = e.flags;
    console.log("onKeyDown shell:", code, ", ", flags);
    if (code == 82 && ((flags & 48) == 48)) {  // ctrl-alt-r
      console.log("(shell.js) Reloading url: ", originalURL);
      childScene.url = originalURL;
      e.stopPropagation();
    }
    else if (code == 72 && ((flags & 48)==48)) {  // ctrl-alt-h
      var homeURL = "browser.js";
      console.log("Loading home url: ", homeURL);
      childScene.url = homeURL;
      e.stopPropagation();
    }
  });
}

  scene.root.on("onPreChar", function(e) {
    console.log("in onchar");
	  var c = e.charCode;
    console.log("onChar:", c);
	  // TODO eating some "undesired" chars for now... need to redo this
    if (c<32) {
      console.log("stop onChar");
      e.stopPropagation()
    }
  });
  
  // TODO if I log out event object e... there is extra stuff??
  scene.on("onResize", function(e) { updateSize(e.w, e.h);});
  updateSize(scene.w, scene.h);

});
