// This test verifies if the loseFocusChain property is set correctly as part of onBlur event when focus changes.
// The loseFocusChain property is added to onBlur event to indicate if a widget is going to lose the 'focus chain'. 
// Essentially, is focus leaving my hierarchy... 
//
// Here is the scene layout for this test
//
// Root Panel (rect)
//   bg - background Panel (rect)
//      cont1 - Widget (rect)
//          group1 - group (rect)
//              btn11 - focusable element (rect)
//              btn12 - focusable element (rect)
//          group2 - group (rect)
//              btn21 - focusable element (rect)
//              btn22 - focusable element (rect)
//
// Following tests are run
//
// test1:- If we start with focus on btn12, then navigate to btn21, the onBlur target would be btn12 and 'loseFocusChain' flag would be true
//    for btn12 and group1 events, but false for cont1 and bg since btn21 is still within cont1's hierarchy.
//
// test2:- If focus moves from btn21 to btn22, onBlur target would be btn21 and 'loseFocusChain' flag would be true for btn21, 
//    but not for group2 or its parent hierarchy.
//
//
//

"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then(function ready(imports) {

var scene = imports.scene;
var root = scene.root;

var assert = imports.assert.assert;
var manual = imports.manual;
var manualTest = manual.getManualTestValue();

var basePackageUri = px.getPackageBaseFilePath();
var newSceneChild = scene.create({t:'scene',parent:root,url:basePackageUri+"/test_onBlur_loseFocusChainChild.js"});
var theChildApi;
var test1_flag = false;
var test2_flag = false;
var resolveCallback;
var rejectCallback;

var test1Callback = function()
{
    console.log("Inside test1Callback");
    // For test1 loseFocusChain should be set to following.
    // btn12 = true, group1 = true, cont1 = false, bg = false.
    var expected_loseFocusChain_test1 = [true,true,false,false];
    // For test1 onBlur target should be btn12
    var expected_onBlurTarget_test1 = "btn12";

    test1_flag = ((expected_loseFocusChain_test1[0] === theChildApi.actual_loseFocusChain[0]) &&
                        (expected_loseFocusChain_test1[1] === theChildApi.actual_loseFocusChain[1]) &&
                        (expected_loseFocusChain_test1[2] === theChildApi.actual_loseFocusChain[2]) &&
                        (expected_loseFocusChain_test1[3] === theChildApi.actual_loseFocusChain[3]) &&
                        (expected_onBlurTarget_test1 === theChildApi.actual_onBlurTarget[0]) &&
                        (expected_onBlurTarget_test1 === theChildApi.actual_onBlurTarget[1]) &&
                        (expected_onBlurTarget_test1 === theChildApi.actual_onBlurTarget[2]) &&
                        (expected_onBlurTarget_test1 === theChildApi.actual_onBlurTarget[3]));

    resolveCallback(assert(test1_flag === true,"test1 failed"));

}

var test2Callback = function()
{
    console.log("Inside test2Callback");
    // For test2 loseFocusChain should be set to following.
    // btn21 = true, group2 = false, cont1 = false, bg = false.
    var expected_loseFocusChain_test2 = [true,false,false,false];
    // For test2 onBlur target should be btn21
    var expected_onBlurTarget_test2 = "btn21";

    test2_flag = ((expected_loseFocusChain_test2[0] === theChildApi.actual_loseFocusChain[0]) &&
                        (expected_loseFocusChain_test2[1] === theChildApi.actual_loseFocusChain[1]) &&
                        (expected_loseFocusChain_test2[2] === theChildApi.actual_loseFocusChain[2]) &&
                        (expected_loseFocusChain_test2[3] === theChildApi.actual_loseFocusChain[3]) &&
                        (expected_onBlurTarget_test2 === theChildApi.actual_onBlurTarget[0]) &&
                        (expected_onBlurTarget_test2 === theChildApi.actual_onBlurTarget[1]) &&
                        (expected_onBlurTarget_test2 === theChildApi.actual_onBlurTarget[2]) &&
                        (expected_onBlurTarget_test2 === theChildApi.actual_onBlurTarget[3]));

    resolveCallback(assert(test2_flag === true,"test2 failed"));

}

var errorCallback = function()
{
    console.log("inside errorCallback");
    resolveCallback(assert(false,"test failed"));
}

var tests = { 
    
    test1: function() {
        return new Promise(function(resolve, reject) {
            newSceneChild.ready.then(function(obj)  {
            theChildApi = obj.api;
            resolveCallback = resolve;
            rejectCallback = reject;
            theChildApi.launchTest1().then(test1Callback,errorCallback);
            });
        });
    },

    test2: function() {
        return new Promise(function(resolve, reject) {
            newSceneChild.ready.then(function()  {
            // clear the results from previous tests
            theChildApi.actual_loseFocusChain = [];
            theChildApi.actual_onBlurTarget = [];
            resolveCallback = resolve;
            rejectCallback = reject;
            theChildApi.launchTest2().then(test2Callback,errorCallback);
            });
        });
    }
}

module.exports.tests = tests;

if(manualTest === true) 
{
    manual.runTestsManually(tests);
}
 
}).catch(function importFailed(err){
    console.error("Import failed for test_onBlur_loseFocusChain.js: " + err);
});
