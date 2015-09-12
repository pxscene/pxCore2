"use strict";

var fs = require('fs');
var url = require('url');
var http = require('http');
var FileArchive = require('rcvrcore/utils/FileArchive');
var SceneModuleManifest = require('rcvrcore/SceneModuleManifest');
var loadFile = require('rcvrcore/utils/FileUtils').loadFile;
var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('SceneModuleLoader');


function SceneModuleLoader() {
  this.fileArchive = null;
  this.manifest = null;
  this.loadedJarFile = false;
  this.defaultManifest = false;
}

SceneModuleLoader.prototype.loadScenePackage = function(fileSpec) {
  var _this = this;
  var filePath = fileSpec.fileUri;

  return new Promise(function (resolve, reject) {
    if (filePath.substring(0, 4) === "http") {
      _this.loadRemoteFile(filePath).then(function dataAvailable(data) {
        _this.processFileData(filePath, data);
        if(_this.processFileArchive() == 0) {
          resolve();
        } else {
          reject();
        }
      }).catch(function onError(err) {
        console.error("SceneModuleLoader#loadScenePackage: error on http get of " + filePath + ": Error=" + err);
        reject(err);
      });
    } else {
      _this.loadLocalFile(filePath).then(function dataAvailable(data) {
        _this.processFileData(filePath, data);
        if(_this.processFileArchive() == 0) {
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
}


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

}

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
    if( packageFileContents != null ) {
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

}

SceneModuleLoader.prototype.getFileArchive = function() {
  return this.fileArchive;
}

SceneModuleLoader.prototype.getManifest = function() {
  return this.manifest;
}

SceneModuleLoader.prototype.isDefaultManifest = function() {
  return this.defaultManifest;
}

SceneModuleLoader.prototype.jarFileWasLoaded = function() {
  return this.loadedJarFile;
}

module.exports = SceneModuleLoader;

function hasExtension(filePath, extension) {
  var idx = filePath.lastIndexOf(extension);
  var rtnValue = (idx !== -1) && ((idx + extension.length) === filePath.length);
  return rtnValue;
}
