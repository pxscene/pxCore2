"use strict";

var fs = require('fs');
var url = require('url');
var http = require('http');
var fs = require("fs");
var JSZip = require("jszip");

var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('FileUtils');

var tarDirectory = {};

function FileArchive(name) {
  this.name = name;
  this.numEntries = 0;
  this.directory = {};
  this.jar = null;
}


FileArchive.prototype.loadFromJarFile = function(filePath) {
  var _this = this;

  if (filePath.substring(0, 4) === "http") {
    return _this.loadRemoteJarFile(filePath);
  } else {
    return _this.loadLocalJarFile(filePath);
  }
}

FileArchive.prototype.getFileCount = function() {
  return this.numEntries;
}

FileArchive.prototype.directoryIterator = function(callback) {
  var directoryMap = this.directory;
  if (directoryMap != null && directoryMap !== 'undefined' ) {
    for (var key in directoryMap) {
      if (this.hasOwnProperty(key)) {
        callback(key, directoryMap[key]);
      }
    }
  }
}

FileArchive.prototype.removeFile = function(filename) {
  if( this.directory.hasOwnProperty(filename) ) {
    this.directory[filename] = null;
    delete this.directory[filename];
    --this.numEntries;
    return true;
  } else {
    return false;
  }

}

FileArchive.prototype.getFileContents = function(filename) {
  if( this.directory.hasOwnProperty(filename) ) {
    return this.directory[filename];
  } else {
    return null;
  }
}

FileArchive.prototype.addFile = function(filename, contents) {
  var wasNewFile = true;
  if( this.directory.hasOwnProperty(filename) ) {
    wasNewFile = false;
  }

  this.directory[filename] = contents;
  ++this.numEntries;
  return wasNewFile;
}


FileArchive.prototype.loadRemoteJarFile = function(filePath) {
  var _this = this;
  return new Promise(function (resolve, reject) {
    var req = http.get(url.parse(filePath), function (res) {
      if (res.statusCode !== 200) {
        console.log(res.statusCode);
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

        var jar = new JSZip(buf);
        _this.processJar(jar);
        resolve(jar);
      });
    });

    req.on("error", function(err){
      reject(err);
    });
  });

}

FileArchive.prototype.loadLocalJarFile = function(jarFilePath) {
  var _this = this;

  return new Promise( function(resolve, reject) {
    fs.readFile(jarFilePath, function (err, data) {
      if (err) {
        reject(err);
      }
      var jar = new JSZip(data);
      _this.processJar(jar);
      resolve(jar);
    });
  });
};

FileArchive.prototype.loadFromJarData = function(dataBuf) {
  var jar = new JSZip(dataBuf);
  this.processJar(jar);
}

FileArchive.prototype.processJar = function(jar) {
  for(var file in jar.files) {
    var fileEntry = jar.files[file];
    if( fileEntry.options.dir === true ) {
      continue;
    }
    this.addArchiveEntry(file, fileEntry.asText());
  }
}


FileArchive.prototype.addArchiveEntry = function(filename, data) {
  ++this.numEntries;
  this.directory[filename] = data;
}

FileArchive.prototype.hasFileContents = function(filename) {
  return this.directory.hasOwnProperty(filename);
}

function hasExtension(filePath, extension) {
  var idx = filePath.lastIndexOf(extension);
  var rtnValue = (idx !== -1) && ((idx + extension.length) === filePath.length);
  return rtnValue;
}


module.exports = FileArchive;
