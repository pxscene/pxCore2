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
 * Markdown style configuration
 *
 * Most values are clear by their name, some are clarified.
 *
 * NOTE about defining fonts.
 * First we define different font styles, and after just use their ids.
 */
'use strict'

module.exports = {
  // paths for various font styles relative to Resource folder
  
  FONT_STYLE: {
    REGULAR: 'fonts/OpenSans-Regular.woff',
    ITALIC: 'fonts/OpenSans-Italic.woff',
    BOLD: 'fonts/OpenSans-Bold.woff',
    BOLD_ITALIC: 'fonts/OpenSans-BoldItalic.woff',
    MONOSPACE: 'fonts/SourceCodePro-Regular.woff',
  },

  // styles of various blocks
  styles: {
    // container of the whole markdown document
    container: {
      paddingTop: 10,
      paddingRight: 10,
      paddingBottom: 10,
      paddingLeft: 10,
    },

    // BLOCK level blocks
    paragraph: {
      font: 'REGULAR',
      textColor: 0x000000FF,
      pixelSize: 16,
      marginBottom: 20,
    },
    blockquote: {
      font: 'REGULAR',
      textColor: 0x000000FF,
      pixelSize: 16,
      lineOffsetLeft: 15,    // left offset of the decor line
      lineWidth: 5,          // width of the decor line
      lineColor: 0x000000FF, // color of the decor line
      paddingLeft: 30,       // padding left of the text ignoring decor line
      marginBottom: 20,
    },
    'header-1': {
      font: 'BOLD',
      textColor: 0x000000FF,
      pixelSize: 45,
      marginBottom: 20,
    },
    'header-2': {
      font: 'BOLD',
      textColor: 0x000000FF,
      pixelSize: 36,
      marginBottom: 20,
    },
    'header-3': {
      font: 'BOLD',
      textColor: 0x000000FF,
      pixelSize: 27,
      marginBottom: 20,
    },
    'header-4': {
      font: 'BOLD',
      textColor: 0x000000FF,
      pixelSize: 22,
      marginBottom: 20,
    },
    'header-5': {
      font: 'BOLD',
      textColor: 0x000000FF,
      pixelSize: 18,
      marginBottom: 20,
    },
    'header-6': {
      font: 'BOLD',
      textColor: 0x000000FF,
      pixelSize: 16,
      marginBottom: 20,
    },
    code: {
      textColor: 0x000000FF,
      font: 'MONOSPACE',
      lineColor: 0xCCCCCCFF, // line color of the code decoration block
      lineWidth: 1,          // line width of the code decoration block
      fillColor: 0xF5F5F5FF, // background color
      marginBottom: 20,      // space after block with background
      paddingTop: 10,
      paddingRight: 10,
      paddingBottom: 10,
      paddingLeft: 10,
    },
    list: {
      marginBottom: 20,      // space after the whole list
    },
    'list-item': {
      font: 'REGULAR',
      textColor: 0x000000FF,
      pixelSize: 16,
      symbol: '•',           // symbol for unordered lists
      symbolOffsetLeft: 10,  // left offset of the symbol or number for order lists
      paddingLeft: 30,       // padding left of the text ignoring symbol and number
      marginBottom: 10,      // space after list item (this space doesn't sum up with list marginBottom)
    },

    // INLINE level block
    text: {                  // no specific style for regular inline text
    },
    link:{
      textColor: 0x2e62b2ff,
      activeColor: 0x1e3e72ff,
    },
    em: {                    // emphasis text (italic)
      font: 'ITALIC',
    },
    strong: {                // strong text (bold)
      font: 'BOLD',
    },
    underline:{
      height: 1,
      fillColor: 0x000000ff,
    },
    codespan: {              // inline code text (monospace)
      font: 'MONOSPACE',
    },
  },
};
