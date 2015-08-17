"use strict";

function SceneModuleManifest() {
  this.jsonData = {};
  this.main = "main.js";
}

SceneModuleManifest.prototype.loadFromJSON = function(fileContents) {
  this.jsonData = JSON.parse(fileContents);
  this.main = this.jsonData['main'];
};

SceneModuleManifest.prototype.getMain = function() {
  return this.main;
};

SceneModuleManifest.prototype.getNamespaceImportPaths = function() {
  if( typeof this.jsonData.importPaths === 'undefined' || typeof this.jsonData.importPaths.namespaces === 'undefined') {
    return {};
  }
  return this.jsonData.importPaths.namespaces;
};

module.exports = SceneModuleManifest;