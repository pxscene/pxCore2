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


/**
 * emit class
 */
function WBEmit() {}
WBEmit.prototype.$emit = function(name) {
    var args = Array.prototype.slice.call(arguments, 1);
    if (this._events && this._events[name])
        this._events[name].forEach(function(cb) {
            cb.apply(this, args)
        }.bind(this));
    return this;
};
WBEmit.prototype.$off = function(name, cb) {
    if (!this._events) return this;
    if (!cb) delete this._events[name];
    if (this._events[name]) this._events[name] = this._events[name].filter(function(i) {
        return i != cb
    });
    return this;
};
WBEmit.prototype.$on = function(name, cb) {
    if (!this._events) this._events = {};
    if (!this._events[name]) this._events[name] = [];
    this._events[name].push(cb);
    return cb;
};

module.exports = WBEmit;
