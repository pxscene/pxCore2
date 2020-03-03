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

const fetch = require('node-fetch');
const Headers = fetch.Headers;
const Request = fetch.Request;
const Response = fetch.Response;

const WebSocket = require('ws');

const URL = require('url').URL;
const URLSearchParams = require('url').URLSearchParams;

const location = new URL(global.__dirname);

class Event extends String {}

class EventTarget extends require('events') {
  addEventListener(type, listener) {
    this.addListener(type, listener)
  }

  removeEventListener(type, listener) {
    this.removeListener(type, listener)
  }

  dispatchEvent(event) {
    this.emit(event)
  }
}

class SparkWindow extends EventTarget {
  constructor(){
    super();
  }

  get innerWidth() {
    return sparkscene.w;
  }

  get innerHeight() {
    return sparkscene.h;
  }

  get location() {
    return location;
  }

  get localStorage() {
    return localStorage;
  }

  get clearTimeout() {
    return clearTimeout;
  }

  get setTimeout() {
    return setTimeout;
  }
}

class SparkDocument extends EventTarget {
  constructor() {
    super();
    this.head = {appendChild: () => {}};
    this.body = {appendChild: () => {}};
    this.fonts = {add: () => {}};
  }

  get location() {
    return location;
  }

  createElement(tagName) {
    if (tagName === 'style') {
      return {sheet: {insertRule: () => {}}, appendChild: () => {}}
    } else if (tagName === 'script') {
      return new SparkScript()
    } else if (tagName === 'link') {
      return {}
    }
  }

  createTextNode() {
    return {}
  }

  getElementById() {
    return null
  }
}

class XMLHttpRequest extends EventTarget {
  constructor() {
    super();
    this.readyState = 0;
  }

  open(method, URL) {
    this._method = method;
    this._URL = relative2absolute(URL);
    this.readyState = 1;
  }

  send(body) {
    let self = this;
    fetch(this._URL, {method:this._method, body:body}).then(r => {
      self.status = r.status;
      self.readyState = 4;
      self.responseText = r._bodyText.toString();
      if (self.onreadystatechange)
        self.onreadystatechange();
    });
  }
}

class FontFace {
  constructor(family, source, descriptors) {
    let m = source.match(/\((.*)\)/);
    this._url = m?m[1]:m;
  }

  load() {
    let fontResource = sparkscene.create({t: "fontResource", url: this._url});
    return fontResource.ready;
  }
}

class SparkScript {
  set onload(callback) {
    this._onload = callback;
  }

  set load(b) {
    this._load = b
  }

  set src(url) {
    url = relative2absolute(url);

    if (this._load) {
      let self = this;
      fetch(url).then(r => {
        if (r.status >= 200 && r.status <= 299) {
          vm.runInThisContext(r._bodyText.toString());
          self._onloaded()
        } else {
          console.log(`HTTP ${r.status} for '${url}'`);
        }
      })
    } else {
      this._onloaded()
    }
  }

  _onloaded() {
    let self = this;
    setImmediate(() => {
      if (self._onload)
        self._onload()
    })
  }
}

const relative2absolute = url =>
  /^\/\//.test(url) ? window.location.protocol + url :
    (/^\//.test(url) ? window.location.origin + url :
      (!/^(?:https?:)/i.test(url) ? require("url").resolve(window.location.toString(), url) :
        url));

const globalsHandler = {
  get: function(obj, prop) {
    return prop in obj ?
      obj[prop] :
      (prop in global ? global[prop] : eval(prop));
  }
};

const window = new Proxy(new SparkWindow(), globalsHandler);
const document = new Proxy(new SparkDocument(), globalsHandler);

module.exports = {
  fetch,
  Headers,
  Request,
  Response,
  WebSocket,
  URL,
  URLSearchParams,
  location,
  Event,
  EventTarget,
  window,
  document,
  XMLHttpRequest,
  FontFace
};
