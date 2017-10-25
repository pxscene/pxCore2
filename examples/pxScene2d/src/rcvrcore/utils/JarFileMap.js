"use strict";

function JarFileMap() {
  this.map = {};
}

JarFileMap.prototype.addArchive = function(xModuleName, fileArchive) {
  this.map[xModuleName] = fileArchive;
};

JarFileMap.prototype.getArchiveFileAsync = function(xModuleName, filePath) {
  var theFileArchive = this.map[xModuleName];
  if (theFileArchive && theFileArchive.hasFileContents(filePath)) {
    return new Promise(function (resolve, reject) {
      resolve(theFileArchive.getFileContents(filePath));
    });
  }
  return null;
};

JarFileMap.prototype.getArchiveFile = function(xModuleName, filePath) {
  var theFileArchive = this.map[xModuleName];
  if (theFileArchive && theFileArchive.hasFileContents(filePath)) {
    return theFileArchive.getFileContents(filePath);
  }
  return null;
};

module.exports = JarFileMap;