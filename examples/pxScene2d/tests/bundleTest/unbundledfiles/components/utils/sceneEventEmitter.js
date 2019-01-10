/**
 * This is a workaround for scene events.
 *
 * Objects of scene support only one event handler with the same source code.
 * Also, event listeners looks like cannot be deleted from scene.
 */
'use strict'
px.import({
  events: './events.js',
}).then(function importsAreReady(imports) {

  var EventEmitter = imports.events;

  /**
   * Return event emitter for scene
   *
   * It cache event emitter and return for the scene same event emitter.
   *
   * @param {Object} scene scene
   *
   * @returns {EventEmitter} eventEmitter
   */
  function getSceneEventEmitter(scene) {
    var cachedScene = getSceneEventEmitter.cache.find((cacheItem) => {
      return cacheItem.scene === scene;
    });

    if (!cachedScene) {
      cachedScene = {
        scene: scene,
        ee: new EventEmitter(),
      }
      cachedScene.ee.setMaxListeners(10000);

      getSceneEventEmitter.supportedEvents.forEach((eventName) => {
        scene.on(eventName, (evt) => {
          cachedScene.ee.emit(eventName, evt);
        });
      });

      getSceneEventEmitter.cache.push(cachedScene);
    }

    return cachedScene.ee;
  }

  // the full list of the events supported by scene
  getSceneEventEmitter.supportedEvents = [
    'onMouseDown',
    'onMouseUp',
    'onMouseMove',
    'onMouseEnter',
    'onMouseLeave',
    'onFocus',
    'onBlur',
    'onKeyDown',
    'onKeyUp',
    'onChar',
    'onResize',
  ]

  getSceneEventEmitter.cache = [];

  module.exports.getSceneEventEmitter = getSceneEventEmitter;

}).catch(function importFailed(err) {
  console.error("Import failed: " + err);
});
