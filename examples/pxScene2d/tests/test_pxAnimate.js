"use strict";
px.import({scene:"px:scene.1.js",assert:"../test-run/assert.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;

root.w = 800;
root.h = 300;

var basePackageUri = px.getPackageBaseFilePath();


module.exports.beforeStart = function() {
  console.log("test_pxAnimate beforeStart()!");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {
  test1: function() {
  return new Promise(function(resolve, reject) {
    var url = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png";
    var ball = scene.create({t:"image",url:url,parent:root});
    ball.ready.then(function() {
      var startX = 450;
      ball.x = startX;
      var animateX = ball.animate({x:0}, 10.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
      animateX.done.then(function(o){
        console.log("test1: animate X is done");
        var results = [];
        var props = animateX.props;
        var details = animateX.details;
        var xDetails = details["x"];

        results.push(assert(props["x"] === 0,"prop x populated"));

        results.push(assert(animateX.status === "ENDED","animation status not proper"));
        results.push(assert(animateX.type === scene.animation.OPTION_LOOP,"animation type not proper"));
        results.push(assert(animateX.interp === scene.animation.TWEEN_LINEAR,"animation interp not proper"));
        results.push(assert(animateX.provduration === 10.0,"animation prov duration not proper"));
        results.push(assert(animateX.provcount === 1,"animation prov count not proper"));
        results.push(assert(animateX.cancelled === false,"animation cancelled status not proper"));

        results.push(assert(xDetails.from === 450,"animation status not proper for x"));
        results.push(assert(xDetails.to === 0,"animation type not proper for x"));
        results.push(assert(xDetails.duration === 10.0,"animation interp not proper for x"));
        results.push(assert(xDetails.count === 1,"animation prov duration not proper for x"));
        results.push(assert(xDetails.cancelled === false,"animation cancelled status not proper for x"));
        results.push(assert(xDetails.status === "ENDED","animation status not proper for x"));
        resolve(results);
      });
    });
  });
  },
/*
  test2: function() {
  return new Promise(function(resolve, reject) {
    var results = [];
    var url = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png"
    var ball = scene.create({t:"image",url:url,parent:root});
    ball.ready.then(function() {});
    ball.dispose();
    ball.draw = false;
    var rejected = false;
    var animateX = ball.animate({x:0}, 10.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
    animateX.done.then(function(o) {}, function(o) { rejected = true});
    //results.push(assert(rejected === true),"animation promise not rejected");
    resolve(results);
  });
  },

  test3: function() {
  return new Promise(function(resolve, reject) {
    var results = [];
    var url = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png"
    var ball = scene.create({t:"image",url:url,parent:root});
    ball.ready.then(function() {});
    ball.draw = false;
    ball.dispose();
    var rejected = false;
    ball.animateTo({x:0}, 10.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1).then(function(o) {}, function(o) { rejected = true});
    resolve(results);
  });
  },
*/
  test4: function() {
  return new Promise(function(resolve, reject) {
    var results = [];
    var url = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png";
    var ball = scene.create({t:"image",url:url,parent:root});
    ball.ready.then(function() {});
    var animateX = ball.animate({x:0}, 10.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, -1);
    animateX.done.then(function(o) {}, function(o) { });
    results.push(assert(animateX.provduration === 10.0,"animation prov duration not proper"));
    resolve(results);
  });
  },

  test5: function() {
  return new Promise(function(resolve, reject) {
    var results = [];
    var url = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png";
    var ball = scene.create({t:"image",url:url,parent:root});
    ball.x = 0;
    ball.ready.then(function() {});
    var animateX = ball.animate({x:450}, 10.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_FASTFORWARD, 1);
    animateX.done.then(function(o) {}, function(o) { });
    results.push(assert(animateX.provduration === 10.0,"animation prov duration not proper"));
    resolve(results);
  });
  },

  test6: function() {
  return new Promise(function(resolve, reject) {
    var results = [];
    var startX = 450;
    var url = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png";
    var ball = scene.create({t:"image",url:url,parent:root});
    ball.x = startX;
    ball.ready.then(function() {});
    var animateX = ball.animate({x:0}, 10.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_REWIND, 1);
    animateX.done.then(function(o) {}, function(o) { });
    results.push(assert(animateX.provduration === 10.0,"animation prov duration not proper"));
    resolve(results);
  });
  },

  test7: function() {
    return new Promise(function(resolve, reject) {
      var rect1 = scene.create({t:"rect",x:400,y:200,w:100,h:100,fillColor:0xff0000ff,lineWidth:1,lineColor:0xC0C0C0C0,parent:root,a:0.2});
        rect1.ready.then(function(){
          var animate = rect1.animate({sx:5,a:1,lineWidth:0.2},1,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_OSCILLATE,1);
          animate.done.then(function(o) {}, function(o) {});
          var results = [];
          results.push(assert(animate.provduration === 1.0,"animation prov duration not proper in test7"));
          resolve(results);
   });
   });
   }
}
module.exports.tests = tests;

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_pxAnimate.js failed: " + err)
});
