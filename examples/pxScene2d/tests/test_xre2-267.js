"use strict";
px.import({scene:"px:scene.1.js",assert:"../test-run/assert.js", net:"net", ws: "ws"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var net = imports.net;
var ws = imports.ws;

var basePackageUri = px.getPackageBaseFilePath();

module.exports.beforeStart = function() {
  console.log("test_xre2-267 beforeStart()!");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {
  testevents: function() {
    return new Promise(function(resolve, reject) {
      var page = scene.create({t:"scene", parent:root, url:"test_events.js"});
      page.ready.then(function()  {
        console.log("events import succeeded, need to check .....");
        var results = [];
        results.push("FAILURE");
        resolve(results);
  }, function() {
        var results = [];
        results.push("SUCCESS");
        resolve(results);
     });
  });
  },

  testfs: function() {
    return new Promise(function(resolve, reject) {
      var page = scene.create({t:"scene", parent:root, url:"test_fs.js"});
      page.ready.then(function()  {
        console.log("fs import succeeded, need to check .....");
        var results = [];
        results.push("FAILURE");
        resolve(results);
  }, function() {
        var results = [];
        results.push("SUCCESS");
        resolve(results);
     });
  });
  },

  testos: function() {
    return new Promise(function(resolve, reject) {
      var page = scene.create({t:"scene", parent:root, url:"test_os.js"});
      page.ready.then(function()  {
        console.log("os import succeeded, need to check .....");
        var results = [];
        results.push("FAILURE");
        resolve(results);
  }, function() {
        var results = [];
        results.push("SUCCESS");
        resolve(results);
     });
  });
  },

  testnet: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      results.push (assert(undefined !== net.createConnection ,"net createConnection interface not present "));
      results.push (assert(undefined !== net.Socket ,"net Socket interface not present "));
      results.push (assert(undefined !== net.isIP ,"net isIP interface not present "));
      results.push (assert(undefined !== net.isIPv4 ,"net isIPv4 interface not present "));
      results.push (assert(undefined !== net.isIPv6 ,"net isIPv6 interface not present "));
      resolve(results);
  });
  },
}

module.exports.tests = tests;

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_xre2-247.js failed: " + err)
});
