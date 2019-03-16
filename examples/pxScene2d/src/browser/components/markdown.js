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
 * Renders markdown by source
 *
 * The code is based on markit (https://github.com/lepture/markit)
 * So the original code can be used to extend functionality of the current implementation
 * which was cut to set of supported functions:
 * - Emphasis
 * - Ordered lists
 * - Unordered lists
 * - Headers
 * - Images (using the MIME dispatcher)
 * - Blockquotes
 * - Inline/block code (without coloring)
 *
 * The implementation has several sections:
 * - `block` - is keept as it is the full support of markdown
 * - `Lexer` - is limited to supported functions only
 * - `inline`- is keept as it is the full support of markdown
 * - `InlineLexer` - is limited to supported functions only
 * - `Renderer` - is limited to supported functions only
 * - `Parser`   - is limited to supported functions only
 */
px.import({
  style: 'markdown.style.js'
}).then(function importsAreReady(imports) {

  var style = imports.style;

  /**
   * Block-Level Grammar
   */

  function _eventEmitter() {
    this.handlers = {}
    this.on = function(eventName, eventHandler) {
      if (!this.handlers[eventName])
        this.handlers[eventName] = []
      this.handlers[eventName].push(eventHandler)
    }
    this.emit = function(eventName) {
      console.log('firing event: ', eventName)
      var handlerz = this.handlers[eventName]
      if (handlerz) {
        for (var h of handlerz) {
          h()
        }
      }
    }
  }

  var block = {
    newline: /^\n+/,
    code: /^( {4}[^\n]+\n*)+/,
    fences: noop,
    hr: /^( *[-*_]){3,} *(?:\n+|$)/,
    heading: /^ *(#{1,6}) *( {1}) *([^\n]+?) *#* *(?:\n+|$)/,
    nptable: noop,
    lheading: /^([^\n]+)\n *(=|-){2,} *(?:\n+|$)/,
    blockquote: /^( *>[^\n]+(\n[^\n]+)*\n*)+/,
    list: /^( *)(bull) [\s\S]+?(?:hr|\n{2,}(?! )(?!\1bull )\n*|\s*$)/,
    html: /^ *(?:comment|closed|closing) *(?:\n{2,}|\s*$)/,
    def: /^ *\[([^^\]]+)\]: *<?([^\s>]+)>?(?: +["(]([^\n]+)[")])? *(?:\n+|$)/,
    footnote: noop,
    table: noop,
    paragraph: /^((?:[^\n]+\n?(?!hr|heading|lheading|blockquote|tag|def))+)\n*/,
    text: /^[^\n]+/
  };

  block.bullet = /(?:[*+-]|\d+\.)/;
  block.item = /^( *)(bull) [^\n]*(?:\n(?!\1bull )[^\n]*)*/;
  block.item = replace(block.item, 'gm')
    (/bull/g, block.bullet)
    ();

  block.list = replace(block.list)
    (/bull/g, block.bullet)
    ('hr', /\n+(?=(?: *[-*_]){3,} *(?:\n+|$))/)
    ();

  block._tag = '(?!(?:'
    + 'a|em|strong|small|s|cite|q|dfn|abbr|data|time|code'
    + '|var|samp|kbd|sub|sup|i|b|u|mark|ruby|rt|rp|bdi|bdo'
    + '|span|br|wbr|ins|del|img)\\b)\\w+(?!:/|[^\\w\\s@]*@)\\b';

  block.html = replace(block.html)
    ('comment', /<!--[\s\S]*?-->/)
    ('closed', /<(tag)[\s\S]+?<\/\1>/)
    ('closing', /<tag(?:"[^"]*"|'[^']*'|[^'">])*?>/)
    (/tag/g, block._tag)
    ();

  block.paragraph = replace(block.paragraph)
    ('hr', block.hr)
    ('heading', block.heading)
    ('lheading', block.lheading)
    ('blockquote', block.blockquote)
    ('tag', '<' + block._tag)
    ('def', block.def)
    ();

  var urlDetectRegex = /(https?:\/\/(?:www\.|(?!www))[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\.[^\s]{2,}|www\.[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\.[^\s]{2,}|https?:\/\/(?:www\.|(?!www))[a-zA-Z0-9]\.[^\s]{2,}|www\.[a-zA-Z0-9]\.[^\s]{2,})/;
  /**
   * Normal Block Grammar
   */

  block.normal = merge({}, block);

  /**
   * GFM Block Grammar
   */

  block.gfm = merge({}, block.normal, {
    fences: /^ *(`{3,}|~{3,}) *(\S+)? *\n([\s\S]+?)\s*\1 *(?:\n+|$)/,
    paragraph: /^/
  });
  block.gfm.paragraph = replace(block.paragraph)
    ('(?!', '(?!'
      + block.gfm.fences.source.replace('\\1', '\\2') + '|'
      + block.list.source.replace('\\1', '\\3') + '|')
    ();

  /**
   * Tables Block Grammar
   */

  block.tables = {
    nptable: /^ *(\S.*\|.*)\n *([-:]+ *\|[-| :]*)\n((?:.*\|.*(?:\n|$))*)\n*/,
    table: /^ *\|(.+)\n *\|( *[-:]+[-| :]*)\n((?: *\|.*(?:\n|$))*)\n*/
  };

  /**
   * Footnotes Block Grammar
   */
  block.footnotes = {
    footnote: /^\[(\^[^\]]+)\]: *([^\n]*(?:\n [^\n]*)*)/,
  };
  block.footnotes.normal = {
    footnote: block.footnotes.footnote
  };
  block.footnotes.normal.paragraph = replace(block.paragraph)(
    '))+)', '|' + block.footnotes.footnote.source + '))+)'
  )();
  block.footnotes.gfm = {
    footnote: block.footnotes.footnote
  };
  block.footnotes.gfm.paragraph = replace(block.gfm.paragraph)(
    '))+)', '|' + block.footnotes.footnote.source + '))+)'
  )();

  /**
   * Block Lexer
   */

  function Lexer(options) {
    this.tokens = [];
    this.tokens.links = {};
    this.tokens.footnotes = [];
    this.options = options || marked.defaults;
    this.rules = block.normal;

    if (this.options.gfm) {
      this.rules = block.gfm;
    }
  }

  /**
   * Expose Block Rules
   */

  Lexer.rules = block;

  /**
   * Static Lex Method
   */

  Lexer.lex = function(src, options) {
    var lexer = new Lexer(options);
    return lexer.lex(src);
  };

  /**
   * Preprocessing
   */

  Lexer.prototype.lex = function(src) {
    src = src
      .replace(/\r\n|\r/g, '\n')
      .replace(/\t/g, '    ')
      .replace(/\u00a0/g, ' ')
      .replace(/\u2424/g, '\n');

    return this.token(src, true);
  };

  /**
   * Lexing
   */

  Lexer.prototype.token = function(src, top) {
    var src = src.replace(/^ +$/gm, '')
      , next
      , key
      , loose
      , cap
      , bull
      , b
      , item
      , space
      , i
      , l;

    while (src) {
      // newline
      if (cap = this.rules.newline.exec(src)) {
        src = src.substring(cap[0].length);
        if (cap[0].length > 1) {
          this.tokens.push({
            type: 'space'
          });
        }
      }

      // code
      if (cap = this.rules.code.exec(src)) {
        src = src.substring(cap[0].length);
        cap = cap[0].replace(/^ {4}/gm, '');
        this.tokens.push({
          type: 'code',
          text: !this.options.pedantic
            ? cap.replace(/\n+$/, '')
            : cap
        });
        continue;
      }

      // fences (gfm)
      if (cap = this.rules.fences.exec(src)) {
        src = src.substring(cap[0].length);
        this.tokens.push({
          type: 'code',
          lang: cap[2],
          text: cap[3]
        });
        continue;
      }

      // heading
      if (cap = this.rules.heading.exec(src)) {
        src = src.substring(cap[0].length);
        this.tokens.push({
          type: 'heading',
          depth: cap[1].length,
          text: cap[3]
        });
        continue;
      }

      // lheading
      if (cap = this.rules.lheading.exec(src)) {
        src = src.substring(cap[0].length);
        this.tokens.push({
          type: 'heading',
          depth: cap[2] === '=' ? 1 : 2,
          text: cap[1]
        });
        continue;
      }

      // blockquote
      if (cap = this.rules.blockquote.exec(src)) {
        src = src.substring(cap[0].length);

        this.tokens.push({
          type: 'blockquote_start'
        });

        cap = cap[0].replace(/^ *> ?/gm, '');

        // Pass `top` to keep the current
        // "toplevel" state. This is exactly
        // how markdown.pl works.
        this.token(cap, top);

        this.tokens.push({
          type: 'blockquote_end'
        });

        continue;
      }

      // list
      if (cap = this.rules.list.exec(src)) {
        src = src.substring(cap[0].length);
        bull = cap[2];

        this.tokens.push({
          type: 'list_start',
          ordered: bull.length > 1
        });

        // Get each top-level item.
        cap = cap[0].match(this.rules.item);

        next = false;
        l = cap.length;
        i = 0;

        for (; i < l; i++) {
          item = cap[i];

          // Remove the list item's bullet
          // so it is seen as the next token.
          space = item.length;
          item = item.replace(/^ *([*+-]|\d+\.) +/, '');

          // Outdent whatever the
          // list item contains. Hacky.
          if (~item.indexOf('\n ')) {
            space -= item.length;
            item = !this.options.pedantic
              ? item.replace(new RegExp('^ {1,' + space + '}', 'gm'), '')
              : item.replace(/^ {1,4}/gm, '');
          }

          // Determine whether the next list item belongs here.
          // Backpedal if it does not belong in this list.
          if (this.options.smartLists && i !== l - 1) {
            b = block.bullet.exec(cap[i + 1])[0];
            if (bull !== b && !(bull.length > 1 && b.length > 1)) {
              src = cap.slice(i + 1).join('\n') + src;
              i = l - 1;
            }
          }

          // Determine whether item is loose or not.
          // Use: /(^|\n)(?! )[^\n]+\n\n(?!\s*$)/
          // for discount behavior.
          loose = next || /\n\n(?!\s*$)/.test(item);
          if (i !== l - 1) {
            next = item.charAt(item.length - 1) === '\n';
            if (!loose) loose = next;
          }

          this.tokens.push({
            type: loose
              ? 'loose_item_start'
              : 'list_item_start'
          });

          // Recurse.
          this.token(item, false);

          this.tokens.push({
            type: 'list_item_end'
          });
        }

        this.tokens.push({
          type: 'list_end'
        });

        continue;
      }

      // top-level paragraph
      if (top && (cap = this.rules.paragraph.exec(src))) {
        src = src.substring(cap[0].length);
        this.tokens.push({
          type: 'paragraph',
          text: cap[1].charAt(cap[1].length - 1) === '\n'
            ? cap[1].slice(0, -1)
            : cap[1]
        });
        continue;
      }

      // text
      if (cap = this.rules.text.exec(src)) {
        // Top-level should never reach here.
        src = src.substring(cap[0].length);
        this.tokens.push({
          type: 'text',
          text: cap[0]
        });
        continue;
      }

      if (src) {
        throw new
          Error('Infinite loop on byte: ' + src.charCodeAt(0));
      }
    }

    return this.tokens;
  };

  /**
   * Inline-Level Grammar
   */

  var inline = {
    escape: /^\\([\\`*{}\[\]()#+\-.!_>])/,
    autolink: /^<([^ >]+(@|:\/)[^ >]+)>/,
    url: noop,
    tag: /^<!--[\s\S]*?-->|^<\/?\w+(?:"[^"]*"|'[^']*'|[^'">])*?>/,
    link: /^!?\[(inside)\]\(href\)/,
    reflink: /^!?\[(inside)\]\s*\[([^\]]*)\]/,
    nolink: /^!?\[((?:\[[^\]]*\]|[^\[\]])*)\]/,
    strong: /^\*\*([\s\S]+?)\*\*(?!\*)/,
    underline: /^__([\s\S]+?)__(?!_)/,
    em: /^\b_((?:__|[\s\S])+?)_\b|^\*((?:\*\*|[\s\S])+?)\*(?!\*)/,
    code: /^(`+)\s*([\s\S]*?[^`])\s*\1(?!`)/,
    br: /^ {2,}\n(?!\s*$)/,
    del: noop,
    footnote: noop,
    text: /^[\s\S]+?(?=[\\<!\[_*`]| {2,}\n|$)/
  };

  inline._inside = /(?:\[[^^\]]*\]|[^\[\]]|\](?=[^\[]*\]))*/;
  inline._href = /\s*<?([\s\S]*?)>?(?:\s+['"]([\s\S]*?)['"])?\s*/;

  inline.link = replace(inline.link)
    ('inside', inline._inside)
    ('href', inline._href)
    ();

  inline.reflink = replace(inline.reflink)
    ('inside', inline._inside)
    ();

  /**
   * Normal Inline Grammar
   */

  inline.normal = merge({}, inline);

  /**
   * Pedantic Inline Grammar
   */

  inline.pedantic = {
    strong: /^__(?=\S)([\s\S]*?\S)__(?!_)|^\*\*(?=\S)([\s\S]*?\S)\*\*(?!\*)/,
    em: /^_(?=\S)([\s\S]*?\S)_(?!_)|^\*(?=\S)([\s\S]*?\S)\*(?!\*)/
  };

  /**
   * GFM Inline Grammar
   */

  inline.gfm = merge({}, inline.normal, {
    escape: replace(inline.escape)('])', '~|])')(),
    url: /^(https?:\/\/[^\s<]+[^<.,:;"')\]\s])/,
    del: /^~~(?=\S)([\s\S]*?\S)~~/,
    text: replace(inline.text)
      (']|', '~]|')
      ('|', '|https?://|')
      ()
  });

  /**
   * GFM + Line Breaks Inline Grammar
   */

  inline.breaks = merge({}, inline.gfm, {
    br: replace(inline.br)('{2,}', '*')(),
    text: replace(inline.gfm.text)('{2,}', '*')()
  });

  /**
   * Footnote Inline Grammar
   */

  inline.footnote = {
    footnote: /^\[\^([^\]]+)\]/
  };

  /**
   * Inline Lexer & Compiler
   */

  function InlineLexer(links, footnotes, options) {
    this.options = options || marked.defaults;
    this.links = links;
    this.footnotes = footnotes || {};
    this.rules = inline.normal;
    this.renderer = this.options.renderer || new Renderer(this.options);

    if (!this.links) {
      throw new
        Error('Tokens array requires a `links` property.');
    }

    if (this.options.gfm) {
      if (this.options.breaks) {
        this.rules = inline.breaks;
      } else {
        this.rules = inline.gfm;
      }
    }

    if (this.options.footnotes) {
      this.rules = merge({}, this.rules, inline.footnote);
    }

    if (this.options.pedantic) {
      this.rules = merge({}, this.rules, inline.pedantic);
    }
  }

  /**
   * Expose Inline Rules
   */

  InlineLexer.rules = inline;

  /**
   * Static Lexing/Compiling Method
   */

  InlineLexer.output = function(src, links, footnotes, options) {
    var inline = new InlineLexer(links, footnotes, options);
    return inline.output(src);
  };

  /**
   * Lexing/Compiling
   */

  InlineLexer.prototype.output = function(src) {
    var out = []
      , cap;

    while (src) {
      // link and image
      if (cap = this.rules.link.exec(src)) {
        src = src.substring(cap[0].length);
        out.push(this.outputLink(cap, {
          href: cap[2],
          title: cap[3]
        }));
        continue;
      }
      
      if(cap = urlDetectRegex.exec(src)){
        if (cap['index'] === 0) {
          src = src.substring(cap[0].length);
          out.push(this.outputLink(cap, {
            href: cap[0],
            title: cap[0]
          }, true));
          continue;
        }
      }

      // underline
      if (cap = this.rules.underline.exec(src)) {
        src = src.substring(cap[0].length);
        out.push(this.renderer.underline(cap[2] || cap[1]));
        continue;
      }

      // strong
      if (cap = this.rules.strong.exec(src)) {
        src = src.substring(cap[0].length);
        out.push(this.renderer.strong(cap[2] || cap[1]));
        continue;
      }

      // em
      if (cap = this.rules.em.exec(src)) {
        src = src.substring(cap[0].length);
        out.push(this.renderer.em(cap[2] || cap[1]));
        continue;
      }

      // code
      if (cap = this.rules.code.exec(src)) {
        src = src.substring(cap[0].length);
        out.push(this.renderer.codespan(cap[2]));
        continue;
      }

      // text
      if (cap = this.rules.text.exec(src)) {
        var matchedStr = cap[0];
        src = src.substring(matchedStr.length);
        out.push(this.renderer.text(matchedStr.replace(/[\t\n\r]+/g, ' ')));
        continue;
      }

      if (src) {
        throw new
          Error('Infinite loop on byte: ' + src.charCodeAt(0));
      }
    }

    return out;
  };

  /**
   * Compile Link
   */

  InlineLexer.prototype.outputLink = function(cap, link, nest) {
    var href = link.href, title = link.title ? link.title : null;
    var ext = href.toLowerCase().split('.').pop();
    if(cap[0].charAt(0) === '!') {
      return this.renderer.image(href, title, cap[1])
    } else {
      if(nest && (ext === 'md' || ext === 'txt' || ext === 'html')){
        return this.renderer.image(href, title, cap[1])
      }
      return this.renderer.link(href, title, cap[1])
    }
  };

  /**
   * Mangle Links
   */

  InlineLexer.prototype.mangle = function(text) {
    var out = ''
      , l = text.length
      , i = 0
      , ch;

    for (; i < l; i++) {
      ch = text.charCodeAt(i);
      if (Math.random() > 0.5) {
        ch = 'x' + ch.toString(16);
      }
      out += '&#' + ch + ';';
    }

    return out;
  };

  /**
   * Renderer
   */

  function Renderer(options) {
    this.options = options || {};

    this.onResizeListeners = [];
  }

  Renderer.prototype.code = function(code, offsetLeft) {
    var options = this.options;
    var scene = options.scene;

    var container = scene.create({
      t: 'object',
      parent: options.parent,
    });

    var decor = scene.create({
      t: 'rect',
      parent: container,
      fillColor: options.styles.code.fillColor,
      lineColor: options.styles.code.lineColor,
      lineWidth: options.styles.code.lineWidth,
    });

    var textBox = scene.create({
      t: 'textBox',
      parent: container,
      text: code,
      x: options.styles.code.paddingLeft || 0,
      y: options.styles.code.paddingTop || 0,
      wordWrap: true,
      font: options.styles.code.font,
      textColor: options.styles.code.textColor,
    });

    function updateSize() {
      container.w = options.parent.w - offsetLeft;
      decor.w = container.w;
      textBox.w = decor.w
        - (options.styles.code.paddingLeft || 0)
        - (options.styles.code.paddingRight || 0);

      var textMeasure = textBox.measureText();

      textBox.h = textMeasure.bounds.y2;
      decor.h = textBox.h
        + (options.styles.code.paddingTop || 0)
        + (options.styles.code.paddingBottom || 0);
      container.h = decor.h + (options.styles.code.marginBottom || 0);
    }

    options.emitter.on('onContainerResize',updateSize)
  
    updateSize();

    return container;
  };

  Renderer.prototype.blockquote = function(blocks, offsetLeft) {
    var options = this.options
    var scene = options.scene

    var container = scene.create({
      t: 'object',
      parent: options.parent,
    });

    var decor = scene.create({
      t: 'rect',
      parent: container,
      fillColor: options.styles.blockquote.lineColor,
      x: options.styles.blockquote.lineOffsetLeft,
      w: options.styles.blockquote.lineWidth,
    });

    blocks.forEach((block) => {
      block.parent = container;
      block.x = options.styles.blockquote.paddingLeft;
    });

    function updateSize() {
      container.w = options.parent.w - offsetLeft;

      var y = 0;
      blocks.forEach((block) => {
        block.parent = container;
        block.y = y;
        y += block.h;
      });

      container.h = y + (options.styles.blockquote.marginBottom || 0);
      decor.h = y;
    }

    this.options.emitter.on('onContainerResize', updateSize)
    updateSize();

    return container;
  };

  Renderer.prototype.heading = function(inlineBlocks, level, offsetLeft) {
    return this.renderTextBlockWithStyle(inlineBlocks, this.options.styles['header-' + level], offsetLeft);
  };

  Renderer.prototype.list = function(listItems, ordered, offsetLeft) {
    var options = this.options
    var scene = options.scene

    var container = scene.create({
      t: 'object',
      parent: options.parent,
    });

    var markers = [];

    listItems.forEach((listItem, index) => {
      var marker = null;
      if (ordered) {
        marker = this.renderInlineTextWithStyle(`${index + 1}.`, options.styles['list-item']);
      } else {
        marker = scene.create({
          t: 'image',
          url: options.mimeBaseURL+'res/unordered_list_fill.svg',
          x: 0,
          y: 0,
          h: options.styles['list-item'].pixelSize,
        })
      }
      

      marker.parent = container;
      marker.x = options.styles['list-item'].symbolOffsetLeft;

      markers.push(marker);

      listItem.parent = container;
      listItem.x = options.styles['list-item'].paddingLeft;
    });

    function updateSize() {
      container.w = options.parent.w - offsetLeft;

      var y = 0

      listItems.forEach((listItem, index) => {
        var listItemContainer = listItem.children && listItem.children[0];
        var listItemInlineBlock = listItemContainer && listItemContainer.children && listItemContainer.children[0];
        var listItemHeight = listItemInlineBlock
          ? (listItemInlineBlock.h + listItemInlineBlock.y)
          : listItem.h;

        markers[index].y = y + (listItemHeight - markers[index].h);
        listItem.y = y;

        y += listItem.h;
      });

      container.h = y
        + (options.styles.list.marginBottom || 0)
        - (options.styles['list-item'].marginBottom || 0);
    }

    options.emitter.on('onContainerResize',updateSize)
    updateSize();

    return container;
  };

  Renderer.prototype.listitem = function(content, offsetLeft) {
    var options = this.options;
    var scene = this.options.scene;

    var container = scene.create({
      t: "object",
      parent: options.parent,
      x: 0,
      y: 0,
    });

    var items = [];

    content.forEach((contentItem) => {
      if (Array.isArray(contentItem)) {
        var listItem = this.renderTextBlockWithStyle(
          contentItem,
          options.styles['list-item'],
          offsetLeft + options.styles['list-item'].paddingLeft
        );
        listItem.parent = container;

        items.push(listItem);
      } else if (contentItem !== null) {
        contentItem.parent = container;
        items.push(contentItem);
      }
    });

    function updateSize() {
      container.w = options.parent.w - offsetLeft;

      var y = 0;
      items.forEach((item) => {
        item.y = y;
        y += item.h;
      })

      container.h = y;
    }

    this.options.emitter.on('onContainerResize',updateSize)
    updateSize();

    return container;
  };

  Renderer.prototype.renderTextBlockWithStyle = function(inlineBlocks, style, offsetLeft) {
    var scene = this.options.scene;
    var options = this.options

    var container = scene.create({
      t: "object",
      parent: options.parent,
      interactive: false,
      x: 0,
      y: 0,
      w: options.parent.w - offsetLeft,
    });

    function resolveFont(blockFont, inlineFont) {
      if (inlineFont === options.FONT_STYLE.BOLD && blockFont === options.FONT_STYLE.ITAlIC ||
        inlineFont === options.FONT_STYLE.ITALIC && blockFont === options.FONT_STYLE.BOLD
      ) {
        return options.FONT_STYLE.BOLD_ITALIC
      }

      if (inlineFont === options.FONT_STYLE.REGULAR) {
        return blockFont;
      }

      return inlineFont ? inlineFont : blockFont;
    }

    function copy(inlineBlock) {
      // for falsy values just return the same value
      if (!inlineBlock) {
        return inlineBlock;
      }

      var type = inlineBlock.type;
      var textColor = style.textColor;
      if( type === 'link' ){
        textColor = options.styles.link.textColor;
      }

      var inlineBlockCopy = scene.create({
        id: inlineBlock.id,
        t: 'text',
        interactive: false,
        x: inlineBlock.x,
        y: inlineBlock.y,
        text: inlineBlock.text,
        textColor: textColor,
        font: resolveFont(style.font, inlineBlock.font),
        pixelSize: style.pixelSize,
      });
      inlineBlockCopy.type = inlineBlock.type;

      if(type === 'link') {
        inlineBlockCopy.onClick = inlineBlock.onClick;
        var clickObj = scene.create({ t: "object", parent: inlineBlockCopy, x: 0, y: 0, w: inlineBlockCopy.w, h: inlineBlockCopy.h});
        clickObj.on('onMouseUp',function() {
          inlineBlockCopy.onClick();
        });
        clickObj.on('onMouseEnter', function(){
          inlineBlockCopy.textColor = options.styles.link.activeColor;
        });
        clickObj.on('onMouseLeave', function(){
          inlineBlockCopy.textColor = options.styles.link.textColor;
        });
      }
      return inlineBlockCopy;
    }

    function renderInlineBlocks() {
      container.removeAll();

      var x = 0;
      var y = 0;

      var inlineBlock;
      var someBlock;
      var blocksToRender = inlineBlocks.slice();
      var lineBlocks = [];

      function getLineHeight() {
        var heights = lineBlocks.map((block) =>  block.h);
        var maxHeight = Math.max(Math.max.apply(null, heights), 0);

        return maxHeight;
      }

      function updateLineBlocksHeights(lineHeight) {
        lineBlocks.forEach((block) => {
          var lineHeightDiff = lineHeight - block.h;
          block.y = block.y + lineHeightDiff;
        });
      }

      function newLine() {
        var lineHeight = getLineHeight();

        updateLineBlocksHeights(lineHeight);
        lineBlocks = [];

        x = 0;
        y += lineHeight;
      }

      while (someBlock = blocksToRender.shift()) {
        if (typeof someBlock.text === 'undefined') {
          if (x + someBlock.w > container.w && x !== 0) {
            newLine();
            // put block back to the list, to draw on the new line
            blocksToRender.unshift(someBlock);
          } else {
            someBlock.x = x;
            someBlock.y = y;
            someBlock.parent = container;

            x += someBlock.w;
            lineBlocks.push(someBlock);
          }
          continue;
        }

        var inlineBlock = copy(someBlock);

        var currentBlockWords = inlineBlock.text.split(' ');
        var newBlockWords = [];


        // if a word length greater than conatiner.w, and cannot split by space
        if (currentBlockWords.length <= 1 && x + inlineBlock.w > container.w) {
          var newWord = '';
          while (x + inlineBlock.w > container.w) {
            newWord = inlineBlock.text.substring(inlineBlock.text.length-1) + newWord;
            inlineBlock.text = inlineBlock.text.substring(0,inlineBlock.text.length-1);
          }
          newBlockWords = [newWord];
        }

        while (x + inlineBlock.w > container.w && currentBlockWords.length > 0) {
          newBlockWords.unshift(currentBlockWords.pop());

          inlineBlock.text = currentBlockWords.join(' ');
        }

        // if even one word cannot be rendered on the new line, then render it anyway
        if (currentBlockWords.length === 0 && x === 0) {
          inlineBlock.text = newBlockWords.shift();
        }

        // render block
        inlineBlock.x = x;
        inlineBlock.y = y;
        inlineBlock.parent = container;
        lineBlocks.push(inlineBlock);

        if(inlineBlock.type === 'underline') { // draw a under line
          scene.create({
            t: 'rect',
            h: options.styles.underline.height,
            fillColor: options.styles.underline.fillColor,
            w: inlineBlock.w,
            parent: inlineBlock,
            x:0,
            y: inlineBlock.h - 1,
          })
        }
        // create same style block with the words which don't fit the line
        if (newBlockWords.length > 0) {
          var newInlineBlock = copy(inlineBlock);

          newInlineBlock.text = newBlockWords.join(' ');

          blocksToRender.unshift(newInlineBlock);

          newLine();
        } else {
          x += inlineBlock.w;

          if (x > container.w) {
            newLine();
          }
        }
      }

      var lastLineHeight = getLineHeight();
      updateLineBlocksHeights(lastLineHeight);

      container.h = y
        + lastLineHeight
        + (style.marginBottom || 0);
    }

    this.options.emitter.on('onContainerResize', function() {
      container.w = options.parent.w - offsetLeft;

      renderInlineBlocks();
    });
    
    renderInlineBlocks();

    return container;
  }

  Renderer.prototype.paragraph = function(inlineBlocks, offsetLeft) {
    return this.renderTextBlockWithStyle(inlineBlocks, this.options.styles.paragraph, offsetLeft);
  };

  Renderer.prototype.renderInlineTextWithStyle = function(text, style) {
    var scene = this.options.scene;

    var textInline = scene.create({
      t: 'text',
      interactive: false,
      text: text,
      font: this.options.mimeBaseURL+style.font,
      textColor: style.textColor,
    });

    return textInline;
  }

  // span level renderer
  Renderer.prototype.strong = function(text) {
    return this.renderInlineTextWithStyle(text, this.options.styles.strong);
  };
  
  Renderer.prototype.underline = function(text) {
    var underlineNode = this.renderInlineTextWithStyle(text, this.options.styles.underline);
    underlineNode.type = 'underline';
    return underlineNode;
  };

  Renderer.prototype.em = function(text) {
    return this.renderInlineTextWithStyle(text, this.options.styles.em);
  };

  Renderer.prototype.text = function(text) {
    return this.renderInlineTextWithStyle(text, this.options.styles.text);
  };

  Renderer.prototype.codespan = function(text) {
    return this.renderInlineTextWithStyle(text, this.options.styles.codespan);
  };

  Renderer.prototype.link = function(href, title, text) {
    var options = this.options;
    var link = this.renderInlineTextWithStyle(text || title, this.options.styles.link);
    link.type = 'link';

    var url = href
    /*
    if (!href.match(/^(?:file|https?|ftp):\/\//)) {
      url = options.basePath + href;
    }
    */
   
    if (!href.match(/^.*:/)) {
      url = options.basePath + href;
    }
    

    link.onClick = function(){
    var scene = options.scene;

    // Send navigate request via bubbling service manager up
    // the chain.
    var n = scene.getService(".navigate");
    if (n) {
      console.log("before navigation request");
      n.setUrl(url);
    }
    else console.log(".navigate service not available");

    }
    return link;
  };

  // This will handle multiple document types
  // Allowing for nested Images, Markdown, Text and Spark Content
  Renderer.prototype.image = function(href, title, text) {
    var scene = this.options.scene;
    var options = this.options;

    var url = href;
    if (!href.match(/^(?:file|https?|ftp):\/\//)) {
      url = options.basePath + href;
    }

    var fontMetrics = options.styles.paragraph.font.getFontMetrics(
      options.styles.paragraph.pixelSize
    );

    let hasWxH = (href.indexOf(' =')  >= 0);
    var WxH = [];

    if(hasWxH)
    {
      var splitz = url.split(' =');

      url = splitz[0].trim();
      WxH = splitz.pop();
      WxH = WxH.split('x');
    }

    // Init
    // Use a 16:9 default aspect ratio
    var ww = (WxH.length > 0 && WxH[0] != '') ? parseInt(WxH[0]) : 528
    var hh = (WxH.length > 1 && WxH[1] != '') ? parseInt(WxH[1]) : 297

    /*
    console.log("#############  MD::Image >>>>  WxH = " + ww + " x " + hh); // JUNK
    console.log("#############  MD::Image >>>>  url = " + url);  // JUNK
    console.log("#############  MD::Image >>>> href = " + href); // JUNK
    */

    var img = scene.create({t:'scene',url:url,w:ww,h:hh,parent:this.options.parent,clip:true})

    
    function updateSize() {
      /*
      if (!img.resource) {
      //img.w = 800
      //img.h = 600
      return;
      }
      if(img.resource.w <= 0 || img.resource.h <= 0){ // w or h is 0, skip this
        return;
      }
      img.maxWidth = options.parent.w;
      var w = Math.min(img.resource.w, options.parent.w);
      var ar = img.resource.w / img.resource.h;

      img.w = w;
      img.h = w / ar;
      */
    }

    if (!hasWxH) {
      img.ready.then(() => {
        if (img.api._ready) {
          img.api._ready.then(function(){
            img.w = img.api._preferredW
            img.h = img.api._preferredH
          })
        }

        this.options.emitter.emit('onImageReady')
      })
    }

    return img;
  };

  Renderer.prototype.emptyLine = function(offsetLeft) {
    var inlineBlock = this.renderInlineTextWithStyle('', this.options.styles.text);

    return this.renderTextBlockWithStyle([inlineBlock], this.options.styles.paragraph, offsetLeft);
  }

  /**
   * Parsing & Compiling
   */

  function Parser(options) {
    this.tokens = [];
    this.token = null;
    this.options = options || marked.defaults;
    this.options.renderer = this.options.renderer || new Renderer(this.options);
    this.renderer = this.options.renderer;
  }

  /**
   * Static Parse Method
   */

  Parser.parse = function(src, options, renderer) {
    var parser = new Parser(options, renderer);
    return parser.parse(src);
  };

  /**
   * Parse Loop
   */

  Parser.prototype.parse = function(src) {
    var options = this.options;
    this.inline = new InlineLexer(src.links, src.footnotes, this.options);
    this.tokens = src.reverse();

    var out = [];
    while (this.next()) {
      var block = this.tok();

      out.push(block);
    }

    function updateSize() {
      var y = 0;

      out.forEach(function(block) {
        // skip space blocks (null)
        if (!block) return;
        block.y = y;
        y += block.h;
      });

      options.parent.h = y;
    }

    this.options.emitter.on('onContainerResize',updateSize)
    updateSize();

    if (src.footnotes.length) {
      out.push(this.renderer.footnotes(src.footnotes));
    }

    return out;
  };

  /**
   * Next Token
   */

  Parser.prototype.next = function() {
    return this.token = this.tokens.pop();
  };

  /**
   * Preview Next Token
   */

  Parser.prototype.peek = function() {
    return this.tokens[this.tokens.length - 1] || 0;
  };

  /**
   * Parse Text Tokens
   */

  Parser.prototype.parseText = function() {
    var body = this.token.text;

    while (this.peek().type === 'text') {
      body += '\n' + this.next().text;
    }

    return this.inline.output(body);
  };

  /**
   * Parse Current Token
   */

  Parser.prototype.tok = function(offsetLeft = 0) {
    var options = this.options;

    switch (this.token.type) {
      case 'space': {
        return null;
      }
      case 'heading': {
        return this.renderer.heading(
          this.inline.output(this.token.text),
          this.token.depth,
          offsetLeft
        );
      }
      case 'code': {
        return this.renderer.code(this.token.text, offsetLeft);
      }
      case 'blockquote_start': {
        var body = [];

        while (this.next().type !== 'blockquote_end') {
          body.push(this.tok(offsetLeft + (options.styles.blockquote.paddingLeft || 0)));
        }

        return this.renderer.blockquote(body, offsetLeft);
      }
      case 'list_start': {
        var body = []
          , ordered = this.token.ordered;

        while (this.next().type !== 'list_end') {
          body.push(this.tok(offsetLeft + (options.styles.list.paddingLeft || 0)));
        }

        return this.renderer.list(body, ordered, offsetLeft);
      }
      case 'list_item_start': {
        var body = [];

        while (this.next().type !== 'list_item_end') {
          var tok = this.token.type === 'text'
            ? this.parseText(offsetLeft + (options.styles['list-item'].paddingLeft || 0))
            : this.tok(offsetLeft + (options.styles['list-item'].paddingLeft || 0));

          body.push(tok);
        }

        return this.renderer.listitem(body, offsetLeft);
      }
      case 'loose_item_start': {
        var body = [];

        while (this.next().type !== 'list_item_end') {
          body.push(this.tok(offsetLeft + (options.styles['list-item'].paddingLeft || 0)));
        }

        return this.renderer.listitem(body, offsetLeft);
      }
      case 'paragraph': {
        return this.renderer.paragraph(this.inline.output(this.token.text), offsetLeft);
      }
      case 'text': {
        return this.renderer.paragraph(this.parseText(), offsetLeft);
      }
    }
  };

  /**
   * Helpers
   */
  function replace(regex, opt) {
    regex = regex.source;
    opt = opt || '';
    return function self(name, val) {
      if (!name) return new RegExp(regex, opt);
      val = val.source || val;
      val = val.replace(/(^|[^\[])\^/g, '$1');
      regex = regex.replace(name, val);
      return self;
    };
  }

  function noop() {}
  noop.exec = noop;

  function merge(obj) {
    var i = 1
      , target
      , key;

    for (; i < arguments.length; i++) {
      target = arguments[i];
      for (key in target) {
        if (Object.prototype.hasOwnProperty.call(target, key)) {
          obj[key] = target[key];
        }
      }
    }

    return obj;
  }

  var defaults = {
    gfm: true,
    tables: true,
    footnotes: false,
    breaks: false,
    pedantic: false,
    sanitize: false,
    smartLists: false,
    silent: false,
    highlight: null,
    langPrefix: 'lang-',
    headerPrefix: '',
    renderer: null,
    xhtml: false
  };

  function Markdown(scene, parent, options) {
    this.scene = scene;
    this.parent = parent;
    this.options = options;

    this.source;

    this.emitter = new _eventEmitter()

    this.prepareStyle(style, this.options.mimeURL || '');
    
    this.container = scene.create({
      t: 'object',
      x: this.options.styles.container.paddingLeft || 0,
      y: this.options.styles.container.paddingTop || 0,
      parent: this.getParentRoot(),
      interactive: false,
      w: parent.root.w
        - (this.options.styles.container.paddingLeft || 0)
        - (this.options.styles.container.paddingRight || 0),
      h: parent.root.h
        - (this.options.styles.container.paddingTop || 0)
        - (this.options.styles.container.paddingBottom || 0),
    });

    this.update = this.update.bind(this);

    //JRJR why do we need to do this?
    var that = this
    this.update2 = function() {
      that.update()
    }

    this.scene.on('onResize', this.update2)

    this.emitter.on('onImageReady', this.update)
  }

  Markdown.prototype.getParentRoot = function() {
    return this.parent.root ? this.parent.root : this.parent;
  }

  Markdown.prototype.prepareStyle = function(style, baseURL) {
    this.options.FONT_STYLE = {};
    Object.keys(style.FONT_STYLE).forEach((fontStyle) => {

      this.options.FONT_STYLE[fontStyle] = this.scene.create({
        t: 'fontResource',
        url: baseURL + style.FONT_STYLE[fontStyle],
      });
    });

    this.options.styles = {};
    Object.keys(style.styles).forEach((blockName) => {
      var blockStyle = Object.assign({}, style.styles[blockName]);

      if (blockStyle.font) {
        blockStyle.font = this.options.FONT_STYLE[blockStyle.font];
      }

      this.options.styles[blockName] = blockStyle;
    });
  }

  Markdown.prototype.setSource = function(markdownSource) {
    this.source = markdownSource;
    this.render();
  }

  Markdown.prototype.update = function() {
    var parentRoot = this.getParentRoot();

    this.container.w = parentRoot.w
      - (this.options.styles.container.paddingLeft || 0)
      - (this.options.styles.container.paddingRight || 0);

    this.emitter.emit('onContainerResize')

    this.updateParent();
  }

  Markdown.prototype.render = function() {
    var opt = merge({}, defaults, {
      basePath: this.options.basePath,
      scene: this.scene,
      parent: this.container,
      emitter: this.emitter,
      FONT_STYLE: this.options.FONT_STYLE,
      styles: this.options.styles,
      mimeBaseURL: this.options.mimeURL || '',
    });

    var tokens = Lexer.lex(this.source, opt);

    Parser.parse(tokens, opt);

    this.updateParent();
  }

  Markdown.prototype.updateParent = function() {
    this.getParentRoot().h = this.container.h
        + (this.options.styles.container.paddingTop || 0)
        + (this.options.styles.container.paddingBottom || 0);

    this.parent.update && this.parent.update();
  }

  Markdown.prototype.prepare = function(){

    var fontsResources = [];
    var keys = Object.keys(this.options.FONT_STYLE);
    var scene = this.scene;
    keys.forEach((fontStyle) => {
      fontsResources.push(scene.create({ t: 'fontResource', url: this.options.FONT_STYLE[fontStyle].url}).ready)
    });
    var that = this;

    // clear old node
    var children = scene.root.children;
    for( var i = 0 ; i < children.length; i ++){
      if( children[i].markAsDelete === true){
        children[i].remove()
      }
    }

    return Promise.all(fontsResources).then(function (resources) {
      for(var i = 0 ; i < keys.length ; i ++){
        that.options.FONT_STYLE[keys[i]] = resources[i];
        console.log("font " + resources[i].url + " loaded.")
      }
      return Promise.resolve(that);
    });
  }
  module.exports.Markdown = Markdown;

}).catch(function importFailed(err) {
  console.error("Import failed: " + err);
});
