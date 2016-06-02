/**
 * FileName    : xretextresource.js
 * Created     : 06/18/2015
 * Description : Defines class to handle text resource
 **/

var Constants = require("../utils/config.js");
var Common = require("../utils/common.js");
var TEXT_TRUNC_STYLE = require("../utils/enum.js").TEXT_TRUNC_STYLE;
var HORIZONTAL_ALIGN_STYLE = require("../utils/enum.js").HORIZONTAL_ALIGN_STYLE;
var VERTICAL_ALIGN_STYLE = require("../utils/enum.js").VERTICAL_ALIGN_STYLE;
var onTextMetadataEvent = require("../events/resourceEvent.js").COnTextMetadataEvent;
var XREResource = require("./xreresource.js");
var XRElogger = require('../utils/xrelogger.js');
var FontResource = require('./xrefontresource.js');
var defaultLogger = XRElogger.getLogger("default");
/*
 *  Class TextResource
 *  Creates a text object and set properties
 **/
var TextResource = function(id, app, params, scene) {
    //Calling parent constructor
    XREResource.call(this, id, app);

    var textItems = [];
    //Setting default values 
    var textColor = 0x000000FF;
    var pixelSize = 12;
    var leading = 0;
    var blinkingCursor = false;
    var text = "";
    var font = null;
    var faceURL = Common.getDefaultFontURL();

    /** Set the param values to class properties **/
    this.setProperties = function(params) {

        if (params.hasOwnProperty("text")) {
            // TODO Check for extended chars and set a flag
            if (params.text != null) {
                text = params.text;
            }
        }
        if (params.hasOwnProperty("font")) {
            font = app.getResource(params.font);
            if (font instanceof FontResource) {
                faceURL = font.getFontUrl();

            }
            defaultLogger.log("debug", "printing font");
            defaultLogger.log("debug", faceURL);
            defaultLogger.log("debug", "*************************");
        }
        if (params.hasOwnProperty("color")) {
            textColor = Common.processColor(params.color);
        }
        if (params.hasOwnProperty("size")) {
            pixelSize = params.size;
        }
        if (params.hasOwnProperty("leading") && params.leading) {
            defaultLogger.log("debug", "Leading--------");
            leading = params.leading;
        }
        if (params.hasOwnProperty("blinkingCursor")) {
            blinkingCursor = params.blinkingCursor;
        }
        //Updating all resource items
        if (textItems.length > 0) {
            for (var idx = 0; idx < textItems.length; idx++) {
                textItems[idx].updateResource();
            }
        }
    };

    /* Function to create pxTextItem object */
    this.createPXItem = function(view) {
        var item = {};
        clog("createPXItem");
        if(textItems.length === 0 || textItems[textItems.length - 1].getParentView()){
            clog("Creating new textItem");
            item = new PXTextItem(view, id, app, scene, this);
            textItems.push(item);
        } else {
            clog("Text item already present");
            item = textItems[textItems.length - 1];
            textItems[textItems.length - 1].setParentView(view);
            textItems[textItems.length - 1].updateResource();
        }
        return item;
    };

    /* Method to get color */
    this.getColor = function() {
        return textColor;
    };

    /* Method to get size */
    this.getSize = function() {
        return pixelSize;
    };

    /* Method to get leading */
    this.getLeading = function() {
        return leading;
    };

    /* Method to get faceurl */
    this.getFaceURL = function() {
        return faceURL;
    };

    /* Method to get text */
    this.getText = function() {
        return text;
    };

    /* Method to get font resource */
    this.getFont = function() {
        return font;
    };

    /**
     *   Function get numeric value of a property by its name
     **/
    this.getNumericPropertyByName = function(property) {
        //clog("textItems");
        //clog(textItems);
        var textObj = textItems[0].getSceneObject();
        clog("getNumericPropertyByName");
        if (!faceURL) {
            clog("error getNumericPropertyByName in text not got font");
            return 0.0;
        }
        clog("Calling getFontMetrics");
        var fm = textObj.getFontMetrics();
        switch (property) {
            case "documentMargin":
                //TODO
                // return ::DOCUMENT_MARGIN;
                return 1;
            case "lineHeight":
                return fm.height;
            case "lineSpacing":
                //Returning sum of height and leading
                clog("lineSpacing");
                clog(fm.height + textObj.leading);
                return (fm.height + textObj.leading);
            case "leading":
                return textObj.leading;
            case "naturalLeading":
                //TODO
                clog("Natural leading");
                clog(fm.naturalLeading);
                return fm.naturalLeading;
            case "ascent":
                return fm.ascent;
            case "descent":
                return fm.descent;
            default:
                defaultLogger.log("debug", "Error : Property " + property + "not available");
                return 0.0;

        }
    };
    //Setting parameters to text object
    this.setProperties(params);
    var item = new PXTextItem(null, id, app, scene, this);
    textItems.push(item);
};
/** 
 *   Class to handle text scene object and asign properties to the object 
 **/
