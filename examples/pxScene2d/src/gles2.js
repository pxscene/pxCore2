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

var init = function(options) {
    options = options || {};

    var width = (typeof options.width == "number" ? options.width : 1280);
    var height = (typeof options.height == "number" ? options.height : 720);
    var fullscreen = !!options.fullscreen;
    var title = options.title || "";

    //gles2.init(width, height, fullscreen, title);

    return require('./webgl').instance;
};

module.exports = {
    init: init
};


