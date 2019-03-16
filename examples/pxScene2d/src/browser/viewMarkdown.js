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

//'use strict';

px.import({
  scene: 'px:scene.1.js',
  scrollable: 'components/scrollable.js',
  markdown: 'components/markdown.js'
}).then(function importsAreReady(imports) {

  var scene = imports.scene;
/*
  scene.create({
    t: "text",
    parent: scene.root,
    text: "Top"
  });
  */
  
  /**
   * Variables
   */
  var Scrollable = imports.scrollable.Scrollable;
  var Markdown = imports.markdown.Markdown;

  /**
   * Private method to render markdown
   *
   * @param {String} markdownSource source of markdown
   */
  function renderMarkdown(markdownSource) {
    this.scrollable = new Scrollable(this.scene, this.container, {blank: this.options.args.from === 'markdown'});

    //var mimeURL = this.options.mimeType.url.split('/');
    //var mimeURL = 'https://www.pxscene.org/mime/';
    var mimeURL = px.getPackageBaseFilePath()+'/'
    //mimeURL.pop();
    this.markdown = new Markdown(this.scene, this.scrollable, {
      basePath: this.basePath,
      mimeURL: mimeURL//mimeURL.join('/') + '/',
    });    
    
    var md = this.markdown;
    this.markdown.prepare().then(() => {
      md.setSource(markdownSource);
      this.renderDefer.resolve(md);
    }).catch((err) => {
      console.error(err);
      md.setSource("#### failed process markdown, " + JSON.stringify(err));
      this.renderDefer.reject(err);
    });
  }

  /**
   * Markdown Mime Renderer
   *
   * @param {Object} scene   scene
   * @param {Object} options regular object options
   */
  function MarkdownMimeRenderer(scene, options) {
    this.scene = scene;
    this.options = options;

    this._url;
    this.basePath;
    this.scrollable;
    this.markdown;

    this.renderDefer = Promise.defer();
    this.renderReady = this.renderDefer.promise;

    this.container = scene.create({
      t: "object",
      parent: options.parent,
      interactive: true,
      w: options.maxWidth || options.parent.w,
      h: options.parent.h,
      draw: true,
      clip: true,
    });

    // set url
    Object.defineProperty(this, 'url', {
      set: function(url) {
        this._url = url;
        var separatorMatch = url.match(/[\\\/]{1}/);
        var separator = separatorMatch ? separatorMatch[0] : '/';
        this.basePath = url.split(separator).slice(0, -1).join(separator) + separator;

        this.renderDefer = Promise.defer();
        this.renderReady = this.renderDefer.promise;

        px.getFile(url)
          .then((markdownSource) => {
            renderMarkdown.call(this, markdownSource ? markdownSource.toString() : '');
          })
          .catch((err) => {
            console.log(err);
            console.log("get Markdown failed ####" + url);
            //renderMarkdown.call(this, "#### Load markdown file failed");
          });
      }
    });

    Object.defineProperty(this, 'content', {
      set: function(c) {
        console.log('&&&&&&&&&&& before renderMarkdown call', c)
        renderMarkdown.call(this, c)
      }
    })

    // ready
    Object.defineProperty(this, 'ready', {
      get: function () {
        return this.renderReady;
      },
    });

    // read/write props for both container and renderer
    ['w', 'h'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        set: function (val) {
          this.container[prop] = val;

          if (this.scrollable) {
            this.scrollable.update();
          }
        },
        get: function () {
          return this.container[prop];
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

    // resource
    ['resource'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        get: function () {
          if (this.markdown) {
            // for now make markdown report the height not more than 500
            var h = this.options.args.from === 'markdown' ? this.markdown.container.h : Math.min(this.markdown.container.h, 500);
            return {
              h: h,
              w: options.maxWidth || this.container.w,
            };
          }
        },
      });
    });

    // read/write options
    
    ['maxWidth'].forEach((prop) => {
      Object.defineProperty(this, prop, {
        get: function () {
          return options[prop];
        },
        set: function (val) {
          options[prop] = val;
        }
      });
    });
    

    // apply options defined in constructor
    Object.keys(this.options).forEach((prop) => {
      this[prop] = this.options[prop];
    });
  }

  function createRenderer(scene, option) {
    return new MarkdownMimeRenderer(scene, option);
  }

  var mdUrl = px.appQueryParams.url;
  var r = createRenderer(scene, {parent:scene.root, url:mdUrl, args:{from:''}})

  function updSize(w,h)
  {
    r.w = w
    r.h = h
  }

  scene.on("onResize", function(e) { updSize(e.w,e.h) })

  updSize(scene.w,scene.h)

  console.log('~~~~~~~~~~~~~~~~~ exposing setContent')
  module.exports.setContent = function(c) {
    console.log('!!!!!!!!!!! trying to set content')
    r.content = c
  }
  
  //module.exports.MarkdownMimeRenderer = MarkdownMimeRenderer;
  //module.exports.createRenderer = createRenderer;
/*
  scene.create({
    t: "text",
    parent: scene.root,
    text: "Bottom",
    y:100
  });
  */

}).catch(function importFailed(err){
  console.error("Import failed: " + err);
});
