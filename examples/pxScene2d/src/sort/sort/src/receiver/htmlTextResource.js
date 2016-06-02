/**
 * FileName    : htmltextresource.js
 * Created     : 15/07/2015
 * Description : Defines class to handle HTMLtext resource
 **/

var Constants = require("../utils/config.js");
var Common = require("../utils/common.js");
var TEXT_TRUNC_STYLE = require("../utils/enum.js").TEXT_TRUNC_STYLE;
var HORIZONTAL_ALIGN_STYLE = require("../utils/enum.js").HORIZONTAL_ALIGN_STYLE;
var VERTICAL_ALIGN_STYLE = require("../utils/enum.js").VERTICAL_ALIGN_STYLE;
var FONT_MAP = require("../utils/enum.js").FONT_MAP;
var FONT_STYLE = require("../utils/enum.js").FONT_STYLE;
var HTMLLinkClickedEvent = require("../events/resourceEvent.js").HTMLLinkClickedEvent;
var XREResource = require("./xreresource.js");
var XRElogger = require('../utils/xrelogger.js');
var XREHTMLView = require('./htmlView.js');
/*
 *  Class HtmlTextResource
 *  Creates a pxscene text object and set properties
 **/
var HTMLTextResource = function(id, app, params, scene) {
    clog("Inside HTMLTextResource");
    var defaultLogger = XRElogger.getLogger("default");
    //var textObj;
    obj1 = new XREHTMLView(scene);
    var count = 0;

    //Calling parent constructor

    XREResource.call(this, id, app);
    //textObj.font = "with-serif";
    this.parentView = Constants.XRE_ID_NULL;
    clog("After ====");
    /** Set the param values to class properties and scene object **/

    this.setProperties = function(params, htmlText, style) {
        var textObj = scene.create({
            t: "text2"
        });

        clog("Inside setProperties");
        textObj.text = htmlText;
        clog("HTML TEXT");
        clog(textObj.text);
        if (params.hasOwnProperty('stylesheet')) {
            var stylesheetObj = this.GetApp().getResource(params.stylesheet);
            var param = stylesheetObj.getProperty("css");
            param = param.substring(1, param.length);
            for (i = 1; i <= 4; i++) {
                switch (i) {
                    case 1:
                        font_size = param.substring(param.indexOf(':') + 1, param.indexOf(';'));
                        textObj.pixelSize = font_size.replace('px', '');
                        break;
                    case 2:
                        family = param.substring(param.indexOf(':') + 1, param.indexOf(';'))
                        break;
                    case 3:
                        color = param.substring(param.indexOf(':') + 1, param.indexOf(';'));
                        clog("Text color===");
                        //textObj.textColor = parseInt('0x' + color.replace(' #', ''), 16);
                        //textObj.textColor=0xffffff;
                        var color = '0x00' + color.replace(' #', '');
                        clog(color);
                        break;
                    default:
                        margin_right = param.substring(param.indexOf(':') + 1, param.indexOf(';'));
                        margin_right = margin_right.replace('px', '');
                }
                param = param.substr(param.indexOf(';') + 1, param.length);
            }

            clog("before serif");
            clog(style);
            clog(family);
            if (FONT_MAP[family] != undefined && FONT_MAP[family][FONT_STYLE[style]]) {
                clog("Inside FONT====");
                textObj.faceURL = FONT_MAP[family][FONT_STYLE[style]];
            } else {
                textObj.faceURL = Common.getDefaultFontURL();
            }

        };
        if (params.hasOwnProperty("leading")) {
            defaultLogger.log("debug", "Leading--------");
            textObj.leading = params.leading;
        }

        obj1.setResourceItem(count, textObj, color, this);
        count++;
    };

    /** Returns text instance of pxscene after proper alignment */

    this.createPXItem = function(view) {
        defaultLogger.log("debug", "getResourceItem");
        clog("Inside getResourceItem======");
        var childView = obj1.getView(view);
        if (childView) {
            var viewObj = view.getViewObj();
            // TODO Need to confirm this setting
            childView.x = viewObj.x;
            childView.y = viewObj.y;
            childView.w = viewObj.w;
            childView.h = viewObj.h;
            //this.parentView = view;
            childView.parent = view.getViewObj();
            view.setResourceItem(childView);

        }
    };

    this.reportMetadata = function(event) {
        //event emitter of object
        this.getEmitter().emit('Event', event, this);
    };

    this.handleLink = function(link) {
        var event = new HTMLLinkClickedEvent(link);
        event.setHandlerID(this.GetId());
        event.setSourceID(this.GetId());
        this.getEmitter().emit('Event', event, this);
    };


    //Setting parameters to text object
    this.setProperty = function(params) {
        clog("Inside setProperty====");
        var i = 0;
        var index = 0;
        var j = 0;
        var pattern = new RegExp('</?p ?[^>]*>', 'g');
        var html = params.htmlText;
        html = html.replace(pattern, '');
        while (html.length > 0) {

            if (html.charAt(i) == '<' && html.charAt(i + 1) == 'b') {
                str = html.substr(html.indexOf('<b'), html.indexOf('</') + 4)
                data = str.replace(/<[^>]+>/g, '');
                html = html.substr(str.length, html.length);
                clog("Bold=====");
                this.setProperties(params, data, "BOLD")

            } else if (html.charAt(i) == '<' && html.charAt(i + 1) == 'i') {
                clog("inside italics======")
                str = html.substr(html.indexOf('<i'), html.indexOf('</') + 4)
                data = str.replace(/<[^>]+>/g, '');
                html = html.substr(str.length, html.length);
                clog("Italics=====");
                this.setProperties(params, data, "ITALIC")
            } else {
                if (html.match(/<[^>]+>/g)) {
                    str = html.substr(html.charAt(i), html.indexOf('<'));
                    html = html.substr(str.length, html.length);
                    this.setProperties(params, str, "NORMAL")
                } else {
                    str = html.substr(html.charAt(i), html.length);
                    html = html.substr(str.length, html.length);
                    this.setProperties(params, str, "NORMAL");
                }
            }
            index++;
        }
    };
    clog("befor setProperty====");
    this.setProperty(params);
};

//Inherting XREResource
HTMLTextResource.prototype = Object.create(XREResource.prototype);
HTMLTextResource.prototype.constructor = HTMLTextResource;

module.exports = HTMLTextResource;

/*jslint node: true */
"use strict";
