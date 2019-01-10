"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene  = imports.scene;
var root   = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var svgSource = "<rect fill='#1E90FF' stroke='#fff' stroke-width='3' x='0' y='0' width='190' height='90'/>";
var svgData   =  "data:image/svg," + svgSource;

var base64src  = "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";
var base64data = "data:image/png;base64," + base64src;


var container = scene.create({t:'object',parent:root});

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_pxImageResource64svg.js ... beforeStart ...");
  var promise = new Promise(function(resolve,reject) {
    resolve(assert(true,"beforeStart succeeded"));
  });
  return promise;
}

var tests = {

  testImageFromDataURI_SVG: function()
  {
    console.log("Running testImageFromDataURI_SVG");
    return new Promise(function(resolve, reject)
    {
      var results = [];

      scene.create({t: "image",url: svgData, parent: container}).ready
      .then(function(o)
      {
        console.log("IMAGE SVG RESOURCE >> url: " + o.url)
        var ans = "md5sum/svg,00990844208069C127C80ADEF2B0673B"

        results.push(assert(o.url        === ans, "IMAGE SVG >> image url is not correct when queried"));
        results.push(assert(o.w          ===  -1, "IMAGE SVG >> image.w is not correct when queried"));
        results.push(assert(o.h          ===  -1, "IMAGE SVG >> image.w is not correct when queried"));
        results.push(assert(o.resource.w === 190, "IMAGE SVG >> image.resource.w is not correct when queried"));
        results.push(assert(o.resource.h ===  90, "IMAGE SVG >> image.resource.h is not correct when queried"));
      },
      function(o) {//rejected
        results.push(assert(false, "image load failed : "+o));
      }).catch(function(o) {
          results.push(assert(false, "image load failed : "+o));
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testImageFromDataURI_SVG");
      });
    }); // return Promise()
  },// test function()

  testImageResourceFromDataURI_SVG: function()
  {
    console.log("Running testImageResourceFromDataURI_SVG");
    return new Promise(function(resolve, reject)
    {
      var results = [];

      var res = scene.create({t: "imageResource",url: svgData });
      var img = scene.create({t: "image", resource: res, parent: container });

      Promise.all([res.ready, img.ready]).then(function(o)
      {
        console.log("IMAGE SVG RESOURCE >> url: " + img.url)
        var ans = "md5sum/svg,00990844208069C127C80ADEF2B0673B"

        results.push(assert(img.url        === ans, "IMAGE SVG RESOURCE >> image url is not correct when queried"));
        results.push(assert(img.w          ===  -1, "IMAGE SVG RESOURCE >> image.w is not correct when queried"));
        results.push(assert(img.h          ===  -1, "IMAGE SVG RESOURCE >> image.w is not correct when queried"));
        results.push(assert(img.resource.w === 190, "IMAGE SVG RESOURCE >> image.resrouce.w is not correct when queried"));
        results.push(assert(img.resource.h ===  90, "IMAGE SVG RESOURCE >> image.resrouce.h is not correct when queried"));
      },
      function(o) {//rejected
        results.push(assert(false, "image load failed : "+o));
      }).catch(function(o) {
          results.push(assert(false, "image load failed : "+o));
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testImageResourceFromDataURI_SVG");
      });
    }); // return Promise()
  },// test function()

  testImageFromDataURI_BASE64: function()
  {
    console.log("Running testImageFromDataURI_BASE64");
    return new Promise(function(resolve, reject)
    {
      var results = [];

      scene.create({t: "image", url: base64data, parent: container}).ready
      .then(function(o)
      {
        console.log("IMAGE 64 >> url: " + o.url)

        var ans = "md5sum/png;base64,8097B693B49E144DB3508E099C0C07EB"

        results.push(assert(o.url        === ans, "IMAGE 64 >> image url is not correct when queried"));
        results.push(assert(o.w          ===  -1, "IMAGE 64 >> image.w is not correct when queried"));
        results.push(assert(o.h          ===  -1, "IMAGE 64 >> image.w is not correct when queried"));
        results.push(assert(o.resource.w ===  64, "IMAGE 64 >> image.resource.w is not correct when queried"));
        results.push(assert(o.resource.h ===  64, "IMAGE 64 >> image.resource.h is not correct when queried"));
      },
      function(o) {//rejected
        results.push(assert(false, "image load failed : "+o));
      }).catch(function(o) {
          results.push(assert(false, "image load failed : "+o));
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testImageFromDataURI_BASE64");
      });
    }); // return Promise()
  },// test function()

  testImageResourceFromDataURI_BASE64: function()
  {
    console.log("Running testImageResourceFromDataURI_BASE64");
    return new Promise(function(resolve, reject)
    {
      var results = [];

      var res = scene.create({t: "imageResource", url: base64data });
      var img = scene.create({t: "image", resource: res, parent: container });

      Promise.all([res.ready, img.ready]).then(function(o)
      {
        console.log("IMAGE 64 RESOURCE >> url: " + img.url)
        var ans = "md5sum/png;base64,8097B693B49E144DB3508E099C0C07EB"

        results.push(assert(img.url        === ans, "IMAGE 64 RESOURCE >> image url is not correct when queried"));
        results.push(assert(img.w          ===  -1, "IMAGE 64 RESOURCE >> image.w is not correct when queried"));
        results.push(assert(img.h          ===  -1, "IMAGE 64 RESOURCE >> image.w is not correct when queried"));
        results.push(assert(img.resource.w ===  64, "IMAGE 64 RESOURCE >> image.resrouce.w is not correct when queried"));
        results.push(assert(img.resource.h ===  64, "IMAGE 64 RESOURCE >> image.resrouce.h is not correct when queried"));
      },
      function(o) {//rejected
        results.push(assert(false, "image load failed : "+o));
      }).catch(function(o) {
          results.push(assert(false, "image load failed : "+o));
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testImageResourceFromDataURI_BASE64");
      });
    }); // return Promise()
  },// test function()

}
module.exports.tests = tests;
module.exports.beforeStart = beforeStart;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import for test_pxImageResource64svg.js failed: " + err)
});
