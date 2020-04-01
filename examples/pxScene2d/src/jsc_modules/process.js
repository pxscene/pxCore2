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

module.exports = {
    _tickCallback: function() {},
    exit: function() { _exit(); },
    binding: function() { throw new Error("process.binding is not supported"); },
    hrtime: function() { return _hrtime(); },
    memoryUsage: function() { return 0; },
    platform: _platform(),
    cwd: function() {
        // TODO
        return ""
    },
    env: new Proxy({}, {
        set: function(obj, prop, value) {
            throw new Error("Not supported");
        },
        get: function(obj, prop) {
            return null;
        }
    }),
};
