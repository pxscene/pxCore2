
px.configImport({"browser:" : /*px.getPackageBaseFilePath() + */ "browser/"});


px.import({ scene:      'px:scene.1.js',
             keys:      'px:tools.keys.js',
             EditBox: 'browser:editbox.js'
}).then( function importsAreReady(imports)
{  
  var url   = "";
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;

  var urlFocusColor     = 0x303030ff;
  var urlSucceededColor = 0x0c8508ff;
  var urlFailedColor    = 0xde0700ff;

  var myStretch = scene.stretch.STRETCH;

  var bg        = scene.create({t:"image", parent: root, url:"browser/images/status_bg.png", stretchX: myStretch, stretchY: myStretch});
  var contentBG = scene.create({t:"rect",  parent: bg, x:10, y:60, fillColor: 0xffffffff, a: 0.05, draw: false});
  var content   = scene.create({t:"scene", parent: bg, x:10, y:60, clip:true});
  var spinner   = scene.create({t:"image", url:"browser/images/spinningball2.png",cx:50,cy:50,y:-30,parent:bg,sx:0.3,sy:0.3,a:0.0});
   
  var inputBox = new imports.EditBox( { parent: bg, url: "browser/images/input2.png", x: 10, y: 10, w: 800, h: 35, pts: 24 });

  function reload(u)
  {
    if (!u)
      u = inputBox.text;
    else
      inputBox.text = u;

    spinner.a = 1;

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
    if (u.indexOf(':') == -1)
      u = 'http://www.pxscene.org/examples/px-reference/gallery/' + u;

    console.log("RELOADING.... [ " + u + " ]");


    content.url    = u;
    content.focus  = true;
    inputBox.focus = false;

    if (true)
    {
      content.ready.then(
        function(o) {
          spinner.a = 0;
          console.log(o);
          contentBG.draw = true;

          inputBox.textColor = urlSucceededColor;
                         
          inputBox.hideCursor();
        },
        function() {
          spinner.a = 0;
          contentBG.draw = false;

          inputBox.textColor = urlFailedColor;

          content.focus = false;
          inputBox.focus = true;
                         
          inputBox.showCursor();
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

    inputBox.w  = w - 100;

    spinner.x  = inputBox.x + inputBox.w;
    spinner.y  = inputBox.y - inputBox.h;
  }

  scene.root.on("onPreKeyDown", function(e)
  {
    if (e.keyCode == keys.L && keys.is_CTRL( e.flags )) { // ctrl-l

      inputBox.focus = true;
      inputBox.selectAll();

      e.stopPropagation();
    }
  });

  scene.root.on("onKeyDown", function(e)
  {
    var code = e.keyCode; var flags = e.flags;
    console.log("123 onKeyDown browser.js  >> code: " + code + " key:" + keys.name(code) + " flags: " + flags);

    if( keys.is_CTRL_ALT( flags ) ) // CTRL-ALT keys !!
    {
      if (code == keys.R)  //  CTRL-ALT-R
      {
        console.log("Browser.js Reloading");
        reload();
        e.stopPropagation();
        console.log("Browser.js reload done");
      }
      else if (code == keys.H)  //  CTRL-ALT-H
      {
        var homeURL = "browser.js";
        console.log("browser.js Loading home");
        reload("gallery.js");
        e.stopPropagation();
      }
    }
    if( code == keys.ENTER)
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

  Promise.all([inputBox, bg, spinner])
      .catch( (err) => 
      {
          console.log(">>> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
        inputBox.focus = true;
        spinner.animateTo({r:360},1.0, scene.animation.TWEEN_LINEAR,
                                       scene.animation.OPTION_LOOP,
                                       scene.animation.COUNT_FOREVER);
        updateSize(scene.w, scene.h);
      });

}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err);
});

