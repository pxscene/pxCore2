/** 
 * This test is to validate changes made for XRE2-579 to no longer 
 * reject promises when animation.
 */

"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var shots = imports.shots;
var manual = imports.manual;

var doScreenshot = shots.getScreenshotEnabledValue();
var testPlatform=scene.info.build.os;

var manualTest = manual.getManualTestValue();
var timeoutForScreenshot = 40;

var basePackageUri = px.getPackageBaseFilePath();

var urls = [
  basePackageUri+"/images/ball.png",
  basePackageUri+"/../../../images/tiles/001.jpg",
  basePackageUri+"/../../../images/tiles/002.jpg",
  basePackageUri+"/../../../images/tiles/003.jpg",
  basePackageUri+"/../../../images/dolphin.jpg",
];
    
var parent = scene.create({t:'object', parent:root, y: 50, clip:false});

var rect1 = scene.create({t:'rect', x:298, y:298, w:304, h:304,  parent:parent, lineWidth:2});

var ball = scene.create({t:'image', w:300, h:300, stretchX:scene.stretch.STRETCH, stretchY:scene.stretch.STRETCH, parent:parent,url:urls[0]});



var tests = {
  // testCancelWithAnimation: creating a new animation for props already being animated should cancel the animation and 
  // cause the promise to be resolved.
  testCancelWithAnimation: function() 
  {
    var results = [];
    return new Promise(function(resolve, reject) {
      
      ball.ready.then(obj=>{
        console.log("ball is ready");
        
        console.log("anim1");
        obj.animateTo({x:500, y:500}, 3.0,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1).then(obj=>{
          console.log("anim1 done");
          results.push(assert(true, "anim1 promise resolution received and expected"));
        }, function reject(obj){ 
          console.log("anim1 rejection");
          results.push( assert(false, "anim1 promise rejection received; promise resolution was expected"));
        }).catch(function() {
          console.log("anim1 error");
          results.push(assert(false, "anim1: Error on promise received; promise resolution was expected"));
         });
         
        // Previous animation is very slow, so this animation should definitely interrupt it
        console.log("anim2");
        ball.animateTo({x:200, y:-50}, 1.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1).then(obj=>{
            console.log("anim2 done");
            results.push(assert(true, "anim2 promise resolution received and expected"));
            results.push(assert(ball.x === 200, "Ball.x is correct"));
            results.push(assert(ball.y === -50, "Ball.y is correct"));
            if( doScreenshot) 
            {
                setTimeout( function() {
                  shots.validateScreenshot(basePackageUri+"/images/screenshot_results/"+testPlatform+"/test_animationRejection.png", false).then(function(match){
                    console.log("test result is match: "+match);
                    results.push(assert(match == true, "screenshot comparison for test_animationRejection failed"));
                    resolve(results);
                  }).catch(function(err) {
                    results.push(assert(false, "screenshot comparison for test_animationRejection failed due to error: "+err));
                    resolve(results);
                });
                }, timeoutForScreenshot);
            } 
            else
              resolve(results);
          }, function reject(obj) {
            console.log("anim2 rejection");
            results.push( assert(false, "anim2 promise rejection received; promise resolution was expected"));
        }).catch(function() {
          console.log("anim2 error");
          results.push(assert(false, "anim2: Error on promise received; promise resolution was expected"));
        });

      
      }); // ball ready
          
      }); //end Promise
  },
  // testCancelWithAnimation: creating a new animation for props already being animated should cancel the animation and 
  // cause the promise to be resolved.
  testCancelWithSet: function() 
  {
    var results = [];
    return new Promise(function(resolve, reject) {
      
      ball.ready.then(obj=>{
        console.log("ball is ready");
        
        console.log("anim1Set");
        obj.animateTo({x:500, y:500}, 5.0,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1).then(obj=>{
          console.log("anim1Set done");
          results.push(assert(true, "anim1Set promise resolution received and expected"));
        }, function reject(obj){ 
          console.log("anim1Set rejection");
          results.push( assert(false, "anim1Set promise rejection received; promise resolution was expected"));
        }).catch(function() {
          console.log("anim1Set error");
          results.push(assert(false, "anim1Set: Error on promise received; promise resolution was expected"));
        }).then(function(obj) {
          results.push(assert(ball.y != 500, "Animation for y property finished when it should not have"));
          resolve(results);
        });

        // Previous animation is very slow, so this set should definitely interrupt it
        // Don't wait for animate to finish. Instead, start a timer to cause 
        // cancellation before it completes
        setTimeout( function() { ball.x = 300;ball.y=20;}, 
                    1000);
        
      
      }); // ball ready
          
    }); //end Promise
  }
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import for test_promiseRejectionAnimation.js failed: " + err)
});
