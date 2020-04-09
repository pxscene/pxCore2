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

function runInNewContext(...args) {
  return _runInNewContext(...args);
}

function runInContext(...args) {
  return _runInContext(...args);
}

function createContext(...args) {
  return _createContext(...args);
}

class Script {
  constructor(source) {
    this.source = source;
  }

  runInContext(ctx) {
    return _runInContext(this.source, ctx);
  }
}

class SourceTextModule {
  constructor(code, options) {
    this.code = code;
    this.options = options;
    this.status = this.linkingStatus = 'unlinked';
  }

  link(linker) {
    let self = this;
    self.status = 'linking';
    return Promise.resolve().then(() => self.status = self.linkingStatus = 'linked');
  }

  instantiate() {
    this.namespace = {};
  }

  evaluate() {
    let self = this;
    self.status = 'evaluating';
    return new Promise(resolve => {
      _runInContext(self.code, self.options.context);
      resolve();
    }).then(() => self.status = 'evaluated');
  }
}

function runInThisContext(...args) {
  return _runInThisContext(...args);
}

module.exports = {
  runInNewContext: runInNewContext,
  runInContext: runInContext,
  createContext: createContext,
  Script: Script,
  runInThisContext: runInThisContext,
  SourceTextModule: SourceTextModule,
};
