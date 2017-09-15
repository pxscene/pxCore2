var through = require('through2');
var gutil = require('gulp-util');
var read = require('read-file');
var PluginError = gutil.PluginError;

const PLUGIN_NAME = 'gulp-append-prepend';

function filesGetContents(filepaths){
    if (!(filepaths instanceof Array)) {
        filepaths = [filepaths];
    }

    var filesContents = [];
    for(i = 0; i < filepaths.length; i++){
        filesContents.push(read.sync(filepaths[i], 'utf8'));
    }
    return filesContents;
}

function insert(texts, separator, type) {
    if(!texts){
        throw new PluginError(PLUGIN_NAME, 'Missing text or path !');
    }

    if (!(texts instanceof Array)) {
        texts = [texts];
    }

    if (type != "append" && type != "prepend") {
        throw new PluginError(PLUGIN_NAME, 'Missing type !');
    }

    if (!separator) {
        separator = "\n";
    }

    var buffers = [];
    for (i = 0; i < texts.length; i++) {
        if (type == "prepend") {
            buffers.push(new Buffer(texts[i].trim()+separator));
        }else if(type == "append") {
            buffers.push(new Buffer(separator+texts[i].trim()));
        }
    }

    var stream = through.obj(function(file, enc, cb) {
        if (file.isStream()) {
            this.emit('error', new PluginError(PLUGIN_NAME, 'Streams are not supported!'));
            return cb();
        }

        if (file.isBuffer()) {
            var concat = [];
            if (type == "append") {
                concat.push(file.contents);
            }
            for(i = 0; i < buffers.length; i++){
                concat.push(buffers[i]);
            }
            if (type == "prepend") {
                concat.push(file.contents);
            }

            file.contents = Buffer.concat(concat);
        }

        this.push(file);
        cb();
    });

    return stream;
}

module.exports.appendFile = function(filepath, separator) {
    return insert(filesGetContents(filepath), separator, "append");
};

module.exports.prependFile = function(filepath, separator) {
    return insert(filesGetContents(filepath), separator, "prepend");
};

module.exports.appendText = function(text, separator) {
    return insert(text, separator, "append");
};

module.exports.prependText = function(text, separator) {
    return insert(text, separator, "prepend");
};