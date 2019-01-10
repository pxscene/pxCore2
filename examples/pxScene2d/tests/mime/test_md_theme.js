/**
 * unit tests for markdown theme
 */
px.import({    scene: "px:scene.1.js",
            testUtil: "test_md_util.js",
              assert: "../../test-run/assert.js",
              manual: "../../test-run/tools_manualTests.js"
}).then( function ready(imports)
{
  var assert = imports.assert.assert;
  var manual = imports.manual;
  var scene = imports.scene;
  var testUtil = imports.testUtil;
  var manualTest = manual.getManualTestValue();
  var childScene = scene.create({t:"scene",  w:scene.root.w, h:scene.root.h, parent:scene.root});
  var basePackageUri = px.getPackageBaseFilePath();

  var tests = {
    /**
     * test markdown container
     */
    test_theme_container: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/bold.md', function(payload) {
        var results = [];
        var container = payload.options.styles.container;

        var expectedW = payload.root.parent.w - container.paddingLeft - container.paddingRight;
        var expectedH = payload.root.children[0].children[0].h + container.paddingTop + container.paddingBottom;

        results.push(assert(payload.root.x === container.paddingLeft, 'position x shoud be ' + container.paddingLeft));
        results.push(assert(payload.root.y === container.paddingTop,  'position y shoud be ' + container.paddingTop));
        results.push(assert(payload.root.h === expectedH, 'height shoud be ' + expectedH));
        results.push(assert(payload.root.w === expectedW, 'width should be ' + expectedW));
        return results;
      }); 
    },

    /**
     * test the paragraph theme
     */
    test_theme_paragraph: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/paragraph.md', function(payload) {
        var results = [];
        var paragraph = payload.options.styles.paragraph;
        var FONT_STYLE = payload.options.FONT_STYLE;
        var textNode =  payload.root.children[0].children[0];

        results.push(assert(textNode.font === FONT_STYLE.REGULAR, 'font should be REGULAR'));
        results.push(assert(textNode.textColor === paragraph.textColor,  'textColor shoud be ' + paragraph.textColor));
        results.push(assert(textNode.pixelSize === paragraph.pixelSize, 'pixelSize shoud be ' + paragraph.pixelSize));
        results.push(assert(textNode.h === textNode.parent.h - paragraph.marginBottom, 'marginBottom should be ' + paragraph.marginBottom));
        return results;
      }); 
    },

    /**
     * test the quote theme
     */
    test_theme_quote: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/blockquote.md', function(payload) {
        var results = [];
        var blockquote = payload.options.styles.blockquote;
        var FONT_STYLE = payload.options.FONT_STYLE;

        var rectangle = payload.root.children[0].children[0];
        var textNode = payload.root.children[0].children[1].children[0];

        results.push(assert(textNode.font === FONT_STYLE.REGULAR, 'font should be REGULAR'));
        results.push(assert(textNode.textColor === blockquote.textColor, 'textColor shoud be ' + blockquote.textColor));
        results.push(assert(textNode.pixelSize === blockquote.pixelSize, 'pixelSize shoud be ' + blockquote.pixelSize));
        results.push(assert(rectangle.x === blockquote.lineOffsetLeft, 'lineOffsetLeft shoud be ' + blockquote.lineOffsetLeft));
        results.push(assert(rectangle.w === blockquote.lineWidth, 'lineWidth shoud be ' + blockquote.lineWidth));
        results.push(assert(rectangle.fillColor === blockquote.lineColor, 'lineColor shoud be ' + blockquote.lineColor));
        results.push(assert(textNode.parent.x === blockquote.paddingLeft, 'paddingLeft shoud be ' + blockquote.paddingLeft));
        results.push(assert(textNode.parent.h === textNode.h + blockquote.marginBottom, 'marginBottom shoud be ' + blockquote.marginBottom));
        return results;
      }); 
    },

    /**
     * test the header theme
     */
    test_theme_header: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/header.md', function(payload) {
        var results = [];
        var styles = payload.options.styles;
        for(var i = 0 ; i < 6; i += 1) {
          var headerTextNode = payload.root.children[i].children[0];
          var style = styles['header-' + (i+1)];
          var key = 'header-' + (i+1);

          results.push(assert(headerTextNode.font === style.font, key + " font should be " + style.font.url));
          results.push(assert(headerTextNode.textColor === style.textColor, key + " textColor should be " + style.textColor));
          results.push(assert(headerTextNode.pixelSize === style.pixelSize, key + " pixelSize should be " + style.pixelSize));
          results.push(assert(headerTextNode.parent.h === headerTextNode.h + style.marginBottom, key + " marginBottom should be " + style.marginBottom));
        }
        return results;
      }); 
    },

    /**
     * test the code theme
     */
    test_theme_code: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/code.md', function(payload) {
        var results = [];
        var styles = payload.options.styles;
        var inlineCodeRoot = payload.root.children[0];
        var codeRoot = payload.root.children[1];

        var codeText = codeRoot.children[1];
        var codeRect = codeRoot.children[0];
        var code = styles.code;
        
        results.push(assert(inlineCodeRoot.children[0].font === styles.codespan.font, "font should be " + styles.codespan.font.url));
        results.push(assert(codeText.textColor === code.textColor, "textColor should be " + code.textColor));
        results.push(assert(codeText.font === code.font, "font should be " + code.font.url));
        results.push(assert(codeRect.lineColor === code.lineColor, "lineColor should be " + code.lineColor));
        results.push(assert(codeRect.lineWidth === code.lineWidth, "lineWidth should be " + code.lineWidth));
        results.push(assert(codeRect.fillColor === code.fillColor, "fillColor should be " + code.fillColor));
        results.push(assert(codeRoot.parent.h === codeRoot.h + code.marginBottom + code.paddingTop + code.paddingBottom, "marginBottom should be " + code.marginBottom));

        results.push(assert(codeText.y === code.paddingTop, "paddingTop should be " + code.paddingTop));
        results.push(assert(codeText.x === code.paddingLeft, "paddingLeft should be " + code.paddingLeft));
        results.push(assert(codeText.w === codeText.parent.w - code.paddingRight - code.paddingLeft, "paddingRight should be " + code.paddingRight));
        results.push(assert(codeRect.h === codeText.h + code.paddingTop + code.paddingBottom, "paddingBottom should be " + code.paddingBottom));
      
        return results;
      }); 
    },

    /**
     * test the list theme
     */
    test_theme_list: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/list.md', function(payload) {
        var results = [];
        var listRoot = payload.root.children[0];
        var styles = payload.options.styles;
        var list = styles.list;
        var listItem = styles['list-item'];

        var itemTitleTxt = listRoot.children[0];
        var itemText = listRoot.children[1].children[0].children[0];

        results.push(assert(itemText.font === listItem.font, "list-item font should be " + listItem.font.url));
        results.push(assert(itemText.textColor === listItem.textColor, "list-item textColor should be " + listItem.textColor));
        results.push(assert(itemText.pixelSize === listItem.pixelSize, "list-item pixelSize should be " + listItem.pixelSize));
        results.push(assert(itemTitleTxt.x === listItem.symbolOffsetLeft, "list-item symbolOffsetLeft should be " + listItem.symbolOffsetLeft));
        results.push(assert(listRoot.children[1].x === listItem.paddingLeft, "list-item paddingLeft should be " + listItem.paddingLeft));

        var expectedH = listRoot.children[1].h * 3 + list.marginBottom - listItem.marginBottom;
        results.push(assert(listRoot.h === expectedH, "list-item marginBottom should be " + listItem.marginBottom));
        results.push(assert(listRoot.h === expectedH, "list marginBottom should be " + list.marginBottom));

        return results;
      }); 
    },

    /**
     * test the unordered list theme
     */
    test_theme_unordered_list: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/unordered-list.md', function(payload) {
        var results = [];
        var listRoot = payload.root.children[0];
        var styles = payload.options.styles;
        var list = styles.list;
        var listItem = styles['list-item'];

        var itemImage = listRoot.children[0];
        var itemText = listRoot.children[1].children[0].children[0];

        results.push(assert(itemText.font === listItem.font, "list-item font should be " + listItem.font.url));
        results.push(assert(itemText.textColor === listItem.textColor, "list-item textColor should be " + listItem.textColor));
        results.push(assert(itemText.pixelSize === listItem.pixelSize, "list-item pixelSize should be " + listItem.pixelSize));
        results.push(assert(itemImage.x === listItem.symbolOffsetLeft, "list-item symbolOffsetLeft should be " + listItem.symbolOffsetLeft));
        results.push(assert(listRoot.children[1].x === listItem.paddingLeft, "list-item paddingLeft should be " + listItem.paddingLeft));

        var expectedH = listRoot.children[1].h * 3 + list.marginBottom - listItem.marginBottom;
        results.push(assert(listRoot.h === expectedH, "list-item marginBottom should be " + listItem.marginBottom));
        results.push(assert(listRoot.h === expectedH, "list marginBottom should be " + list.marginBottom));

        return results;
      }); 
    },

    /**
     * test the link theme
     */
    test_theme_link: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/link.md', function(payload) {
        var results = [];
        var link = payload.root.children[0].children[0];
        var styles = payload.options.styles;
        results.push(assert(link.textColor === styles.link.textColor, 'textColor shoud be ' + styles.link.textColor));
        return results;
      }); 
    },

    /**
     * test the italic/em theme
     */
    test_theme_em: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/italics.md', function(payload) {
        var results = [];
        var styles = payload.options.styles;
        var textNode = payload.root.children[0].children[0];
        results.push(assert(textNode.font === styles.em.font, 'font shoud be ' + styles.em.font.url));
        return results;
      }); 
    },

    /**
     * test the strong theme
     */
    test_theme_strong: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/bold.md', function(payload) {
        var results = [];
        var styles = payload.options.styles;
        var textNode = payload.root.children[0].children[0];
        results.push(assert(textNode.font === styles.strong.font, 'font shoud be ' + styles.strong.font.url));
        return results;
      }); 
    },

    /**
     * the underline theme
     */
    test_theme_underline: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/underline.md', function(payload) {
        var results = [];
        var underlineRectangle = payload.root.children[0].children[0].children[0];
        var underline = payload.options.styles.underline;
        results.push(assert(underlineRectangle.h === underline.height, "height should be " + underline.height));
        results.push(assert(underlineRectangle.fillColor === underline.fillColor, "fillColor should be " + underline.fillColor));
        return results;
      }); 
    },

    /**
     * test the inline code theme
     */
    test_theme_codespan: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/code.md', function(payload) {
        var results = [];
        var styles = payload.options.styles;
        var inlineCodeRoot = payload.root.children[0];
        results.push(assert(inlineCodeRoot.children[0].font === styles.codespan.font, "font should be " + styles.codespan.font.url));
        return results;
      }); 
    },
  }

  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }

}).catch( function importFailed(err){
  console.error("Import failed for test_md_list.js: " + err)
});
