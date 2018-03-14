/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

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

module.exports = {
    'readFile': readFile,
}
