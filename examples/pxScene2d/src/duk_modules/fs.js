"use strict";

function readFile(filePath, cb) {
    var ares = uv.fs_access(filePath, 'r');
    if (ares < 0) {
        cb(ares, "");
        return;
    }
    var fd = uv.fs_open(filePath, 'r', 420);
    var stat = uv.fs_stat(filePath);
    var res = uv.fs_read(fd, stat.size, 0);
    uv.fs_close(fd);
    cb(0, res);
}

function readFileSync(filePath) {
    var ares = uv.fs_access(filePath, 'r');
    if (ares < 0) {
        return "";
    }
    var fd = uv.fs_open(filePath, 'r', 420);
    var stat = uv.fs_stat(filePath);
    var res = uv.fs_read(fd, stat.size, 0);
    uv.fs_close(fd);
    return res;
}

module.exports = {
    'readFile': readFile,
    'readFileSync': readFileSync,
}
