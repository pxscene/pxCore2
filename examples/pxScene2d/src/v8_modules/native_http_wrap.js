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

const Emit = require('emit');
const url = require('url');

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
 * @param {*} scene current scene
 * @param {*} options the url or options
 * @param {*} cb the callback
 */
function NativeHttp(scene, options, cb) {

    this.options = options;
    this.timeoutHandler = null;
    this.responseHandler = cb;
    this.scene = scene;
    this.errorHander = null;
    this.blockedHandler = null;
    this.isTimeout = false;
    this.blocked = false;    
    this.emit = new Emit();

    if (cb) {
        this.emit.$on('response', cb);
    }

    this._httpInstance = httpGet(options, (res)=>{
        this.responseFunc(res);
    });

    NativeHttpManager.add(this);
}


NativeHttp.prototype.responseFunc = function(res) {
    if (this.timeoutHandler) {
        clearTimeout(this.timeoutHandler);
    }
    if (this.isTimeout) {
        console.log('request timeout and reponse reached, ignore this response.');
        NativeHttpManager.remove(this);
        return;
    }

    // check CORS
    var requestOrigin = this._getRequestOrigin();
    var appOrigin = this.origin();
    console.log(`appOrigin = ${appOrigin}`)
    console.log(`requestOrigin = ${requestOrigin}`)
    if ( !this.passesAccessControlCheck(res.rawHeaders, false, requestOrigin) )
    {
        this.blocked = true;
        var message = "CORS block for request to origin: '" + requestOrigin + "' from origin '" + appOrigin + "'";
        if (this.blockedHandler) {
            this.blockedHandler({message});
        } else if (this.errorHander) {
            this.errorHander({message});
        }
        NativeHttpManager.remove(this);
        return;
    }
    
    this.emit.$emit('response', res);
    NativeHttpManager.remove(this);
}

NativeHttp.prototype.on = function(eventName, handler) {

    if(eventName === 'error') {
        this.errorHander = handler;
    }

    if(eventName === 'blocked') {
        this.blockedHandler = handler;
    }

    if(eventName === 'response') {  // no need emit to native instance
        this.emit.$on('response', handler);
        return;
    }
    
    this._httpInstance.on(eventName, handler);
    return this;
};

NativeHttp.prototype.abort = function() {
    this._httpInstance.abort();
}

NativeHttp.prototype.end = function() {

    var requestOrigin = this._getRequestOrigin();
    var permissions = this.scene ? this.scene.permissions : null;
    // check permissions
    if (permissions) {
        // no permissions
        if (!permissions.allows(requestOrigin)) {
            // next tick
            setTimeout(() => {
                this.blocked = true;
                if (this.blockedHandler) {
                    this.blockedHandler({message: "Permissions block"});
                } else if (this.errorHander) {
                    this.errorHander({message: "Permissions block"});
                }
                NativeHttpManager.remove(this);
            }, 1);
            return;
        }
    }

    var appOrigin = this.origin();
    if (appOrigin) {
        this.setHeader("Origin", appOrigin);
    }

    this._httpInstance.end();
}
NativeHttp.prototype.write = function(...args) {
    this._httpInstance.write(...args);
}

NativeHttp.prototype.passesAccessControlCheck = function (rawHeaders, withCredentials, origin) {
    if (this.scene) {
      var cors = this.scene.cors;
      if (cors) {
        return cors.passesAccessControlCheck(rawHeaders, withCredentials, origin);
      }
    }
    return true;
};

NativeHttp.prototype.setTimeout = function(ms, timeoutCB) {
    // c++ native didn't implement the setTimeout
    // so i implement this in js side
    this.timeoutHandler = setTimeout(()=>{
        this.isTimeout = true;
        timeoutCB();
        if (this.errorHander) {
            this.errorHander({message:'request timeout'});
        }
    }, ms);
}
NativeHttp.prototype.setHeader = function(...args) {
    this._httpInstance.setHeader(...args);
}

NativeHttp.prototype.getHeader = function(...args) {
    return this._httpInstance.getHeader(...args);
}

NativeHttp.prototype.removeHeader = function(...args) {
    this._httpInstance.removeHeader(...args);
}

NativeHttp.prototype.origin = function () {
    if (this.scene) {
      return this.scene.origin;
    }
    return null;
};

NativeHttp.prototype._getRequestOrigin = function () {
    if (typeof this.options === 'object') {
        return `${this.options.protocol}://${this.options.host}`;
    }
    var parts = url.parse(this.options);
    return `${parts.protocol}//${parts.host}`;
}

NativeHttp.isCORSRequestHeader = function (name) {
    if (name) {
      if (name.match(/^(Origin|Access-Control-Request-Method|Access-Control-Request-Headers)$/ig)) {
        return true;
      }
    }
    return false;
};

module.exports = (scene)=>{
    return {
        'get': (...args) => NativeHttpManager.get(scene, ...args),
        'request': (...args) => NativeHttpManager.request(scene, ...args),
    };
};