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
