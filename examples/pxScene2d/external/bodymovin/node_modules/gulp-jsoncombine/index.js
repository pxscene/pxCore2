"use strict";
var through = require('through');
var path = require('path');
var gutil = require('gulp-util');
var PluginError = gutil.PluginError;
var File = gutil.File;

module.exports = function (fileName, converter) {
  if (!fileName) {
    throw new PluginError('gulp-jsoncombine', 'Missing fileName option for gulp-jsoncombine');
  }
  if (!converter) {
    throw new PluginError('gulp-jsoncombine', 'Missing converter option for gulp-jsoncombine');
  }

  var data = {};
  var firstFile = null;
  //We keep track of when we should skip the conversion for error cases
  var skipConversion = false;

  function bufferContents(file) {
    if (!firstFile) {
      firstFile = file;
    }

    if (file.isNull()) {
      return; // ignore
    }
    if (file.isStream()) {
	  skipConversion = true;
      return this.emit('error', new PluginError('gulp-jsoncombine', 'Streaming not supported'));
    }
    try {
      data[file.relative.substr(0,file.relative.length-5)] = JSON.parse(file.contents.toString());
    } catch (err) {
      skipConversion = true;
      return this.emit('error',
		  new PluginError('gulp-jsoncombine', 'Error parsing JSON: ' + err + ', file: ' + file.path.slice(file.base.length)));
    }
  }

  function endStream() {
    if (firstFile && !skipConversion) {
      var joinedPath = path.join(firstFile.base, fileName);

	  try {
	    var joinedFile = new File({
          cwd: firstFile.cwd,
          base: firstFile.base,
          path: joinedPath,
          contents: converter(data)
        });
		this.emit('data', joinedFile);
	  }	catch (e) {
		return this.emit('error', new PluginError('gulp-jsoncombine', e, { showStack: true }));
	  }
    }
    this.emit('end');
  }

  return through(bufferContents, endStream);
};
