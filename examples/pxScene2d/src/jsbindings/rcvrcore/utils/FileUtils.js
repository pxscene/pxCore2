"use strict";

var fs = require("fs");
var http = require("http");
var url = require("url");

var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('FileUtils');

/**
 * Load a file from either the filesystem or webservice
 * @param fileUri the full path of the file
 * @returns  a promise that will be fulfilled if the file is found, else it is rejected.
 */
function loadFile(fileUri) {
  return new Promise(function(resolve, reject) {
    var code = [];
    if (fileUri.substring(0, 4) == "http") {
      var options = url.parse(fileUri);
      if (fileUri.substring(0, 5) == "https") {
        throw 'not supported'
      }
      else {
        var req = http.get(options, function (res) {
          res.on('data', function (data) {
            code += data;
          });
          res.on('end', function () {
            if( res.statusCode == 200 ) {
              log.message(3, "Got file[" + fileUri + "] from web service");
              resolve(code);
            } else {
              log.error("FAILED to read file[" + fileUri + "] from web service");
              reject(res.statusCode);
            }
          });
        });
        req.on('error', function (err) {
          log.error("FAILED to read file[" + fileUri + "] from web service");
          reject(err);
        });
      }
    }
    else {
      var infile = fs.createReadStream(fileUri);
      infile.on('data', function (data) {
        code += data;
      });
      infile.on('end', function () {
        log.message(3, "Got file[" + fileUri + "] from file system");
        resolve(code);
      });
      infile.on('error', function (err) {
        log.error("FAILED to read file[" + fileUri + "] from file system");
        reject(err);
      });
    }
  });
}

function hasExtension(filePath, extension) {
  var idx = filePath.lastIndexOf(extension);
  return( (idx !== -1) && ((idx + extension.length) === filePath.length) );
}

module.exports = {
  loadFile: loadFile,
  hasExtension: hasExtension
};
