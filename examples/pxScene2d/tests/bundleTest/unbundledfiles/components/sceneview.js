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


px.import({ scene: 'px:scene.1.js',
             keys: 'px:tools.keys.js'
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var apps  = imports.apps;

  const HL = scene.alignHorizontal.LEFT;
  const HC = scene.alignHorizontal.CENTER;
  const VC = scene.alignVertical.CENTER;

  var AR = scene.w / scene.h;

  var cols = 3;
  
  var ww = Math.round(scene.w/cols);
  var hh = Math.round(ww / AR);

  var bw = 30; // .. 10 px border

  var dw = ww - (2 * bw); // .. less border
  var dh = Math.round(dw / AR);

  let caption_h = 80;
  let that = null;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  var okFullTimer  = null
  var lastToTimer  = null

  function SceneView(scene, container)
  {
    that = this;

    var appFrame_w = scene.w;
    var appFrame_h = appFrame_w / AR;

    var font = scene.create({ t: "fontResource", url: "FreeSans.ttf" });
 
    this.appWindow = scene.create({ t: "object", parent: container, x: scene.w/ 2, y: scene.h / 2, 
                                w:  appFrame_w ,
                                h:  appFrame_h + caption_h, px: 0.5, py: 0.5, sx: 0.0, sy: 0.0,
                                cx: appFrame_w /2, 
                                cy: (appFrame_h + caption_h) / 2, id: " APP WINDOW",
                                focus: true, fillColor: "#2f2"});

    this.appFrame = scene.create({ t: "rect", parent: this.appWindow,
                                w: appFrame_w, h: appFrame_h, id: " APP FRAME ",
                                cx: appFrame_w/2, cy: appFrame_h/2,
                                fillColor: "#000", lineColor: "#888", lineWidth: 4, interactive: false, a: 1});

    this.titleBox = scene.create({ t: "rect", parent: this.appWindow, x: 0, y: appFrame_h, w: appFrame_w, h: caption_h, 
                                    interactive: false, fillColor: "#222", a: 1 });

    // TEXT - title, description
    var common1 = { parent: this.titleBox, t: "textBox", interactive: false, w: appFrame_w, h: 25,  pixelSize: 22, 
                          alignHorizontal: HC, alignVertical: VC, font: font, textColor: "#fff" };

    this.title       = scene.create( Object.assign(common1, { x: 0, y: 10, text: ""  }) );
    this.description = scene.create( Object.assign(common1, { x: 0, y: 40, text: "", wordWrap: false,  textColor: "#888"  }) );

    // TEXT - OK, LAST messages
    var common2 = { parent: this.titleBox, t: "textBox", interactive: false, w: appFrame_w/4, h: 25,  pixelSize: 18, a: 0,
                          alignHorizontal: HC, alignVertical: VC, font: font, textColor: "#eee" };

    this.okFullscreen = scene.create( Object.assign(common2, { x: appFrame_w, y: 40, px: 1.0, py: 0.0, text: "Press OK for Fullscreen" }) );
    this.lastToCancel = scene.create( Object.assign(common2, { x: 0,          y: 40, px: 0.0, py: 0.0, text: "Press LAST to Close"     }) );

    this.ready        =  Promise.all([this.appWindow.ready, this.appFrame.ready, this.titleBox.ready, this.title.ready, 
                                      this.description.ready, this.okFullscreen.ready, this.lastToCancel.ready]);

    this.appScene = null;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    this.appWindow.on('onPreKeyDown', function (e)
    {
      var window = e.target;

      if(e.keyCode == keys.ENTER)
      {
        window.animateTo({ sx: 1.00, sy: 1.00, py: 0.45 }, 0.5,
          scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
      }    
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    this.showInfo = function(info)
    {
        this.title.text       = info.title;
        this.description.text = info.description;

        return Promise.all([ this.title.ready, this.description.ready ])
        .then( function(o)
        {
          that.appWindow.animateTo({ a: 1.0, sx: 0.75, sy: 0.75, x: scene.w/2, y: scene.h/2}, 0.5,
            scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);

          that.appScene = scene.create({ t: "scene", parent: that.appFrame, url: info.attributes.url, 
                                      x: 10, y: 10, w: that.appFrame.w - 20, h: that.appFrame.h - 20, clip: true });

          that.appScene.ready.then( function(o)
          {
            if(okFullTimer == null)
            {
              // Show message after a delay
              okFullTimer = displayAfter( that.okFullscreen, 3000, 3000 );
            }

            if(lastToTimer == null)
            {
              // Show message after a delay
              lastToTimer = displayAfter( that.lastToCancel, 6000, 3000 );
            }

            that.appScene.focus = true;
          })
          .catch( (err) => {
            console.error("Exception: >> " + info.attributes.url + " << launch failed.  Err:" + err);
          });
        });
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    this.hideInfo = function(o)
    {
      return new Promise(function (resolve, reject)
      {
        that.appWindow.animateTo({ a: 0, sx: 0, sy: 0 }, 0.25,
          scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1)
        .then( function(o)
        {
          if(that.appScene)
          {
            that.appScene.remove();
            that.appScene = null;
          }

          if(that.okFullTimer)
          {
            clearTimeout(that.okFullTimer);
            that.okFullTimer = null;
          }

          resolve();
        });
      });
    }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function displayAfter(control, waitBefore, waitAfter )
  {
    var timer = setTimeout(function ()
    {
      control.animateTo({ a: 1.0 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1)
        .then(function ()
        {
          timer = setTimeout(function ()
          {
            control.animateTo({ a: 0.0 }, 0.5, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1)
              .then(function (o)
              {
                timer = null;
              });
          }, waitAfter); // Display for 'waitAfter' milliseconds
        });
    }, waitBefore); // Wait for 'waitBefore' milliseconds

    return timer;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  this.updateSize = function (w, h)
  {
    this.appWindow.x = w/2;
    this.appWindow.y = h/2;

    AR = w / h;
  }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    this.animate = function ( params, t, e1, e2, l)
    {
      this.appWindow.animate( params, t, e1, e2, l);
    }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  container.on('onPreKeyDown', function (e)
  {
    if(that.appScene && e.keyCode == keys.ENTER)
    {
      that.appWindow.sx = 1.0; 
      that.appWindow.sy = 1.0; 
      that.appWindow.y  = scene.h/2 + caption_h/2;

      e.stopPropagation(); 
    }
  });

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  }//class

  module.exports = SceneView;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}).catch(function importFailed(err) {
  console.error("Import for sceneview.js failed: " + err);
});

