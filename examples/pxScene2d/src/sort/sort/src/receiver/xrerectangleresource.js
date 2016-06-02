/**
 * FileName    : xrerectangleresource.js
 * Created     : 06/12/2015
 * Description : Defines class to handle rectangle resource
 **/

var Constants = require("../utils/config.js");
var Common = require("../utils/common.js");
var XREResource = require("./xreresource.js");
var XRElogger = require('../utils/xrelogger.js');
var defaultLogger = XRElogger.getLogger("default");
/*
 *  Class RectangleResource
 *  Creates a rectangle object and set properties
 **/
var RectangleResource = function(id, app, params, scene) {
    //Calling parent constructor
    XREResource.call(this, id, app);

    var rectItems = [];
    //Setting default values 
    var fillColor = 0xFFFFFFFF;
    var lineColor = 0x000000FF;
    var lineWidth = 0;
    /**
     *   Function to set properties for rectangle resource
     **/
    this.setProperties = function(params) {
        if (params.hasOwnProperty('color')) {
            fillColor = Common.processColor(params.color);
        }
        if (params.hasOwnProperty('borderColor')) {
            lineColor = Common.processColor(params.borderColor);
        }
        if (params.hasOwnProperty('borderThickness')) {
            lineWidth = params.borderThickness;
        }
        //Updating all resource items
        if (rectItems.length > 0) {
            for (var idx = 0; idx < rectItems.length; idx++) {
                rectItems[idx].updateResource();
            }
        }
    };

    /* Function to create pxTextItem object */
    this.createPXItem = function(view) {
        var item = {};
        clog("createPXItem");
        if (rectItems.length === 0 || rectItems[rectItems.length - 1].getParentView()) {
            clog("Creating new rectItem");
            item = new PXRectangleItem(view, id, app, scene, this);
            rectItems.push(item);
        } else {
            clog("Rect item already present");
            item = rectItems[rectItems.length - 1];
            rectItems[rectItems.length - 1].setParentView(view);
            rectItems[rectItems.length - 1].updateResource();
        }
        return item;
    };

    /**
     * Function to get a property from rectangle resource
     **/
    this.getProperty = function(name) {
        var value = Constants.XRE_ID_NULL;
        if (name === "color") {
            value = fillColor;
        } else if (name === "borderColor") {
            value = lineColor;
        } else if (name === "borderThickness") {
            value = lineWidth;
        }
        return value;
    };

    /* Method to get color */
    this.getColor = function() {
        return fillColor;
    };

    /* Method to get border color */
    this.getBorderColor = function() {
        return lineColor;
    };

    /* Method to get border thickness */
    this.getBorderThickness = function() {
        return lineWidth;
    };

    //Setting parameters to rectangle object
    this.setProperties(params);
    var item = new PXRectangleItem(null, id, app, scene, this);
    rectItems.push(item);
};

/** 
 *   Class to handle rectangle scene object and asign properties to the object 
 **/
var PXRectangleItem = function(view, id, app, scene, res) {

    var parentView = null;
    var resource = res;

    var rectObj = scene.createRectangle();

    /**
     *   Method to set resource object with required properties  
     **/
    this.updateResource = function() {
        if (rectObj) {
            rectObj.fillColor = resource.getColor();
            rectObj.lineColor = resource.getBorderColor();
            rectObj.lineWidth = resource.getBorderThickness();
            if (parentView) {
                var viewObj = parentView.getViewObj();
                rectObj.w = viewObj.w; // - rectObj.lineWidth - 1;
                rectObj.h = viewObj.h; // - rectObj.lineWidth - 1;
                //rectObj.x = viewObj.x + rectObj.lineWidth;
                //rectObj.y = viewObj.y + rectObj.lineWidth;
                rectObj.useMatrix = true;
            }
        }
    };

    /**
     *    Function to get rectangle object measurements
     **/
    this.getObjectRect = function() {
        var objRect = {};
        objRect.X = rectObj.x;
        objRect.Y = rectObj.y;
        objRect.W = rectObj.w;
        objRect.H = rectObj.h;
        return objRect;
    };
    /**
     *    Function to get scene rectangle object
     **/
    this.getSceneObject = function() {
        return rectObj;
    };
    this.getWidth = function() {
        return rectObj.w;
    };
    this.getHeight = function() {
        return rectObj.h;
    };
    this.getParentView = function() {
        return parentView;
    };
    this.setParentView = function(view) {
        parentView = view;
        rectObj.parent = parentView.getViewObj();
        resource.registerMouseEvent(resource, rectObj, view);
    };
    if (view) {
        this.setParentView(view);
    }
    this.updateResource();
};

//Inherting XREResource
RectangleResource.prototype = Object.create(XREResource.prototype);
RectangleResource.prototype.constructor = RectangleResource;

module.exports = RectangleResource;
