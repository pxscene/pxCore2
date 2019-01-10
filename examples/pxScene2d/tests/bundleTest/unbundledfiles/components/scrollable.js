/**
 * Scrollable block
 */
'use strict'

px.configImport({
  "utils:" : px.getPackageBaseFilePath() + "/components/utils/"
});

px.import({
 sceneEventEmitter: 'utils:sceneEventEmitter.js',
  style: './scrollable.style.js',
   keys: 'px:tools.keys.js'
}).then(function importsAreReady(imports) {

  var style = imports.style;
  var getSceneEventEmitter = imports.sceneEventEmitter.getSceneEventEmitter;
  var keys  = imports.keys;
  /**
   * Shows scrollbars if inner content too big
   *
   * Supports vertical scrollbars only
   *
   * @param {Object} scene  scene
   * @param {Object} parent parent
   * @param {Object} render additional args {blank:}
   */
  function Scrollable(scene, parent, args) {
    this.scene = scene;
    this.parent = parent;
    this.options = JSON.parse(JSON.stringify(style)); // deep clone
    this.args = args || {};
    this.isDragging = false;

    if(this.args.blank){ // ignore
      this.options.scrollbarStyle.w = 0;
      this.options.lineWidth = 0;
      this.options.scrollbarHandleStyle.w = 0;
      this.options.scrollbarHandleActiveStyle.w = 0;
    }
    this.sceneEE = getSceneEventEmitter(scene);
    this.mouseDownYStart;
    this.scrollbarHandleMargin = Math.round(
      (this.options.scrollbarStyle.w - this.options.scrollbarHandleStyle.w) / 2
    );

    this.container = scene.create(Object.assign({
      t: "rect",
      id: "scroller content",
      x: 0,
      y: 0,
      parent: parent,
      interactive: false,
    }, this.options.containerStyle));

    this.content = scene.create({
      t: "object",
      id: "scroller content",
      x: 0,
      y: 0,
      parent: this.container,
      interactive: false,
    });

    this.scrollbar = scene.create(Object.assign({
      t: "rect",
      id: "scroller bar",
      y: 0,
      w: this.options.scrollbarStyle.w,
      parent: this.container,
    }, this.options.scrollbarStyle));

    this.scrollbarHandle = scene.create(Object.assign({
      t: "rect",
      id: "scroller handle",
      parent: this.scrollbar,
      x: this.scrollbarHandleMargin,
      y: this.scrollbarHandleMargin,
      w: this.options.scrollbarStyle.w,
    }, this.options.scrollbarHandleStyle));

    this.ready = Promise.all(
      [ 
        this.container.ready, 
        this.content.ready, 
        this.scrollbar.ready, 
        this.scrollbarHandle.ready
      ]);

    // bind event handlers to the scope
    this.onMouseEnter = this.onMouseEnter.bind(this);
    this.onMouseLeave = this.onMouseLeave.bind(this);
    this.onMouseDown  = this.onMouseDown.bind(this);

    this.onSceneMouseDown = this.onSceneMouseDown.bind(this);
    this.onSceneMouseMove = this.onSceneMouseMove.bind(this);
    this.onSceneMouseUp   = this.onSceneMouseUp.bind(this);
    this.update           = this.update.bind(this);

    // attach events
    this.scrollbarHandle.on('onMouseEnter', this.onMouseEnter);
    this.scrollbarHandle.on('onMouseLeave', this.onMouseLeave);
    this.scrollbarHandle.on('onMouseDown', this.onMouseDown);
    this.scrollbar.on('onMouseUp', this.onScrollBarClick.bind(this));

    this.sceneEE.on('onMouseDown', this.onSceneMouseDown);
    this.sceneEE.on('onResize', this.update);

    scene.root.on("onKeyDown", this.onKeyDown.bind(this));

    // expose this.content as a root for users of this class
    this.root = this.content;

    // calculate positions of all scrollbar elements
    this.update();
  }

  Scrollable.prototype.onKeyDown = function(e) {
    var code = e.keyCode; var flags = e.flags;
    console.log("DEBUG: onKeyDown > [ " + e.keyCode + " ]   << No Key modifier");
    var currentY =  this.scrollbarHandle.y;
    var maxY = this.scrollbar.h - this.scrollbarHandleMargin - this.scrollbarHandle.h;
    switch(code)
    {
      case keys.UP: {
        if(style.useArrowKeys)
        {
          this.doScroll(currentY - style.keyboardDiffHeight, maxY);
        }
        break;
      }

      case keys.DOWN: {
        if(style.useArrowKeys)
        {
          this.doScroll(currentY + style.keyboardDiffHeight, maxY);
        }
        break;
      }

      case keys.PAGEUP: {
        if(style.usePageKeys)
        {
          this.doScroll(currentY + this.scrollbarHandleMargin - style.rowScrollHeight, maxY);
        }
        break;
      }

      case keys.PAGEDOWN: {
        if(style.usePageKeys)
        {
          this.doScroll(currentY + this.scrollbarHandleMargin + style.rowScrollHeight, maxY);
        }
        break;
      }
    }
  }
  Scrollable.prototype.getContentWidth = function() {
    return this.container.w - this.options.scrollbarStyle.w;
  }

  Scrollable.prototype.update = function()
  {
    this.container.w = this.parent.w;
    this.container.h = this.parent.h;

    this.content.w = this.container.w - this.options.scrollbarStyle.w;
    this.content.h = Math.max(this.container.h, this.content.h);

    var scrollDiff = this.content.h - this.container.h;

    this.content.y = Math.max(this.content.y, - scrollDiff);

    this.scrollbar.x = this.container.w - this.options.scrollbarStyle.w;
    this.scrollbar.h = this.container.h;

    this.maxY = this.scrollbar.h - this.scrollbarHandleMargin - this.scrollbarHandle.h;

    this.scrollbarHandle.draw = (scrollDiff > 0);
    this.scrollbar.draw       = (scrollDiff > 0);

    if (scrollDiff)
    {
      this.scrollbarHandle.h = (this.scrollbar.h - 2 * this.options.scrollbarStyle.lineWidth)
      * this.container.h / this.content.h;

      this.scrollbarHandle.y = this.clampScrollbarHandleY(
        - this.content.y / scrollDiff
        * (this.scrollbar.h - this.scrollbarHandle.h)
      );
    }
  }

  Scrollable.prototype.onMouseEnter = function() {
    Object.assign(this.scrollbarHandle, this.options.scrollbarHandleActiveStyle);
  }

  Scrollable.prototype.onMouseLeave = function() {
    Object.assign(this.scrollbarHandle, this.options.scrollbarHandleStyle);
  }

  Scrollable.prototype.onSceneMouseDown = function(evt) {
    this.mouseDownYStart = evt.y;
  }

  Scrollable.prototype.onMouseDown = function() {
    this.sceneEE.on('onMouseMove', this.onSceneMouseMove);
    this.sceneEE.on('onMouseUp',   this.onSceneMouseUp);
    // we have to stop scrolling when mouse goes beyond scene
    // as we don't get events from beyond scene
    // so we cannot know if user releases the scrollbar handle outside of the scene
    this.sceneEE.on('onMouseLeave', this.onSceneMouseUp);
    this.isDragging = true; // think is draging
  }

  Scrollable.prototype.onScrollBarClick = function(evt){
    if (this.isDragging) {
      return;
    }
    var newY = Math.max(evt.y - this.scrollbarHandle.h*0.5, this.scrollbarHandleMargin);
  
    this.doScroll(newY, this.maxY);
  }

  Scrollable.prototype.onSceneMouseMove = function(evt) {
    var newY = Math.max(this.scrollbarHandle.y + evt.y - this.mouseDownYStart, this.scrollbarHandleMargin);
    this.mouseDownYStart = evt.y;
  
    this.doScroll(newY, this.maxY);
  }

  Scrollable.prototype.doScroll = function(newY, maxY)
  {
    if(this.content.y == newY ) return; // do nothing 

    newY = Math.max(0, newY);
    newY = Math.min(newY, maxY);
    this.scrollbarHandle.y = newY;

    var scrollRate =   (newY - this.scrollbarHandleMargin) / maxY;
    var posY       = - (this.content.h - this.container.h) * scrollRate;
  //  this.content.y = - (this.content.h - this.container.h) * scrollRate;

  this.content.animateTo({  y: posY }, 0.2,
    this.scene.animation.TWEEN_LINEAR, this.scene.animation.OPTION_FASTFORWARD, 1);

    console.log("#### this.content.y = " + posY)
  }

  Scrollable.prototype.onSceneMouseUp = function() {
    this.mouseDownYStart = null;
    var that = this;
    setTimeout(function() {
      that.isDragging = false;
    }, 32); // next tick set value, make sure all mouse event dispacthed

    this.sceneEE.off('onMouseMove',  this.onSceneMouseMove);
    this.sceneEE.off('onMouseUp',    this.onSceneMouseUp);
    this.sceneEE.off('onMouseLeave', this.onSceneMouseUp);
  }

  Scrollable.prototype.clampScrollbarHandleY = function(y) {
    var minY = this.scrollbarHandleMargin;

    return Math.min(Math.max(y, minY), this.maxY);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // scrollPos
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Scrollable.prototype.getScrollPos = function()    { return this.content.y };
  Scrollable.prototype.setScrollPos = function(pos) { this.doScroll(pos, this.maxY) };

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    

  module.exports.Scrollable = Scrollable;

}).catch(function importFailed(err) {
  console.error("Import failed: " + err);
});

