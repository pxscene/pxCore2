"use strict";

var FileArchive = require('rcvrcore/utils/FileArchive');

var SceneModuleManifest = require('rcvrcore/SceneModuleManifest');
var loadFile = require('rcvrcore/utils/FileUtils').loadFile;

function SceneModuleLoader() {
  this.fileArchive = null;
  this.manifest = null;
  this.defaultManifest = false;
}

SceneModuleLoader.prototype.loadScenePackage = function(scenePackage) {
  var _this = this;

  return new Promise(function (resolve, reject) {
    if (scenePackage == null || scenePackage === 'undefined' || scenePackage.fileUri === null
      || scenePackage.fileUri === 'undefined') {
      reject("loadScenePackage: package spec doesn't contain fileUri!");
    }
    var hasManifest = false;
    if (scenePackage.manifest != null && scenePackage.manifest != 'undefined') {
      hasManifest = true;
    }

    _this.fileArchive = new FileArchive('SceneModule');

    if (hasExtension(scenePackage.fileUri, '.tar') || hasExtension(scenePackage.fileUri, '.tar.gz')) {
      _this.fileArchive.loadFromTarFile(scenePackage.fileUri).then(function () {
        (_this.processFileArchive() == 0)? resolve() : reject();
      }).catch(function (error) {
        reject("Error loading tar file: " + error)
      });
    } else if (hasManifest && hasExtension(scenePackage.fileUri, '.js')) {
      loadFile(scenePackage.manifest).then(function (fileContents) {
        _this.fileArchive.addFile('package.json', fileContents);
        return loadFile(scenePackage.fileUri);
      }).then(function (fileContents) {
        _this.fileArchive.addFile(scenePackage.fileUri, fileContents);
        (_this.processFileArchive() == 0)? resolve() : reject();
      }).catch(function (error) {
        reject("Json and JavaScript loading Error: " + error)
      });

    } else if (hasExtension(scenePackage.fileUri, '.js')) {
      loadFile(scenePackage.fileUri).then(function (fileContents) {
        _this.fileArchive.addFile('package.json', "{ \"main\" : \"" + scenePackage.fileUri + "\" }");
        _this.fileArchive.addFile(scenePackage.fileUri, fileContents);
        _this.defaultManifest = true;

        if(_this.processFileArchive() == 0) {
          resolve();
        } else {
          reject();
        }

      }).catch(function(){
        reject("Did not find file: " + scenePackage.fileUri);
      });
    } else {
      reject("Unknown file extension: " + scenePackage.fileUri);
    }
  });

}

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

module.exports = SceneModuleLoader;

function hasExtension(filePath, extension) {
  var idx = filePath.lastIndexOf(extension);
  var rtnValue = (idx !== -1) && ((idx + extension.length) === filePath.length);
  return rtnValue;
}
