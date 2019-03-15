// TODO... 
// * Xuse passed in url/file
// * Xfix onClose
// * Xfix xsetInterval and xclearInterval
// * Xbug when having a working gl scene and load non-existent gl scene
// * load nonexistent gl scene blue context??
// * Xmultiple loadings of gl scenes
// * requestAnimationTimer
// * fix deletionn of gl context (swith to MIke's new api)
// * multiple opengl scenes at same time
// * key events, onSize etc... 
// * view events... event delegation?? to scene
// * why do I need to pass in beginDrawing and endDraving
// NOTE TO self... declarations with no "var" are in global namespace... danger
// are setInterval and clearInterval polluting global namespace... ?
// /Users/johnrobinson/code/pxgl/examples/pxScene2d/external/spark-webgl/examples/2-triangle/triangle.js

var _timers = require('timers')
var fs = require('fs')
var path = require('path')

var _intervals = []

function loadUrl(url, beginDrawing,endDrawing, _view) {

  var succeeded = false

  view = _view

  setInterval = function(f,i){
    var interval = _timers.setInterval(function() {
      return function() { 
        beginDrawing(); f(); endDrawing(); }
      }(),i)
    _intervals.push(interval)
    return interval
  }

  clearInterval = function(interval) {
    var index = _intervals.indexOf(interval);
    if (index > -1) {
      _intervals.splice(index, 1);
    }
    _timers.clearInterval(interval)
  }

  var filename = ''

  if (url.startsWith('gl:'))
    filename = url.substring(3)

  var initGLPath = __dirname+'/initGL.js'

  try {
    require(filename)
    succeeded = true
  }
  catch(e) {}

  try {
    var module = require.resolve(initGLPath)
    if (typeof require.cache[module] != undefined)
      delete require.cache[module]
  }
  catch(e) {}

  try {
    var module = require.resolve(filename)
    if (typeof require.cache[module] != undefined)
      delete require.cache[module]
  }
  catch(e) {}

  return succeeded
}

var _clearIntervals = function() {
  for(var interval of _intervals) {
    _timers.clearInterval(interval)
  }
  _intervals = []
}

function onClose() {
  _clearIntervals()
}

exports.loadUrl = loadUrl;
exports.onClose = onClose;
