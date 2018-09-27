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

"use strict";

var fs = require('fs');
var url = require('url');
var http = require('http');
var FileArchive = require('rcvrcore/utils/FileArchive');
var SceneModuleManifest = require('rcvrcore/SceneModuleManifest');
//var loadFile = require('rcvrcore/utils/FileUtils').loadFile;
var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('SceneModuleLoader');


function SceneModuleLoader() {
  this.fileArchive = null;
  this.manifest = null;
  this.loadedJarFile = false;
  this.defaultManifest = false;
}

SceneModuleLoader.prototype.loadScenePackage = function(scene, fileSpec) {

  fileSpec.fileUri = fileSpec.fileUri.replace('%20', '\ '); // replace HTML escaped spaces with C/C++ escaping

  var filePath = fileSpec.fileUri;//decodeURI(fileSpec.fileUri);

  var _this = this;
  return new Promise(function (resolve, reject) {
    scene.loadArchive(fileSpec.fileUri)
      .ready.then(function(a) {
          if (a.loadStatus.httpStatusCode && a.loadStatus.httpStatusCode != 200)
          {
            console.error("http status is not 200 rejecting");
            reject(a.loadStatus);
          }
          else
          {
        log.message(4, "loadScenePackage: loadArchive succeeded for ("+filePath+").");

        _this.fileArchive = new FileArchive(filePath, a);
        log.message(4, "Number of files: " + a.fileNames.length);
        _this.loadedJarFile = (a.fileNames.length > 1);
        log.message(10, "LoadedFromJarFile= " + _this.loadedJarFile);
        if(isFileInList("package.json", a.fileNames)) {
          log.message(10, "Has package.json");
          _this.fileArchive.addFile('package.json', a.getFileAsString('package.json'));
          _this.defaultManifest = false;
        } else {
          log.message(10, "Doesn't have package.json");
          _this.fileArchive.addFile('package.json', "{ \"main\" : \"" + filePath + "\" }");
          _this.defaultManifest = true;
        }

        _this.manifest = new SceneModuleManifest();
        _this.manifest.loadFromJSON(_this.fileArchive.getFileContents('package.json'));
        resolve();
          }

      }, function(a){
        console.error("loadScenePackage: loadArchive failed for (",filePath,").");
        reject(a.loadStatus);
      });
  });
};

function isFileInList(fileName, list) {
  for(var k = 0; k < list.length; ++k) {
    if( list[k] === fileName ) {
      return true;
    }
  }

  return false;
}

SceneModuleLoader.prototype.loadScenePackage0 = function(scene, fileSpec) {
  log.message(4, "loadScenePackage - fileSpec.fileUri=" + fileSpec.fileUri);
  var _this = this;
  var filePath = fileSpec.fileUri;

  return new Promise(function (resolve, reject) {
    if (filePath.substring(0, 4) === "http" || filePath.substring(0,5) == "https") {
      _this.loadRemoteFile(filePath).then(function dataAvailable(data) {
        _this.processFileData(filePath, data);
        if(_this.processFileArchive() === 0) {
          resolve();
        } else {
          reject();
        }
      }).catch(function onError(err) {
        console.error("SceneModuleLoader#loadScenePackage: error on http get of " + filePath + ": Error=" + err);
        reject(err);
      });
    } else {
      if( filePath.substring(0,5) === 'file:' ) {
        filePath = filePath.substring(5);
      }
      _this.loadLocalFile(filePath).then(function dataAvailable(data) {
        _this.processFileData(filePath, data);
        if(_this.processFileArchive() === 0) {
          resolve();
        } else {
          reject();
        }
      }).catch(function onError(err) {
        console.error("SceneModuleLoader#loadScenePackage: error on file get: " + err);
        reject(err);
      });
    }
  });
};

SceneModuleLoader.prototype.processFileData = function(filePath, data) {
  this.fileArchive = new FileArchive(filePath);
  if( data[0] === 80 && data[1] === 75 && data[2] === 3 && data[3] === 4) {
    this.fileArchive.loadFromJarData(data);
  } else {
    this.fileArchive.addFile('package.json', "{ \"main\" : \"" + filePath + "\" }");
    this.fileArchive.addFile(filePath, data);
    this.defaultManifest = true;
  }
};


SceneModuleLoader.prototype.loadRemoteFile = function(filePath) {
  var _this = this;
  return new Promise(function (resolve, reject) {
    log.message(3, "loadRemoteFile: " + filePath);
    var req = http.get(url.parse(filePath), function (res) {
      if (res.statusCode !== 200) {
        console.error(res.statusCode);
        reject("http get error. statusCode=" + res.statusCode);
      }
      var data = [], dataLen = 0;

      // don't set the encoding, it will break everything !
      res.on("data", function (chunk) {
        data.push(chunk);
        dataLen += chunk.length;
      });

      res.on("end", function () {
        var buf = new Buffer(dataLen);
        for (var i=0,len=data.length,pos=0; i<len; i++) {
          data[i].copy(buf, pos);
          pos += data[i].length;
        }

        resolve(buf);
      });
    });

    req.on("error", function(err){
      reject(err);
    });
    req.setTimeout(10000, function httpGetTimeout() {
      reject("Timeout on http.get(" + filePath +")");
    });
  });
};

SceneModuleLoader.prototype.loadLocalFile = function(filePath) {
  var _this = this;

  return new Promise( function(resolve, reject) {
    fs.readFile(filePath, function (err, data) {
      if (err) {
        reject(err);
      }
      resolve(data);
    });
  });
};


SceneModuleLoader.prototype.processFileArchive = function() {
  if( this.fileArchive.getFileCount() >= 2 ) {
    var packageFileContents = this.fileArchive.getFileContents("package.json");
    if( packageFileContents !== null ) {
      this.manifest = new SceneModuleManifest();
      this.manifest.loadFromJSON(packageFileContents);
      this.fileArchive.removeFile("package.json");
      return 0;
    } else {
      console.error("No package manifest");
      return -1;
    }
  } else {
    console.error("Expected archive to have at least two files.");
    return -1;
  }
};

SceneModuleLoader.prototype.getFileArchive = function() {
  return this.fileArchive;
};

SceneModuleLoader.prototype.getManifest = function() {
  return this.manifest;
};

SceneModuleLoader.prototype.isDefaultManifest = function() {
  return this.defaultManifest;
};

SceneModuleLoader.prototype.jarFileWasLoaded = function() {
  return this.loadedJarFile;
};

module.exports = SceneModuleLoader;

function hasExtension(filePath, extension) {
  var idx = filePath.lastIndexOf(extension);
  var rtnValue = (idx !== -1) && ((idx + extension.length) === filePath.length);
  return rtnValue;
}
