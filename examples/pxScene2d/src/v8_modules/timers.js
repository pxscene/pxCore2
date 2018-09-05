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

function setTimeout(cb, delay, a1, a2, a3) {
    var tid = uv_timer_new();
    uv_timer_start(tid, delay, 0, function () { cb(a1, a2, a3); });
    return tid;
}

function clearTimeout(tid) {
    uv_timer_stop(tid);
}

function setInterval(cb, delay, a1, a2, a3) {
    var tid = uv_timer_new();
    uv_timer_start(tid, delay, delay, function () { cb(a1, a2, a3); });
    return tid;
}

function clearInterval(tid) {
    uv_timer_stop(tid);
}

module.exports = {
    setTimeout: setTimeout,
    clearTimeout: clearTimeout,
    setInterval: setInterval,
    clearInterval: clearInterval,
}
