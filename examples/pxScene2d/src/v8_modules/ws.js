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


const CONNECTING = 0;
const OPEN = 1;
const CLOSING = 2;
const CLOSED = 3;


/**
 * websocket emit class
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

/**
 * Create a new websocket instance
 * @param {*} address  the websocket address
 * @param {*} protocols  the websocket protocols
 * @param {*} options  the websocket options  {timeout: , headers}
 */
function WebSocket(address, protocols, options) {

    var uri = address;
    this.readyState = CONNECTING;
    this.emit = new WBEmit();

    if (protocols) {
        uri = protocols + '://' + uri;
    }

    options = options || {};
    options.timeout = options.timeout || 60 * 1000; // 60 seconds
    options.headers = options.headers || {};

    var params = {
        uri: uri,
        headers: options.headers,
        timeoutMs: options.timeout,
    };

    this._instance = webscoketGet(params);

    // next tick
    setTimeout(() => {
        this._instance.connect();
    });

    this._instance.on('open', (...args) => {
        this.readyState = OPEN;
        this.emit.$emit('open', ...args)
    })

    this._instance.on('close', (...args) => {
        this.readyState = CLOSED;
        this.emit.$emit('close', ...args)
    })

    this._instance.on('message', (...args) => this.emit.$emit('message', ...args));
    this._instance.on('error', (...args) => this.emit.$emit('error', ...args));
}

/**
 * when websocket event happened
 * @param name the event name {open,error,message,close}
 * @param fn the function
 */
WebSocket.prototype.on = function(name, fn) {
    this.emit.$on(name, fn);
}

/**
 * close websocket
 */
WebSocket.prototype.close = function() {
    this.readyState = CLOSING;
    this._instance.close();
}

/**
 * send data
 * @param data the string data
 */
WebSocket.prototype.send = function(data) {
    this._instance.send(data);
}

/**
 * remove event listener
 * @param name the event name
 * @param fn the function
 */
WebSocket.prototype.removeListener = function(name, fn) {
    this.emit.$off(name, fn);
}

/**
 * remove all event listeners
 */
WebSocket.prototype.removeAllListeners = function(name) {
    this.emit.$off(name);
}

/**
 * close websocket
 */
WebSocket.prototype.closeimmediate = function() {
    this.close();
}

module.exports = WebSocket;