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

//"use strict";
px.import({
  scene: 'px:scene.1.js',
  scrollable: 'components/scrollable.js',
  style: 'components/text.style.js',
  keys: 'px:tools.keys.js'}
).then( function ready(imports)
{

  var scene = imports.scene;

  var CONSOLE_VERSION = "1.0";

  var Scrollable = imports.scrollable.Scrollable;
  var style            = imports.style;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  var TEXT_SIZE        = style.textSize;
  var COLOR_TEXT       = style.textColor;
  var COLOR_BACKGROUND = style.backgroundColor;

  var marginTop = style.marginTop;
  var marginBottom = style.marginBottom;
  var marginLeft = style.marginLeft;
  var marginRight = style.marginRight;

  var historyMax = style.historyMax; // lines

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  var fontRes = scene.create({t:"fontResource",url:"FreeSans.ttf"});


  /**
   * Variables
   */
  var Scrollable = imports.scrollable.Scrollable;

  /**
   * Private method to render text
   *
   * @param {String} text source of text content
   */
  function renderText(text) {

    var isNest = false //this.options.args.from === 'markdown';
    marginLeft = isNest ? style.marginLeftForNest : style.marginLeft; // update margin left
    this.scrollable = new Scrollable(this.scene, this.container, {blank: isNest});
    this.scrollable.container.fillColor = COLOR_BACKGROUND;

    this.consoleTxt = scene.create({
      t: "textBox" ,
      parent: this.scrollable.root,   
      interactive: false, 
      x: marginLeft,
      y:  marginTop, 
      w: this.scrollable.getContentWidth() - marginLeft - marginRight,                          
      text: "", 
      textColor: COLOR_TEXT, 
      font: fontRes, 
      pixelSize: TEXT_SIZE, 
      wordWrap: true, 
      lignHorizontal: scene.alignHorizontal.LEFT,
      alignVertical: scene.alignVertical.TOP
    });
    var that = this;
    function appendLog(txt)
    {
      txt = txt + ""; // NB: Force to string

      var lines = txt.split("\n"); // process multi-line output...

      // Add lines to history
      for(var l = 0; l < lines.length; l++)
      {
        if(that.history.length > historyMax)
        {
          that.history.shift();                // remove from HEAD
        }
        that.history.push( lines[l] + "\n" );  // append at   TAIL
      }

      // Assemble new string
      var txt = "";
      for(var h = 0; h < that.history.length; h++)
      {
        txt += that.history[h];
      }

      // Update the Console text ....
      that.consoleTxt.text = txt;
    }

    appendLog(text);
    updateSize.call(this);
    this.renderDefer.resolve(this.consoleTxt);
  }

  function updateSize() {
    this.consoleTxt.w = this.scrollable.getContentWidth() - marginLeft - marginRight;
    var measure = this.consoleTxt.measureText();
    this.scrollable.root.h = measure.bounds.y2 + marginTop + marginBottom;
    this.scrollable.update();
  }

  /**
   * Markdown Text Renderer
   *
   * @param {Object} scene   scene
   * @param {Object} options regular object options
   */
  function MarkdownTextRenderer(scene, options) {
    this.scene = scene;
    this.options = options;

    this._url;
    this.basePath;
    this.scrollable;
    this.consoleTxt;
    this.history    = [];

    this.renderDefer = Promise.defer();
    this.renderReady = this.renderDefer.promise;

    this.container = scene.create({
      t: "object",
      parent: scene.root,
      w: scene.w,
      h: scene.h,
      clip: true,
    });

    // set url
    Object.defineProperty(this, 'url', {
      set: function(url) {
        this._url = url;
        this.renderDefer = Promise.defer();
        this.renderReady = this.renderDefer.promise;

        console.log('start to fetch txt file ' + url);
        px.getFile(url)
          .then((text) => {
            renderText.call(this, text);
          })
          .catch((err) => {
            console.log(err);
            renderText.call(this, "#### Load txt file failed from " + url);
          });
      }
    });

    // read/write props for both container and renderer
    ['w', 'h'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        set: function (val) {
          this.container[prop] = val
          updateSize.call(this);
        },
        get: function () {
          return this.container[prop];
        },
      });
    });
  }

  var txtUrl = px.appQueryParams.url;
  var r = new MarkdownTextRenderer(scene,{/*parent:scene.root*//*myContainer*//*, url:txtUrl, args:{from:""}*/})
  r.url = txtUrl

  function updSize(w,h)
  {
    r.w = w
    r.h = h
  }

  scene.on("onResize", function(e) { updSize(e.w,e.h) })

  updSize(scene.w,scene.h)

}).catch( function importFailed(err){
  console.log("err: " + err);
  console.error("Import for viewText.js failed: " + err)
});