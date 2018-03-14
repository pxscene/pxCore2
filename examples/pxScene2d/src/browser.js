/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/


px.configImport({"browser:" : /*px.getPackageBaseFilePath() + */ "browser/"});


px.import({ scene:      'px:scene.1.js',
             keys:      'px:tools.keys.js',
             EditBox: 'browser:editbox.js'
}).then( function importsAreReady(imports)
{  
  var url   = "";
  var helpShown = false;

  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;

  var urlFocusColor     = 0x303030ff;
  var urlSucceededColor = 0x0c8508ff;
  var urlFailedColor    = 0xde0700ff;

  var myStretch = scene.stretch.STRETCH;

  var fontRes   = scene.create({ t: "fontResource",  url: "FreeSans.ttf" });

  var bg        = scene.create({t:"image", parent: root, url:"browser/images/status_bg.png", stretchX: myStretch, stretchY: myStretch});
  var contentBG = scene.create({t:"rect",  parent: bg, x:10, y:60, fillColor: 0xffffffff, a: 0.05, draw: false});
  var content   = scene.create({t:"scene", parent: bg, x:10, y:60, clip:true});
  var spinner   = scene.create({t:"image", url:"browser/images/spinningball2.png",cx:50,cy:50,y:-80,parent:bg,sx:0.3,sy:0.3,a:0.0});
   
  var inputBox = new imports.EditBox( { parent: bg, url: "browser/images/input2.png", x: 10, y: 10, w: 800, h: 35, pts: 24 });
  var helpBox   = null;

  scene.addServiceProvider(function(serviceName, serviceCtx){
    if (serviceName == ".navigate")
      // TODO JRJR have to set url in a timer to avoid reentrancy
      // should move deferring to setUrl method... 
      return {setUrl:function(u){setTimeout(function(){
        content.url = u;},1);}}  // return a javascript object that represents the service
    else
      return "allow"; // allow request to bubble to parent
  });  

  scene.on('onClose', function(e) {
    keys = null;
    for (var key in inputBox) { delete inputBox[key]; }
    inputBox = null;
    scene = null;
  });

  function reload(u)
  {
    if (!u)
      u = inputBox.text;
    else
      inputBox.text = u;

    if (u.indexOf("local:") === 0) // LOCAL shorthand
    {
      var txt = u.slice(6, u.length);
      var pos = txt.indexOf(':');
      if ( pos == -1)
      {
        u = "http://localhost:8080/" + txt;   // SHORTCUT:   "local:filename.js""  >>  "http://localhost:8080/filename.js" (default to 8080)
      }
      else
      {
        var str = txt.split('');
        str[pos] = "/"; // replace : with /
        txt = str.join('');

        u = "http://localhost:" + txt;       // SHORTCUT:   "local:8081:filename.js" >> "http://localhost:8081/filename.js""
      }

      url = u;
    }

    // TODO Temporary hack
    if(u == "about.js")
    {
        u = "about.js";
    }
    else        
    if (u.indexOf(':') == -1)
    {
      u = 'http://www.pxscene.org/examples/px-reference/gallery/' + u;
      //  inputBox.text = u;
    }

    console.log("RELOADING.... [ " + u + " ]");

    // Prime the Spinner !
    inputBox.doLater( function() { spinner.a = 1.0; }, 500 ); // 500 ms

    if(false)
    {
      // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
      setTimeout(function delayLoadURL() { // Simulate latency in URL loading

          content.url = u;
          inputBox.cancelLater( function() { spinner.a = 0;} );
      }, 3000);
      // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
    }
    else
    {
      content.url = u;
    }

    inputBox.focus = false;

    if (true)
    {
      content.ready.then(
        function(o)
        {
          console.log(o);
          contentBG.draw = true;
          content.focus = true;

          inputBox.textColor = urlSucceededColor;
                         
          inputBox.hideCursor();
          inputBox.cancelLater( function() { spinner.a = 0;} );
        },
        function()
        {
          contentBG.draw = false;

          inputBox.textColor = urlFailedColor;

          content.focus = false;
          inputBox.focus = true;
                         
          inputBox.showCursor();
          inputBox.cancelLater( function() { spinner.a = 0;} );
        }
      );
    }
  }//reload()

//##################################################################################################################################

  content.on("onMouseUp", function(e)
  {
    inputBox.focus = false;
    content.focus=true;
  });  

  function updateSize(w,h)
  {
    // console.log("Resizing...");

    bg.w = w;
    bg.h = h;

    // Apply insets
    content.w   = w - 20;
    content.h   = h - 70;

    contentBG.w = w - 20;
    contentBG.h = h - 70;  

    inputBox.w  = w - 20;

    helpBox.x   = inputBox.x;
    helpBox.y   = inputBox.y + 20;

    spinner.x   = inputBox.x + inputBox.w - 60;
    spinner.y  = inputBox.y - inputBox.h;
  }

  scene.root.on("onPreKeyDown", function(e)
  {
    if(keys.is_CTRL( e.flags ) ||
       keys.is_CMD ( e.flags ) )
    {
      if (e.keyCode == keys.L )
      {
      inputBox.focus = true;
      inputBox.selectAll();

      e.stopPropagation();
      }
    }
  });

  function showHelp(delay_ms)
  {
    helpBox.animateTo({ a: 1.0  }, 0.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1).then
    (
      function(o) {
        helpShown = true;
        hideHelp(delay_ms);  // auto hide
      }
    );
  }

  function hideHelp(delay_ms)
  {
      setTimeout(function()
      {
          helpBox.animateTo({ a: 0 }, 0.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1).then
          (
            function(o) { helpShown = false; }
          )
      }, delay_ms);
  }

  scene.root.on("onKeyDown", function(e)
  {
    var code = e.keyCode; var flags = e.flags;
   // console.log("onKeyDown browser.js  >> code: " + code + " key:" + keys.name(code) + " flags: " + flags);

    if( keys.is_CTRL_ALT( flags ) ) // CTRL-ALT keys !!
    {
      if (code == keys.R)  //  CTRL-ALT-R
      {
        console.log("Browser.js Reloading");
        reload();
        e.stopPropagation();
        console.log("Browser.js reload done");
      }
      else if (code == keys.A)  //  CTRL-ALT-A
      {
        console.log("about.js Loading about");
        reload("about.js");
        e.stopPropagation();
      }
      else if (code == keys.H)  //  CTRL-ALT-H
      {
        var homeURL = "browser.js";
        console.log("browser.js Loading home");
        reload("gallery.js");
        e.stopPropagation();
      }
      else
      if(e.keyCode == keys.K)  //  CTRL-ALT-K
      {
        helpShown ? hideHelp(0) : showHelp(4500); // Hide / Show
      
        e.stopPropagation();
    }
    }
    else if( code == keys.ENTER && inputBox.focus == true)
    {
      url = inputBox.text;
      inputBox.moveToEnd();

      reload(url);
    }
    else
    {
      inputBox.textColor = urlFocusColor;
      inputBox.showCursor();
    }
  });

  scene.on("onResize", function(e) { updateSize(e.w,e.h); });

  Promise.all([inputBox, bg, spinner, content, fontRes])
      .catch( function (err)
      {
          console.log(">>> Loading Assets ... err = " + err);
      })
      .then( function (success, failure)
      {
        inputBox.focus = true;
        spinner.animateTo({r:360},1.0, scene.animation.TWEEN_LINEAR,
                                       scene.animation.OPTION_LOOP,
                                       scene.animation.COUNT_FOREVER);

        helpBox = scene.create({t:"textBox", parent: bg, textColor: 0x202020ff,
                                      x: 20, y: 100,  w: 350, h: 520, a: 0.0,
                                      font: fontRes, pixelSize: 14, wordWrap: true,
                                      interactive: false,  // <<< Essential !
                                      text: " BROWSER: \n\n"+
                                            "  CTRL-ALT-K        ...  Show Keys \n" +
                                            "\n"+
                                            "  CTRL-ALT-A        ...  Show About.js \n" +
                                            "  CTRL-ALT-R        ...  Reload URL \n" +
                                            "  CTRL-ALT-H        ...  Load 'Browser.js' \n\n" +
                                            " SHELL:   \n\n"+
                                            "  CTRL-ALT-D        ...  Toggle Dirty Rectangles \n" +
                                            "  CTRL-ALT-O        ...  Toggle Outlines \n" +
                                            "  CTRL-ALT-S        ...  Screenshot > screenshot.png \n" +
                                            "  CTRL-ALT-Y        ...  Toggle FPS \n" +
                                            "\n"+
                                            "  CTRL-ALT-SHIFT-H  ...  Log Debug Metrics \n" +
                                            "  CTRL-ALT-SHIFT-H  ...  Reload HOME \n" +
                                            "  CTRL-ALT-SHIFT-R  ...  Reload BROWSER \n",
                                      alignHorizontal: scene.alignHorizontal.LEFT,
                                      alignVertical:   scene.alignVertical.CENTER})

        updateSize(scene.w, scene.h);
      });

}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err);
});

