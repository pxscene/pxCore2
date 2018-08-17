/**
 * List of supported file extensions and associated mime types (renderers)
 *
 * There are two types of mime renderers:
 * - scene - are rendered as a scene
 * - object - are rendered as a object
 */

'use strict';
const mimeBaseURL = 'https://sparkdown-dev.herokuapp.com/mime/'
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
