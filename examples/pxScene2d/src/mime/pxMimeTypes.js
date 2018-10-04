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
 * List of supported file extensions and associated mime types (renderers)
 *
 * There are two types of mime renderers:
 * - scene - are rendered as a scene
 * - object - are rendered as a object
 */
'use strict';
const mimeBaseURL = 'https://www.pxscene.org/mime/'
module.exports = {
  svg: {
    url: mimeBaseURL + 'mime_IMAGE.js',
    type: 'object'
  },
  jpg: {
    url: mimeBaseURL + 'mime_IMAGE.js',
    type: 'object'
  },
  png: {
    url: mimeBaseURL + 'mime_IMAGE.js',
    type: 'object'
  },
  txt: {
    url: mimeBaseURL + 'mime_TEXT.js',
    type: 'object'
  },
  html: {
    url: mimeBaseURL + 'mime_TEXT.js',
    type: 'object'
  },
  md: {
    url: mimeBaseURL + 'mime_MARKDOWN.js',
    type: 'object'
  },
  '':  {
    url: mimeBaseURL + 'mime_UNSUPPORTED.js',
    type: 'object'
  }
};
