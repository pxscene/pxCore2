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

var FileArchive = require('rcvrcore/utils/FileArchive');
var SceneModuleManifest = require('rcvrcore/SceneModuleManifest');
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
