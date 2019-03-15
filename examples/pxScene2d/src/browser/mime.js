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
 * Mime renderer
 */
'use strict';

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

  var baseUrl = "https://www.pxscene.org/examples/px-reference/gallery/";

  function resolveSceneUrl(url) {
    url = prepareUrl(url);

    /*
    if (url && url.toLowerCase().indexOf('.js?') > 0) { // this is a js file with query params
      return url;
    }
    if (url && !url.match(/\.js$/)) {
      url = 'mimeScene.js?url=' + url;
    }
    */

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

    if(url == "about.js")
    {
      url = "about.js";
    }
    else if ( (url.indexOf(':') == -1) && url.endsWith(".js") )
    {
      url = baseUrl + url;
    }

    var ext = url.split('.').pop();

    if (ext !== 'js') {
      // remove file protocol for mime loaders
      url = url.replace(/^file:\/\//i, '');
    }

    return url;
  }

  module.exports.resolveSceneUrl = resolveSceneUrl