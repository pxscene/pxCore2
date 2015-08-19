"use strict";

var fs = require('fs');
var url = require('url');
var http = require('http');
var tar = require('tar-stream');
var concat = require('concat-stream');
var zlib = require('zlib');

var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('FileUtils');

var tarDirectory = {};

function FileArchive(name) {
  this.name = name;
  this.numEntries = 0;
  this.directory = {};
}


FileArchive.prototype.loadFromTarFile = function(filePath) {
  var _this = this;

  if (filePath.substring(0, 4) === "http") {
    return _this.loadRemoteTarFile(filePath);
  } else {
    return _this.loadLocalTarFile(filePath);
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


FileArchive.prototype.loadRemoteTarFile = function(filePath) {
  var _this = this;
  return new Promise(function (resolve, reject) {

    if (hasExtension(filePath, "tar.gz")) {
      httpGetGzipped(filePath, function (responseData) {
        var rs = tar.pack();
        rs.push(responseData);
        rs.finalize();
        _this.processTarStream(rs, function() {
          resolve();
        });
      }, function (errorCode) {;
        console.error("Error getting file " + filePath + ", error=" + errorCode);
        reject();
      });
    } else {
      httpTarGet(filePath, function (responseData) {
        //var rs = tar.pack();
        //rs.finalize();
        _this.processTarStream(responseData, function() {
          resolve();
        });
      }, function (errorCode) {
        console.error("Error getting file " + filePath + ", error=" + errorCode);
        reject();
      });
    }

  });

}

FileArchive.prototype.loadLocalTarFile = function(tarStream) {
  var _this = this;

  return new Promise( function(resolve, reject) {
    if (hasExtension(tarStream, "tar.gz")) {
      fileGetGzipped(tarStream, function(responseData) {
          var rs = tar.pack();
          rs.push(responseData);
          rs.finalize();
          _this.processTarStream(rs, function() {
            resolve();
          });
        },
        function(error) {
          console.error("Error during unzipping tar file: " + error);
          reject(error);
        }
      );

      return;
    }
    var extract = tar.extract();

    var onfile1 = function(header, stream, callback) {
      stream.pipe(concat(function(data) {
        _this.addArchiveEntry(header.name, data);
        callback()
      }))
      extract.once('entry', onfile1);
    }

    extract.once('entry', onfile1)

    extract.on('finish', function() {
      resolve();
    })

    extract.end(fs.readFileSync(tarStream));
  });

}

FileArchive.prototype.processTarStream = function(tarStream, onCompleteCallback) {
  var _this = this;

  var extract = tar.extract();

  var onfile1 = function(header, stream, callback) {
    //log.message(2, "On Entry Received[" + header.type + "]: filename=" + header.name + ", size=" + header.size + ", time=" + header.mtime + "!!!");
    stream.pipe(concat(function(data) {
      _this.addArchiveEntry(header.name, data);
      callback()
    }))
    extract.once('entry', onfile1);
  }

  extract.on('error', function() {
    console.error("Error:")
  });

  extract.on('abort', function() {
    console.error("Abort:")
  });

  extract.once('entry', onfile1);

  extract.on('finish', function() {
    onCompleteCallback(_this.numEntries);
  })
  tarStream.pipe(extract);

}


FileArchive.prototype.processTarStream0 = function(tarStream) {
  var _this = this;

  var extract = tar.extract();

  var onfile1 = function (header, stream, callback) {
    //log.message(2, "On Entry Received[" + header.type + "]: filename=" + header.name + ", size=" + header.size + ", time=" + header.mtime + "!!!");
    stream.pipe(concat(function (data) {
      _this.addArchiveEntry(header.name, data);
      callback()
    }))
    extract.once('entry', onfile1);
  }

  extract.once('entry', onfile1)

  extract.on('finish', function () {
    log.message(2, "Done: noEntries=" + _this.numEntries);
    //onFinishCallback(_this.numEntries);
  })
  tarStream.pipe(extract);

}


FileArchive.prototype.addArchiveEntry = function(filename, data) {
  ++this.numEntries;
  this.directory[filename] = data;
}

function httpTarGet (uri, dataCallback, failureCallback) {
  var options = url.parse(uri);
  var Readable = require('stream').Readable;
  var rs = tar.pack(); //new Readable;
  rs.setEncoding('binary');

  var req = http.get(options, function(res) {
    res.on('data',  function(data)  {
      rs.push(data);
    });
    res.on('end',   function()      {
      if( res.statusCode == 200 ) {
        rs.finalize();
        dataCallback(rs);
      } else {
        failureCallback(res.statusMessage, res);
      }
    });
  });
  req.on('error',   function(err)   {
    failureCallback(err); });
}


function httpGet(uri, dataCallback, failureCallback) {
  var options = url.parse(uri);
  var code = '';
  var req = http.get(options, function(res) {
    res.on('data',  function(data)  {
      code += data;
    });
    res.on('end',   function()      {
      if( res.statusCode == 200 ) {
        dataCallback(code);
      } else {
        failureCallback(res.statusMessage, res);
      }
    });
  });
  req.on('error',   function(err)   {
    failureCallback(err); });
}

function httpGetGzipped(url, callback,failureCallback) {
  // buffer to store the streamed decompression
  var buffer = [];

  http.get(url, function(res) {
    // pipe the response into the gunzip to decompress
    var gunzip = zlib.createGunzip();
    if( res.statusCode != 200 ) {
      failureCallback(res.statusMessage);
    }
    res.pipe(gunzip);

    gunzip.on('data', function(data) {
      // decompression chunk ready, add it to the buffer
      buffer.push(data.toString())

    }).on("end", function() {
      // response and decompression complete, join the buffer and return
      callback(buffer.join(""));

    }).on("error", function(e) {
      failureCallback(e);
    })
  }).on('error', function(e) {
    failureCallback(e)
  });
}

function fileGetGzipped(url, callback,failureCallback) {
  // buffer to store the streamed decompression
  var buffer = [];

  var gunzip = zlib.createGunzip();
  var infile = fs.createReadStream(url);
  infile.pipe(gunzip);

  gunzip.on('data', function(data) {
    // decompression chunk ready, add it to the buffer
    buffer.push(data.toString())

  }).on("end", function() {
    // response and decompression complete, join the buffer and return
    callback(buffer.join(""));

  }).on("error", function(e) {
    failureCallback(e);
  })
}



function hasExtension(filePath, extension) {
  var idx = filePath.lastIndexOf(extension);
  var rtnValue = (idx !== -1) && ((idx + extension.length) === filePath.length);
  return rtnValue;
}


module.exports = FileArchive;
