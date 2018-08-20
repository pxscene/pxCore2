/**
 * Mime renderer
 */
'use strict';

px.import({
  mimeTypes: 'pxMimeTypes.js',
})
.then(function importsAreReady(imports) {

  var mimeTypes = imports.mimeTypes;

  /**
   * This is helper method which resolves resource URL for scene
   * - it resolves various shortcuts using prepareUrl() method
   * also
   * - for .js files it returns URL as it is
   * - for other files (MIME files) it returns the URL of wrapper scene which
   *   will draw provided URL with the mimeRenderer
   *
   * @param {String} url url
   *
   * @returns {String} URL for a scene
   */
  function resolveSceneUrl(url) {
    url = prepareUrl(url);

    if (url && url.toLowerCase().indexOf('.js?') > 0) { // this is a js file with query params
      return url;
    }
    if (url && !url.match(/\.js$/)) {
      url = 'mimeScene.js?url=' + url;
    }

    return url;
  }
  
  /**
   * Prepares URL by unifying it
   *
   * Mainly resolves URL shortcuts
   *
   * @param {String} url url
   *
   * @return {String} unified URL
   */
  function prepareUrl(url) {
    // resolve shortcuts
    if (url.indexOf('local:') === 0) { // LOCAL shorthand
      var txt = url.slice(6, url.length);
      var pos = txt.indexOf(':');

      if (pos == -1) {
        // SHORTCUT:   'local:filename.js'  >>  'http://localhost:8080/filename.js' (default to 8080)
        url = 'http://localhost:8080/' + txt;
      } else {
        var str = txt.split('');
        str[pos] = '/'; // replace : with /
        txt = str.join('');

        // SHORTCUT:   'local:8081:filename.js' >> 'http://localhost:8081/filename.js'
        url = 'http://localhost:' + txt;
      }
    }

    var ext = url.split('.').pop();

    if (ext !== 'js') {
      // remove file protocol for mime loaders
      url = url.replace(/^file:\/\//i, '');
    }

    return url;
  }

  function loadObjectRenderer(url, mimeType, args) {
    if(args && args.from === 'markdown') { // skip check
      return _loadObjectRenderer.call(this, url, mimeType, args);
    } else {
      px.getFile(mimeType.url).then(() => {
        return _loadObjectRenderer.call(this, url, mimeType, args);
      }).catch((err) => {
        this.showError("Load " + mimeType.url + " failed, please check your mime server.");
      });
    }
  }

  function _loadObjectRenderer(url, mimeType, args) {
    console.log('loadObjectRenderer ### ' + mimeType.url + ' exit, start to load..');
    px.import({
      mimeTypeRenderer: mimeType.url
    }).then((mimeTypeRendererImports) => {
      var createRenderer = mimeTypeRendererImports.mimeTypeRenderer.createRenderer;
      var options = {
        parent: this.container,
        url: url,
        mimeType: mimeType,
        args: args,
      };
      // options which we pass from general mime renderer to particular implementations
      ['maxWidth'].forEach((opt) => {
        if (this.options[opt]) {
          options[opt] = this.options[opt];
        }
      });
      this.renderer = createRenderer(this.scene, options);
      this.rendererDefer.resolve(this.renderer.ready);
    });
  }

  function loadSceneRenderer(url, mimeType) {
    this.renderer = this.scene.create({
      t: 'scene',
      parent: this.container,
      url: url,
      clip: true,
      mimeType: mimeType,
      h: this.container.h,
      w: this.container.w,
      args: {}
    });
    this.rendererDefer.resolve(this.renderer.ready);
  }

  /**
   * Creates RT object with rendered file which is defined `options.url`.
   *
   * File is rendered according to its MIME type using specific mime renderers.
   *
   * It supports two types of mime renderers:
   * - scene - the one which implemented by scene
   * - object - the one which implemented by rt object
   *
   * @param {Object} scene   scene
   * @param {Object} options options similar to regular rtObject
   */
  function MimeRenderer(scene, options) {
    this.scene = scene;
    this.parent = options.parent;
    this.options = options;
    this.options.args = this.options.args || {};

    this._url;
    this.renderer;
    this.rendererDefer = Promise.defer();
    this.rendererReady = this.rendererDefer.promise;

    this.container = scene.create({
      t: 'object',
      parent: this.parent,
      clip: true,
    });

    Object.defineProperty(this, 'url', {
      set: function (val) {
        {
          // clear what was rendered
          this.container.removeAll();
          this.renderer = null;
          this.rendererDefer = Promise.defer();
          this.rendererReady = this.rendererDefer.promise;

          this._url = prepareUrl(val);

          var ext = this._url.split('.').pop();
          var mimeType = mimeTypes[ext] || mimeTypes[''];

          if(mimeType === mimeTypes[''] && options.args.from === 'link') {
            if(val.toLowerCase().indexOf('http') === 0) {
              if(val.toLowerCase().split('.').pop() === 'js') {
                loadSceneRenderer.call(this, val, mimeType);  // spark application
                return;
              } else {
                mimeType = mimeTypes['txt'];
              }
            }
          }
          // for object type mime renderers
          if (mimeType.type === 'object') {
            loadObjectRenderer.call(this, this._url, mimeType, options.args || {});

          // for scene type mime renderers
          } else {
            loadSceneRenderer.call(this, `${mimeType.url}?url=${this._url}`, mimeType);
          }
        }
      },
      get: function () {
        return this._url;
      },
    });

    // ready property will be resolved when both
    // this.container and this.renderer are ready
    Object.defineProperty(this, 'ready', {
      get: function () {
        return this.container.ready.then(() => {
          return this.rendererReady;
        });
      },
    });

    // read/write props for both container and renderer
    ['w', 'h'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        set: function (val) {
          this.container[prop] = val;

          // if have renderer, set same props
          if (this.renderer) {
            this.renderer[prop] = val;
          }
        },
        get: function () {
          return this.container[prop];
        },
      });
    });

    // we get/set focus preferably to renderer and fallback to container
    ['focus'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        set: function (val) {
          if (this.renderer && typeof this.renderer[prop] !== 'undefined') {
            this.renderer[prop] = val;
          } else {
            this.container[prop] = val;
          }
        },
        get: function () {
          if (this.renderer && typeof this.renderer[prop] !== 'undefined') {
            return this.renderer[prop];
          } else {
            return this.container[prop];
          }
        },
      });
    });

    // this props we set/get only to container
    ['parent', 'x', 'y'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        set: function (val) {
          this.container[prop] = val;
        },
        get: function () {
          return this.container[prop];
        },
      });
    });

    // resource only renderer can provide
    ['resource'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        get: function () {
          if (this.renderer) {
            return this.renderer[prop];
          }
        },
      });
    });

    // read/write properties for renderer
    ['maxWidth'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        get: function () {
          if (this.renderer) {
            return this.renderer[prop];
          }
        },
        set: function (val) {
          if (this.renderer) {
            this.renderer[prop] = val;
          }
        },
      });
    });

    // apply options defined in constructor
    Object.keys(this.options).forEach((prop) => {
      this[prop] = this.options[prop];
    });
  }

  MimeRenderer.prototype.showError = function(txt) {
    if(!this.errorText) {
      this.errorText = this.scene.create({
        t: 'textBox',
        parent: this.scene.root,
        interactive: false,
        w: this.scene.root.w,
        h: this.scene.root.h,
        text: txt,
        textColor: 0xFF0000FF,
        pixelSize: 16,
        wordWrap: true,
        alignHorizontal: this.scene.alignHorizontal.CENTER,
        alignVertical: this.scene.alignVertical.CENTER /* looks like this doesn't work */
      });
    }
    this.errorText.text = txt;
  }

  MimeRenderer.prototype.on = function(eventName, listener) {
    this.container.on(eventName, listener);
  }

  module.exports.MimeRenderer = MimeRenderer;
  module.exports.resolveSceneUrl = resolveSceneUrl;

}).catch(function importFailed(err) {
  console.error("##### mime.js Import failed: " + err);
});
