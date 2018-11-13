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
 * Native http manager used to save http instance (prevent destroyed before response)
 * when response reached, remove the reference in instance, let v8 destory the c++ object
 */
function NativeHttpManager() {}
NativeHttpManager.instances = [];

NativeHttpManager.add = function(httpInstance) {
    NativeHttpManager.instances.push(httpInstance);
}
NativeHttpManager.remove = function(httpInstance) {
    if (NativeHttpManager.instances.length > 0) {
        var index = NativeHttpManager.instances.indexOf(httpInstance);
        if (index != -1) {
            delete NativeHttpManager.instances[index];
            NativeHttpManager.instances.splice(index, 1);
        }
    }
}
NativeHttpManager.get = function(...args) {
    const req = new NativeHttp(...args);
    req.end();
    return req;
}
NativeHttpManager.request = function(...args) {
    return new NativeHttp(...args);
}


/**
 * Wraper native http request to js http Object
 * @param {*} url the url/options
 * @param {*} cb the callback
 */
function NativeHttp(url, cb) {
    this._httpInstance = httpGet(url, res => {
        cb(res);
        NativeHttpManager.remove(this);
    });
    NativeHttpManager.add(this);
}

NativeHttp.prototype.on = function(...args) {
    this._httpInstance.on(...args);
    return this;
};

NativeHttp.prototype.abort = function() {
    this._httpInstance.abort();
}

NativeHttp.prototype.end = function() {
    this._httpInstance.end();
}
NativeHttp.prototype.write = function(...args) {
    this._httpInstance.write(...args);
}
NativeHttp.prototype.setTimeout = function(...args) {
    this._httpInstance.setTimeout.apply(...args);
}
NativeHttp.prototype.setHeader = function(...args) {
    this._httpInstance.setHeader(...args);
}


module.exports = {
    'get': NativeHttpManager.get,
    'request': NativeHttpManager.request,
}