var PXTextItem = function(view, id, app, scene, res) {
    var parentView = null;
    var resource = res;

    var textObj = scene.create({
        t: "text2"
    });
   

    /** Method to set resource object with required properties  */

    this.updateResource = function() {
        defaultLogger.log("debug", "updateResource");
        if (textObj) {

            textObj.textColor = res.getColor();
            textObj.pixelSize = res.getSize();
            textObj.leading = res.getLeading();
            textObj.faceURL = res.getFaceURL();
            //clog("text color");
            //clog(res.getColor());
            //clog("text font");
            //clog(textObj.faceURL);
            if (parentView) {
                clog("Has parentView");
                var options = parentView.getResourceOptions();
                var targetText = res.getText();
                //clog("options- startChar");
                //clog(options.textStartChar);
                if (options.textStopChar !== -1 && options.textStopChar !== -1 && options.textStartChar !== undefined && options.textStopChar !== undefined) {
                    targetText = targetText.substring(options.textStartChar, options.textStopChar - options.textStartChar + 1);
                } else {
                    targetText = targetText.substring(options.textStartChar);
                }
                textObj.text = targetText;
                //clog(textObj.text);
                if (options.textStartPos) {
                    clog("Setting StartPos");
                    textObj.xStartPos = options.textStartPos[0];
                }
                if (options.textStartPos) {
                    textObj.xStopPos = options.textStopPos[0];
                    clog("Setting text stop position : ");
                    clog(textObj.xStopPos);
                }
                /* Not checking width of view and text resource before doing wrap and truncate */
                if (options.textWrap === "WRAP") {
                    textObj.wordWrap = true;
                } else {
                    textObj.wordWrap = false;
                }

            switch (options.textTruncStyle) {
                case TEXT_TRUNC_STYLE.NONE:
                    textObj.truncation = 0;
                    break;
                case TEXT_TRUNC_STYLE.ELLIPSIS:
                    textObj.truncation = 1;
                    break;
                case TEXT_TRUNC_STYLE.ELLIPSIS_AT_WORD_BOUNDARY:
                    textObj.truncation = 2;
                    break;
                default:
                    break;
            }
            if (options.textTruncStyle !== TEXT_TRUNC_STYLE.NONE) {
                textObj.ellipsis = true;
            }
            textObj.verticalAlign = VERTICAL_ALIGN_STYLE[options.verticalAlign];
            textObj.horizontalAlign = HORIZONTAL_ALIGN_STYLE[options.horizontalAlign];
            var fontMetrics = textObj.getFontMetrics();
            var viewObj = parentView.getViewObj();
            // TODO Need to confirm this setting

            //textObj.x = viewObj.x;
            //textObj.y = viewObj.y;
            //textObj.useMatrix = true;
            //textObj.m41 = viewObj.m41;
            //textObj.m42 = viewObj.m42;
            textObj.w = viewObj.w;
            textObj.h = viewObj.h;

            var textMeasurement = textObj.measureText();

            var width = textMeasurement.bounds.x2 - textMeasurement.bounds.x1;
            var height = textMeasurement.bounds.y2 - textMeasurement.bounds.y1;
            // TO CHECK Core dump while printing text object
            //clog(textObj);
            var event = new onTextMetadataEvent(width, height, fontMetrics.ascent, fontMetrics.descent, textObj.leading);

                this.reportMetadata(event);
            }

        }
    };
    /**
     *    Function to get text object measurements
     **/
    this.getObjectRect = function() {
        var objRect = {};
        objRect.X = textObj.x;
        objRect.Y = textObj.y;
        objRect.W = textObj.w;
        objRect.H = textObj.h;
        return objRect;
    };
    /**
     *    Function to get scene text object
     **/
    this.getSceneObject = function() {
        return textObj;
    };
    /**
     *  Function to emit COnTextMetadataEvent
     **/
    this.reportMetadata = function(event) {
        //event emitter of object
        resource.getEmitter().emit('Event', event, resource);
    };
    /**
     *  Function to get bounding box x
     **/
    this.getBoundingBoxX = function() {
        var textMeasurement = textObj.measureText();
        return textMeasurement.bounds.x1;
    };
    /**
     *  Function to get bounding box y
     **/
    this.getBoundingBoxY = function() {
        var textMeasurement = textObj.measureText();
        return textMeasurement.bounds.y1;
    };
    /**
     *  Function to get bounding box width
     **/
    this.getBoundingBoxWidth = function() {
        var textMeasurement = textObj.measureText();
        var width = textMeasurement.bounds.x2 - textMeasurement.bounds.x1;
        return width;
    };
    /**
     *  Function to get bounding box height
     **/
    this.getBoundingBoxHeight = function() {
        var textMeasurement = textObj.measureText();
        var height = textMeasurement.bounds.y2 - textMeasurement.bounds.y1;
        return height;
    };
    this.measuredCharX = function(index) {
        defaultLogger.log("debug", "Method not measuredCharX implemented");
        /*if(textObj.text.length <= index){
            return 0;
        }*/

    };
    this.getWidth = function() {
         return this.getBoundingBoxWidth();
     };
     this.getHeight = function() {
         return this.getBoundingBoxHeight();
     };
    this.measuredCharY = function(index) {
        defaultLogger.log("debug", "Method not measuredCharY implemented");
    };
    this.getParentView = function(){
        return parentView;
    };
    this.setParentView =  function(view){
        parentView = view;
        resource.registerMouseEvent(resource, textObj, view);
        textObj.parent = parentView.getViewObj();
    };
    if(view){
        this.setParentView(view);
    }
    this.updateResource();
};

//Inherting XREResource

TextResource.prototype = Object.create(XREResource.prototype);
TextResource.prototype.constructor = TextResource;

module.exports = {
    TextResource: TextResource,
    PXTextItem: PXTextItem
};

/*jslint node: true */
"use strict";
