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

SceneModuleManifest.prototype.getConfigImport = function() {
  if( typeof this.jsonData.configImport === 'undefined' ) {
    return {};
  }
  return this.jsonData.configImport;
};

module.exports = SceneModuleManifest;