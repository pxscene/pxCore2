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

/**
 * convert multiple messages to single string
 * @param  {...any} args the messages
 */
function stringify(...args) {
    return Array.prototype.slice.apply(arguments).join(' ');
}

function trace(...args) {
    print("[LOG TRACE] " + stringify(...args));
}

function warn(...args) {
    print("[LOG WARN] " + stringify(...args));
}

function error(...args) {
    print("[LOG ERROR] " + stringify(...args));
}

function log(...args) {
    print("[LOG LOG] " + stringify(...args));
}

function info(...args) {
    print("[LOG INFO] " + stringify(...args));
}

module.exports = {
    trace: trace,
    warn: warn,
    error: error,
    log: log,
    info: info,
    time:info,
    timeEnd: info,
};
