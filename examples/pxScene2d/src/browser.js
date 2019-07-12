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

var defaultHomeUrl = "https://www.sparkui.org/examples/text/sample.md";

px.configImport({"browser:" : /*px.getPackageBaseFilePath() + */ "browser/"});

var uiReadyResolve = null;
var uiReadyReject = null;
module.exports.uiReady = new Promise(function(resolve, reject) 
{
  uiReadyResolve = resolve;
  uiReadyReject = reject;
});

px.import({ scene:   'px:scene.1.js',
            keys:    'px:tools.keys.js',
            ListBox: 'browser:listbox.js',
            EditBox: 'browser:editbox.js',
            mime:    'browser:mime.js'
}).then( function importsAreReady(imports)
{
  var base = px.getPackageBaseFilePath();

  console.log(">>>>>>>>>>>>>>> browser base", base)

  var style = {
    star: {
      a:0.5
    },
    home: {
      a:0.65
    }
  }

  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;

  var storage = scene.storage

  var hasStorage = storage?true:false

  function getHomeUrl() {
    var homeUrl

    if (storage)
      homeUrl = storage.getItem('.home.')

    if (!homeUrl)
      homeUrl = defaultHomeUrl
    return homeUrl
  }

  var resolveSceneUrl = imports.mime.resolveSceneUrl;

  var url       = "";
  var helpShown = false;

  const LINEAR      = scene.animation.TWEEN_LINEAR;
  const FASTFORWARD = scene.animation.OPTION_FASTFORWARD;
  const LOOP        = scene.animation.OPTION_LOOP;
  const FOREVER     = scene.animation.COUNT_FOREVER;

  var urlFocusColor     = 0x303030ff; // "#303030"
  var urlSucceededColor = 0x0c8508ff; // "#0c8508"
  var urlFailedColor    = 0xde0700ff; // "#de0700"

  var myStretch = scene.stretch.STRETCH;

  var fontRes       = scene.create({ t: "fontResource",  url: "FreeSans.ttf" });

  var version = scene.info.version
  var bgSVG;
  if (version.startsWith('edge_')) {
    bgSVG = '/browser/images/status_bg_edge.svg';
  } else if (version.startsWith('dev')) {
    bgSVG = '/browser/images/status_bg_dev.svg';
  } else {
    bgSVG = '/browser/images/status_bg.svg';
  }

  var bg        = scene.create({t:"image",  parent: root, url:base+bgSVG, stretchX: myStretch, stretchY: myStretch});
  var contentBG = scene.create({t:"rect",   parent: bg, x:10, y:60, fillColor: 0xffffffff, a: 0.05 });
  var content   = scene.create({t:"scene",  parent: bg,      x:10, y:60, clip:true });
  var browser   = scene.create({t:"object", parent: bg} );

  var highlightBG   = scene.create({ t:"rect",   parent: browser, x:10, y:60, fillColor: "#0000", lineColor: "#080", lineWidth: 8, a: 0.0, interactive:false });
  var inputBox = new imports.EditBox( { parent: browser, url: base+'/browser/images/input2.png', x: 70+10, y: 10, w: 800-70-32, h: 35, pts: 24 });
  var listBox = new imports.ListBox( { parent: browser, x: 950, y: 0, w: 0, h: 100, visible:false, numItems:3 });
  var starContainer = scene.create({t:'object', parent: browser, w:28, h:28, cx:14, cy:14})
  var starRes   = scene.create({t:"imageResource", url: base+'/browser/images/star-empty.svg', w:28, h:28});
  var star = scene.create({t:'image', parent:starContainer,h:28, w:28, resource: starRes, a:0.50, interactive:false})
  var starFillRes   = scene.create({t:"imageResource", url: base+'/browser/images/star-filled.svg', w:28, h:28});
  var starFill = scene.create({t:'image', parent:starContainer,h:28, w:28, resource: starFillRes, a:0.5, interactive:false})
  var homeRes   = scene.create({t:"imageResource", url: base+'/browser/images/home-solid.svg', w:28, h:28});
  var home = scene.create({t:'image', parent:browser,h:28, w:28, resource: homeRes, a:/*0.80*/0, interactive:false})
  var spinner   = scene.create({t:'image',  parent: browser, url: base+'/browser/images/spinningball2.png',  y:-80, cx: 50, cy: 50, sx: 0.3, sy: 0.3,a:0.0,interactive:false});
  var backButtonRes = scene.create({ t: 'imageResource', url: base+'/browser/images/arrow-circle-left-solid.svg', w: 32, h: 32 });
  var foreButtonRes = scene.create({ t: 'imageResource', url: base+'/browser/images/arrow-circle-right-solid.svg', w: 32, h: 32 });
  var menuButtonRes = scene.create({ t: 'imageResource', url: base+'/browser/images/bars-solid.svg', w: 24, h: 24 });
  var backButton = scene.create({t:'image', parent:browser, x:10, y:12, h:32, w:32, resource: backButtonRes, a:0.2})
  var foreButton = scene.create({t:'image', parent:browser, x:10+34, y:12, h:32, w:32, resource: foreButtonRes, a:0.2})
  var menuButton = scene.create({t:'image', parent:browser, x:800-28, y:16, h:24, w:24, resource: menuButtonRes, a:0.80})
  var menu = scene.create({t:'image9',parent:menuButton,url:base+'/browser/images/menu.png',insetLeft:10,insetRight:10,
    insetBottom:10,insetTop:10, w:200, h:300, a:0, interactive:false})
  //var menuItem1 = scene.create({t:'rect', parent:menu, x:10, y:10, w:180, h:32, fillColor:0x000000, interactive:false})
  //var menuItem2 = scene.create({t:'rect', parent:menu, x:10, y:42, w:180, h:32, fillColor:0x000000, interactive:false})
  //var menuText1 = scene.create({t:'text', parent:menuItem1, text:'Go to Home', x:5,pixelSize:18, textColor:0x000000c0, interactive:false})
  //var menuText1a = scene.create({t:'text', parent:menuItem1, text:'Ctrl-Alt-H', x:130,y:7,pixelSize:12, textColor:0x000000c0, interactive:false})
  //var menuText2 = scene.create({t:'text', parent:menuItem2, text:'Go to Favorites', x:5, pixelSize:18, textColor:0x000000c0, interactive:false})
  //var menuText2a = scene.create({t:'text', parent:menuItem2, text:'Ctrl-Alt-F', x:130,y:7,pixelSize:12, textColor:0x000000c0, interactive:false})

  //listBox.visible = true

  menuConfig = ['Go to Favorites', 'Go Home', 'Set Home', 'Reset Home']

  menuItem = []
  menuText = []

  function makeMenu(){
    var top = 10
    for(var i = 0; i < menuConfig.length; i++) {
      menuItem[i] = scene.create({t:'rect', parent:menu, x:10, y:top, w:180, h:32, fillColor:0x000000, interactive:false})
      menuText[i] = scene.create({t:'text', parent:menuItem[i], text:menuConfig[i], x:5,pixelSize:18, textColor:0x000000c0, interactive:false})
      top += 32
      var thisMenuItem = menuItem[i]


      thisMenuItem.on('onMouseEnter', function() {
        var thisMenuItem = menuItem[i]
        return function(e) {
          if (e.target == thisMenuItem) {
            thisMenuItem.fillColor=0x00000080
            curMenuItem = thisMenuItem
          }
        }}())

      thisMenuItem.on('onMouseLeave', function() {
        var thisMenuItem = menuItem[i]
        return function(e) {
          if (e.target == thisMenuItem) {
            thisMenuItem.fillColor=0x00000000
            curMenuItem = null
          }
        }}())

    }
    menu.h = top+10
  }

  makeMenu()

  var helpBox  = null;
  var backUrls = []
  var currUrl  = ''
  var foreUrls = []

  var pageInsetL = 20;
  var pageInsetT = 70;

  var showFullscreen = false;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //  CAPABILITIES:  Test for 'inline' SVG support
  //
  var hasSvg = true;

  if( scene.capabilities              == undefined ||
      scene.capabilities.graphics     == undefined ||
      scene.capabilities.graphics.svg == undefined ||
      scene.capabilities.graphics.svg != 2)
  {
    // If *inline* SVG is not supported...
    hasSvg = false;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //  CAPABILITIES:  Test for 'events' DRAG'n'DROP enhanced support
  //
  var hasDragDrop2 = true;

  if( scene.capabilities                    == undefined ||
      scene.capabilities.events             == undefined ||
      scene.capabilities.events.drag_n_drop == undefined ||
      scene.capabilities.events.drag_n_drop != 2)
  {
    // If DRAG'n'DROP enhanced support NOT supported...
    hasDragDrop2 = false;
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  scene.addServiceProvider(function(serviceName, serviceCtx){
    if (serviceName == ".navigate")
      // TODO JRJR have to set url in a timer to avoid reentrancy
      // should move deferring to setUrl method...
      return {setUrl:function(u){setTimeout(function(){
        /*content.url = u;*/ /*inputBox.text = u;*/ reload(u)},1);}}  // return a javascript object that represents the service
    else
      return "allow"; // allow request to bubble to parent
  });

  var currentGen = 0
  function reload(u, keepHistory)
  {
    currentGen++
    inputBox.textColor = urlFocusColor
    var originalUrl = u?u.trim():''

    if (originalUrl == 'browser:favorites')
      u = 'favorites.md'
    else if (originalUrl == 'browser:about')
      u = 'about.js'
    else if (originalUrl == 'browser:home')
      u = getHomeUrl()
    else {
      // if we hit this point... make url in urlbar and effective url match
      u = resolveSceneUrl(originalUrl)
      originalUrl = u
    }

    console.log("RELOADING.... [ " + u + " ]");

    // Prime the Spinner !
    //inputBox.doLater( function() { spinner.a = 1.0; }, 500 ); // 500 ms
    spinner.a = 1.0

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


    // JRJR BUGBUG
    // Promise doesn't seem to fire if url is empty
    if (u == '')
    {
      spinner.a = 0
    }

    inputBox.text = originalUrl;

    if (keepHistory)
    {
      currUrl = u;
    }

    if (true)
    {
      content.ready.then(
        // Use closure to capture the current navigation gen
        // so we can compare at completion
        function(gen) {
          return function(o) {
            listBox.addItem(inputBox.text);
            contentBG.draw = true;
            content.focus = true;

            inputBox.textColor = urlSucceededColor;

            inputBox.hideCursor();
            inputBox.clearSelection()
            //inputBox.cancelLater( function() { spinner.a = 0;} );
            spinner.a = 0

            // Only truncate forward history if we're completing
            // the current matching navigation to resolve
            // a race condition
            if (gen == currentGen) {
              if (!keepHistory) {
                if (u != '' && currUrl != '' && currUrl != u) {
                  backUrls.push(currUrl)
                  foreUrls = []
                }
                currUrl = u
              }
            }

            backButton.a = backUrls.length?0.65:0.2
            foreButton.a = foreUrls.length?0.65:0.2

            if (content.url == getHomeUrl()) {
              home.a = style.home.a
              starContainer.a = 0
              starContainer.interactive = false
            }
            else if (u == 'favorites.md')
            {
              home.a = 0
              starContainer.a = 0
              starContainer.interactive = false
              var favorites = []

              if (storage)
                favorites = storage.getItems('.fav.')

              var doc = ''
              doc += '# Favorites\n'
              var favsFound = false
              for (f of favorites) {
                favsFound = true
                url = f.key.split('.fav.')[1]
                doc += '* [' + url +'](' + url + ')\n'
              }
              if (!favsFound)
                doc += '*No Favorites Found.  Navigate to an application and use the star button in the url bar to add to favorites.*'
              content.api.setContent(doc)
            }
            else {
              home.a = 0
              if (storage) {
                starContainer.a = 1
                starContainer.interactive = true
                var urlKey = '.fav.'+currUrl//content.url
                if (storage.getItem(urlKey)) {
                  star.a = 0
                  starFill.a = style.star.a
                }
                else {
                  star.a = style.star.a
                  starFill.a = 0
                }
              }
            }

          }
        }(currentGen),
        function()
        {
          inputBox.focus = true
          inputBox.selectAll()
          inputBox.textColor = urlFailedColor;

          //inputBox.cancelLater( function() { spinner.a = 0;} );
          spinner.a = 0
        }
      );
    }
  }//reload()

  function goBack() {
    if (backUrls.length) {
      foreUrls.push(currUrl)
      reload(backUrls.pop(), true)
    }
  }

  function goForward() {
    if (foreUrls.length) {
        backUrls.push(currUrl)
        reload(foreUrls.pop(), true)
    }
  }

  backButton.on('onMouseUp', function(e){
    goBack()
  })

  foreButton.on('onMouseUp', function(e){
    goForward()
  })

  starContainer.on('onMouseUp', function(e) {
    var starState = !(starFill.a != 0)

    var urlKey = '.fav.'+currUrl
    if (starState) {
      storage.setItem(urlKey, true)
      starContainer.r = 0
      starContainer.animateTo({r:360, a:1}, 1, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_FASTFORWARD, 1)
      starFill.animateTo({a:0.5},1, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_FASTFORWARD, 1)
      star.animateTo({a:0},1, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_FASTFORWARD, 1)
    }
    else {
      storage.removeItem(urlKey)
      starContainer.r = 360
      starContainer.animateTo({r:0, a:1}, 1, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_FASTFORWARD, 1)
      starFill.animateTo({a:0},1, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_FASTFORWARD, 1)
      star.animateTo({a:0.5},1, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_FASTFORWARD, 1)
    }

  })

  var curMenuItem = null
  var menuClicked = false

  menuButton.on('onMouseDown', function(e){
    menuClicked = false
    menu.a = 1
    //menuItem1.interactive = true
    //menuItem2.interactive = true
    for (var m of menuItem) {
      m.interactive = true
    }
    //menuItem.fillColor=0x00ff00ff
  })

  menuButton.on('onMouseUp', function(e){
    menuClicked = true;
  })

  menuButton.on('onMouseUp', function(e){
    //if (menuClicked)
    //  return

    if (curMenuItem == menuItem[1]) {
      reload('browser:home')
    }
    else if (curMenuItem == menuItem[0]) {
      reload('browser:favorites')
    }
    else if (curMenuItem == menuItem[2]) {
      storage.setItem('.home.', content.url)
      reload('browser:home')
    }
    else if (curMenuItem == menuItem[3]) {
      storage.removeItem('.home.')
      reload('browser:home')
    }

    menu.a = 0
    //menuItem1.interactive = false
    //menuItem2.interactive = false
    for (var m of menuItem) {
      m.interactive = false
    }
  })

/*
  menuItem1.on('onMouseEnter', function(e) {
    if (e.target == menuItem1) {
      menuItem1.fillColor=0x00000080
      curMenuItem = menuItem1
    }
  })

  menuItem1.on('onMouseLeave', function(e) {
    if (e.target == menuItem1) {
      menuItem1.fillColor=0x00000000
      curMenuItem = null
    }
  })

  menuItem2.on('onMouseEnter', function(e) {
    if (e.target == menuItem2) {
      menuItem2.fillColor=0x00000080
      curMenuItem = menuItem2
    }
  })

  menuItem2.on('onMouseLeave', function(e) {
    if (e.target == menuItem2) {
      menuItem2.fillColor=0x00000000
      curMenuItem = null
    }
  })
  */

//##################################################################################################################################

  content.on("onPreMouseUp", function(e)
  {
    //inputBox.focus = false;
    content.focus = true;
  });

  // layout
  function updateSize(w,h)
  {
    console.log("\n\n BROWSER:  Resizing... WxH: " + w + " x " + h + " \n\n");

    bg.w = w;
    bg.h = h;

    // show/hide browser chrome
    browser.a    = showFullscreen ?     0 : 1;

    // Anchor
    content.x   = showFullscreen ?  0 : 10;
    content.y   = showFullscreen ?  0 : 60;

    // Apply insets
    content.w   = showFullscreen ?  w : w - pageInsetL;
    content.h   = showFullscreen ?  h : h - pageInsetT;

    contentBG.x = showFullscreen?0:10
    contentBG.y = showFullscreen?0:60
    contentBG.w   = showFullscreen ?  w : w - pageInsetL;
    contentBG.h   = showFullscreen ?  h : h - pageInsetT;

    highlightBG.w = content.w + 8;
    highlightBG.h = content.h + 8;

    highlightBG.x = content.x - 4;
    highlightBG.y = content.y - 4;

    inputBox.w  = w - pageInsetL - 70 - 32;

    menuButton.x = w-36
    menu.x = -195+24
    menu.y = 24

    helpBox.x   = inputBox.x;
    helpBox.y   = inputBox.y + pageInsetL;

    spinner.x   = inputBox.x+inputBox.w-70;
    spinner.y   = inputBox.y - inputBox.h+2;

/*
    star.x   = inputBox.x+inputBox.w -34;
    star.y   = inputBox.y - inputBox.h +38;
    starFill.x   = inputBox.x+inputBox.w -34;
    starFill.y   = inputBox.y - inputBox.h +38;
*/
    starContainer.x   = inputBox.x+inputBox.w -34;
    starContainer.y   = inputBox.y - inputBox.h +38;

    home.x   = inputBox.x+inputBox.w -32;
    home.y   = inputBox.y - inputBox.h +40;

    if (hasStorage) {
      starContainer.a = 1
      starContainer.interactive = true
      menuButton.a = 0.8
      menuButton.interactive = true
    }
    else {
      starContainer.a = 0
      starContainer.interactive = false
      menuButton.a = 0.3
      menuButton.interactive = false
    }
  }

  scene.root.on("onPreKeyDown", function(e)
  {
    if(keys.is_CTRL_ALT_SHIFT(e.flags) || keys.is_CTRL_ALT(e.flags))
    {
      if (e.keyCode == keys.L )
      {
        inputBox.focus = true;
        inputBox.selectAll();
        e.stopPropagation();
      }
    }

    if( keys.is_CTRL_ALT( e.flags ) ) // CTRL-ALT keys !!
    {
      switch(code)
      {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.LEFT:   //  CTRL-ALT-LEFT
        {
          console.log("goback")
          goBack()
          e.stopPropagation()
        }
        break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.RIGHT:   //  CTRL-ALT-RIGHT
        {
          goForward()
          e.stopPropagation()
        }
        break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.R:   //  CTRL-ALT-R
        {
          console.log("Browser.js Reloading");
          reload(currUrl, true);
          e.stopPropagation();
          console.log("Browser.js reload done");
        }
        break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.R:   //  CTRL-ALT-H
        {
          console.log("browser.js Loading home");
          reload('browser:home');
          e.stopPropagation();
        }
        break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      }//SWITCH
    }//ENDIF  CTRL-ALT
  });

  function showHelp(delay_ms)
  {
    helpBox.animateTo({ a: 1.0  }, 0.75, LINEAR, FASTFORWARD, 1).then
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
          helpBox.animateTo({ a: 0 }, 0.75, LINEAR, FASTFORWARD, 1).then
          (
            function(o) { helpShown = false; }
          )
      }, delay_ms);
  }

  scene.root.on("onKeyDown", function(e)
  {
    var code = e.keyCode;
    // console.log("onKeyDown browser.js  >> code: " + code + " key:" + keys.name(code) + " flags: " + flags);

    if( keys.is_CTRL_ALT( e.flags ) ) // CTRL-ALT keys !!
    {
      switch(code)
      {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.A:  //  CTRL-ALT-A
        {
          console.log("about.js Loading about");
          reload("about.js");
          e.stopPropagation();
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.F:  //  CTRL-ALT-F
        {
          showFullscreen = !showFullscreen;
          updateSize(scene.w, scene.h)
          e.stopPropagation()
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.H:  //  CTRL-ALT-H
        {
          // console.log("browser.js Loading home");
          // reload(homeUrl);
          // e.stopPropagation();
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.K:  //  CTRL-ALT-K
        {
          helpShown ? hideHelp(0) : showHelp(4500); // Hide / Show
          e.stopPropagation();
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.R:   //  CTRL-ALT-R
        {
          // console.log("Browser.js Reloading");
          // reload(currUrl, true);
          // e.stopPropagation();
          // console.log("Browser.js reload done");
        }
        break;
      }//SWITCH
    }
    else
    {
      switch(code)
      {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.PAGEDOWN:
        {
          listBox.visible = !listBox.visible;
          listBox.focus   = !listBox.focus;
          //e.stopPropagation();
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case keys.ENTER:
        if(inputBox.focus == true)
        {
          url = inputBox.text;
          inputBox.moveToEnd();

          reload(url);
          //e.stopPropagation();
        }
        else
        if(listBox.visible == true)
        {
          var listBoxItem = listBox.selectedItem();
          if (listBoxItem == "UNAVAILABLE")
          {
            url = inputBox.text;
          }
          else
          {
            url = listBoxItem;
          }
          reload(url);
          //e.stopPropagation();
        }
        break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      }//SWITCH
    }//ENDIF   CTRL-ALT
  });

  if(hasDragDrop2)
  {
    content.on("onMouseExit", function (e)
    {
      highlightBG.animateTo({ a: 0 }, 0.5, LINEAR, FASTFORWARD, 1);
    });

    content.on("onDragEnter", function (e)
    {
      highlightBG.animateTo({ a: 1.0  }, 0.5, LINEAR, FASTFORWARD, 1);
    });

    content.on("onDragLeave", function (e)
    {
      highlightBG.animateTo({ a: 0 }, 0.25, LINEAR, FASTFORWARD, 1);
    });

    content.on("onDragMove", function (e)
    {
      if(highlightBG.a <=0)
      {
        highlightBG.animateTo({ a: 1 }, 0.25, LINEAR, FASTFORWARD, 1); /// Hmmm
      }
    });

    // Load URL dropped in 'content' area
    content.on("onDragDrop", function (e)
    {
      highlightBG.animateTo({ a: 0 }, 0.5, LINEAR, FASTFORWARD, 1);

      if(e.type == scene.dragType.URL)
      {
        var proto = (e.dropped.charAt(0) == '/') ? "file://" : "";
        var url   =  "" + proto + e.dropped;

        console.log(">>> Loading Dropped URL ... [ " + e.dropped + " ]");
        reload(url);

        e.stopPropagation();
      }
    });
  }

  scene.on("onResize", function(e) { updateSize(e.w,e.h); });

  Promise.all([listBox, inputBox, bg, spinner, content, fontRes])
      .catch( function (err)
      {
          console.log(">>> Loading Assets ... err = " + err);
          
          uiReadyReject();
      })
      .then( function (success, failure)
      {
        inputBox.focus = true;
        spinner.animateTo({r:360},1.0, LINEAR, LOOP, FOREVER);

        helpBox = scene.create({t:"textBox", parent: bg, textColor: 0x202020ff,
                                      x: 20, y: 100,  w: 350, h: 520, a: 0.0,
                                      font: fontRes, pixelSize: 14, wordWrap: true,
                                      interactive: false,  // <<< Essential !
                                      text: " BROWSER: \n\n"+
                                            "  CTRL-ALT-K        ...  Show Keys \n" +
                                            "\n"+
                                            "  CTRL-ALT-A        ...  Show About.js \n" +
                                            "  CTRL-ALT-R        ...  Reload URL \n" +
                                            "  CTRL-ALT-F        ...  Toggle 'Fullscreen' \n" +
                                            "  CTRL-ALT-H        ...  Load 'Browser.js' \n" +
                                            "\n"+
                                            "  CTRL-ALT-SHIFT-L  ...  Load Another URL \n\n" +
                                            " SHELL:   \n\n"+
                                            "  CTRL-ALT-D        ...  Toggle Dirty Rectangles \n" +
                                            "  CTRL-ALT-O        ...  Toggle Outlines \n" +
                                            "  CTRL-ALT-S        ...  Screenshot > screenshot.png \n" +
                                            "  CTRL-ALT-Y        ...  Toggle FPS \n" +
                                            "\n"+
                                            "  CTRL-ALT-SHIFT-D  ...  Log Debug Metrics  \n" +
                                            "  CTRL-ALT-SHIFT-H  ...  Reload HOME \n" +
                                            "  CTRL-ALT-SHIFT-R  ...  Reload BROWSER \n",
                                      alignHorizontal: scene.alignHorizontal.LEFT,
                                      alignVertical:   scene.alignVertical.CENTER})

        updateSize(scene.w, scene.h);
        reload('browser:home')
        
        uiReadyResolve();
      });


}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err);
});

