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

/**
 * Scrollable block
 */
'use strict'

px.import({
  style: 'scrollable.style.js',
  keys: 'px:tools.keys.js'
}).then(function importsAreReady(imports) {

  var style = imports.style;
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
    this.mouseDownYStart;
    this.scrollbarHandleMargin = Math.round(
      (this.options.scrollbarStyle.w - this.options.scrollbarHandleStyle.w) / 2
    );

    this.container = scene.create(Object.assign({
      t: "rect",
      x: 0,
      y: 0,
      parent: parent,
      interactive: false,
    }, this.options.containerStyle));

    this.content = scene.create({
      t: "object",
      x: 0,
      y: 0,
      parent: this.container,
      interactive: false,
    });

    this.scrollbar = scene.create(Object.assign({
      t: "rect",
      y: 0,
      w: this.options.scrollbarStyle.w,
      parent: this.container,
    }, this.options.scrollbarStyle));

    this.scrollbarHandle = scene.create(Object.assign({
      t: "rect",
      parent: this.scrollbar,
      x: this.scrollbarHandleMargin,
      y: this.scrollbarHandleMargin,
      w: this.options.scrollbarStyle.w,
    }, this.options.scrollbarHandleStyle));

    // bind event handlers to the scope
    this.onMouseEnter       = this.onMouseEnter.bind(this);
    this.onMouseLeave       = this.onMouseLeave.bind(this);
    this.onMouseDown        = this.onMouseDown.bind(this);
    //this.onScrollWheel      = this.onScrollWheel.bind(this);

    this.onSceneMouseDown   = this.onSceneMouseDown.bind(this);
    this.onSceneMouseMove   = this.onSceneMouseMove.bind(this);
    this.onSceneScrollWheel = this.onSceneScrollWheel.bind(this);

    this.onSceneMouseUp = this.onSceneMouseUp.bind(this);
    this.update = this.update.bind(this);

    // attach events
    this.scrollbarHandle.on('onMouseEnter', this.onMouseEnter);
    this.scrollbarHandle.on('onMouseLeave', this.onMouseLeave);
    this.scrollbarHandle.on('onMouseDown', this.onMouseDown);
    this.scrollbar.on('onMouseUp', this.onScrollBarClick.bind(this));
    this.scene.on('onMouseDown', this.onSceneMouseDown);
    this.scene.on('onResize', this.update);

    this.scene.root.on('onScrollWheel', this.onSceneScrollWheel.bind(this));
    this.scene.root.on("onKeyDown", this.onKeyDown.bind(this));

    // expose this.content as a root for users of this class
    this.root = this.content;

    // calculate positions of all scrollbar elements
    this.update();
  }

  Scrollable.prototype.onKeyDown = function(e) {
    var code = e.keyCode; var flags = e.flags;
    // console.log("DEBUG: onKeyDown > [ " + e.keyCode + " ]   << No Key modifier");

    var currentY =  this.scrollbarHandle.y;
    var maxY = this.scrollbar.h - this.scrollbarHandleMargin - this.scrollbarHandle.h;
    switch(code)
    {
      case keys.UP: {
        this.doScroll(currentY - style.keyboardDiffHeight, maxY);
        break;
      }

      case keys.DOWN: {
        this.doScroll(currentY + style.keyboardDiffHeight, maxY);
        break;
      }
    }
  }
  Scrollable.prototype.getContentWidth = function() {
    return this.container.w - this.options.scrollbarStyle.w;
  }

  Scrollable.prototype.update = function() {
    this.container.w = this.parent.w;
    this.container.h = this.parent.h;

    this.content.w = this.container.w - this.options.scrollbarStyle.w;
    this.content.h = Math.max(this.container.h, this.content.h);

    var scrollDiff = this.content.h - this.container.h;

    this.content.y = Math.max(this.content.y, - scrollDiff);

    this.scrollbar.x = this.container.w - this.options.scrollbarStyle.w;
    this.scrollbar.h = this.container.h;

    this.scrollbarHandle.draw = scrollDiff;
    if (scrollDiff) {
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

  Scrollable.prototype.onMouseDown = function(e) {
    this.scene.on('onMouseMove', this.onSceneMouseMove);
    this.scene.on('onMouseUp', this.onSceneMouseUp);
    // we have to stop scrolling when mouse goes beyond scene
    // as we don't get events from beyond scene
    // so we cannot know if user releases the scrollbar handle outside of the scene

    // JRJR BUG BUG window mouse capture bug
    this.scene.on('onMouseLeave', this.onSceneMouseUp);
    this.isDragging = true; // think is draging
    // mouse down in scrollbar thumb don't let scrollbar track
    // get it
    e.stopPropagation()
  }

  Scrollable.prototype.onScrollBarClick = function(evt){
    if (evt.target != this.scrollbar)
      return
    var newY = Math.max(evt.y - this.scrollbarHandle.h*0.5, this.scrollbarHandleMargin);
    var maxY = this.scrollbar.h - this.scrollbarHandleMargin - this.scrollbarHandle.h;
    this.doScroll(newY, maxY);
  }

  Scrollable.prototype.onSceneMouseMove = function(evt) {
    if (!this.isDragging)
      return;
    var newY = Math.max(this.scrollbarHandle.y + evt.y - this.mouseDownYStart, this.scrollbarHandleMargin);
    this.mouseDownYStart = evt.y;
    var maxY = this.scrollbar.h - this.scrollbarHandleMargin - this.scrollbarHandle.h;
    this.doScroll(newY, maxY);
  }

  Scrollable.prototype.onSceneScrollWheel = function(evt) {
    //this.isDragging = true; // think is dragging

    var currentY =  this.scrollbarHandle.y;
    var maxY = this.scrollbar.h - this.scrollbarHandleMargin - this.scrollbarHandle.h;
  
    currentY = currentY-evt.dy

    // if an object can scroll consume the event.
    // Otherwise let it propogate naturally to see if a parent can
    // scroll
    if ((currentY > this.scrollbarHandleMargin && evt.dy > 0) || (currentY < maxY && evt.dy < 0)) {
      this.doScroll(currentY, maxY)
      evt.stopPropagation()
    }

  }

  Scrollable.prototype.doScroll = function(newY, maxY) {
    newY = Math.max(0, newY);
    newY = Math.min(newY, maxY);
    this.scrollbarHandle.y = newY;
    var scrollRate = (newY - this.scrollbarHandleMargin) / maxY;
    this.content.y = - (this.content.h - this.container.h) * scrollRate;
  }

  Scrollable.prototype.onSceneMouseUp = function(e) {
    this.mouseDownYStart = null;
    var that = this;

    this.isDragging = false

    this.scene.delListener('onMouseMove', this.onSceneMouseMove);
    this.scene.delListener('onMouseUp', this.onSceneMouseUp);
    this.scene.delListener('onMouseLeave', this.onSceneMouseUp);    
  }

  Scrollable.prototype.clampScrollbarHandleY = function(y) {
    var minY = this.scrollbarHandleMargin;
    var maxY = this.scrollbar.h - this.scrollbarHandleMargin - this.scrollbarHandle.h;

    return Math.min(Math.max(y, minY), maxY);
  }

  module.exports.Scrollable = Scrollable;

}).catch(function importFailed(err) {
  console.error("Import failed: " + err);
});

