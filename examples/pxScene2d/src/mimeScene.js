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
 * Wrapper scene to render via MIME renderer
 *
 * It just renders provided URL in the whole available space.
 */
'use strict';

px.import({
  scene: 'px:scene.1.js',
  mime: 'mime.js'
}).then(function ready(imports) {

  var scene = imports.scene;
  var MimeRenderer = imports.mime.MimeRenderer;
  var url = px.appQueryParams.url;
  var type = px.appQueryParams.type;
  var from = px.appQueryParams.from;

  var mimeRenderer = new MimeRenderer(scene, {
    url: url,
    parent: scene.root,
    w: scene.w,
    h: scene.h,
    args: {type, from},
  });

  function updateSize(e) {
    mimeRenderer.w = e.w;
    mimeRenderer.h = e.h;
  }

  scene.on('onResize', updateSize);

  module.exports.mimeRenderer = mimeRenderer;
}).catch(function importFailed(err) {
  console.error("Import mimeScene.js failed: " + err)
});
