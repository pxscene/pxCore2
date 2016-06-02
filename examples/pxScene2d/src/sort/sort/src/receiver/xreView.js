/************************************************************************
 * Name         : XREView.js
 * Author       : Jisha , Seetha
 * Created      : 6/9/2015
 * Description  : XREView.js handles the view in XREReceiver.
 *************************************************************************/

Constants = require("../utils/config.js");
var RESULT = require('../utils/enum').RESULT;
var XREObject = require("./xreObject.js");
var XREKeyEvent = require("../events/xreKeyEvent.js");

var COpaqueEvent = require("../events/resourceEvent.js").COpaqueEvent;
var COnResourceMeasuredEvent = require("../events/resourceEvent.js").COnResourceMeasuredEvent;
var EventEmitter = require('events').EventEmitter;
var XRElogger = require('../utils/xrelogger.js');
var XREAnimation = require('../receiver/xreAnimation.js').XREAnimation;
var ExpressionEvaluator = require("./xreexpressionevaluator.js");
var PXTextItem = require("./xretextresource.js").PXTextItem;
/**
 * Class XREView
 **/
var XREView = function(pcanvas, id, app, scene, params) {
    var _this = this;
    var viewObj = scene.create({
        t: "object"
    });

    this.prototype = new XREObject(id, app);
    XREView.prototype.constructor = XREObject;

    //this.app = app;
    var viewId = id;
    var pCanvas = pcanvas;
    var parentObject = Constants.XRE_ID_NULL;
    var parentId = Constants.XRE_ID_NULL;
    var resourceId = Constants.XRE_ID_NULL;
    var maskId = Constants.XRE_ID_NULL;
    var maskUrl = Constants.XRE_ID_NULL;
    var resourceItem = Constants.XRE_ID_NULL;
    var resourceOptions = this;
    var resourceClip = false;
    var painting = true;
    var bIgnoreMouse = false;
    var bPunchThrough = false;
    var bLastVisibleState = true;
    var rect = Constants.XRE_ID_NULL;
    var childViews = [];
    var eventEmitter = new EventEmitter();
    var defaultLogger = XRElogger.getLogger("default");
    /*
     *Returns the parentObject value
     */
    this.getParentObject = function() {
        return parentObject;
    };

    /*
     *Method to raise an event
     */
    this.raiseEvent = function(event, modifiers) {
        defaultLogger.log("info", " Inside raise event " + resourceId);
        var res = this.prototype.GetApp().getResource(resourceId);
        defaultLogger.log("info", "resource ===========");

        //If our child handles view events, don't raise one ourselvs
        if (res && res.handlesViewEvents()) {
            //We don't have a common baseclass for our resource items, so we do this via a signal
            //emit gotEvent(event);
            res.getEmitter().emit('gotEvent', event, modifiers);
            return;
        }
        //To do for EAS


        switch (event.name) {
            case "onKeyDown":
                defaultLogger.log("info", "going raiseEvent  keyDown...");
                //QKeyEvent * keyEvent = static_cast < QKeyEvent * > (event);
                this.raiseKeyEvent(event, "onKeyDown", modifiers);
                defaultLogger.log("info", " raisedEvent  keyDown...");
                break;
            case "onKeyUp":
                defaultLogger.log("info", "going raiseEvent  keyUp...");
                this.raiseKeyEvent(event, "onKeyUp", modifiers);
                defaultLogger.log("info", "raisedEvent  keyDown...");
                break;

            default:
                break;
        }



    };
    /*
     *Method to raise Key event
     */
    this.raiseKeyEvent = function(event, eName, modifiers) {
        var keyMap = this.prototype.GetApp().getKeyMap();
        defaultLogger.log("info", event.keyCode);
        var vk = keyMap.getVK(event.keyCode, modifiers);
        defaultLogger.log("info", "virtualkeycode=================" + vk);
        if (vk != "unknown") {
            var res = this.prototype.GetApp().getResource(resourceId);
            var xreEvent = new XREKeyEvent(vk, event.keyCode, modifiers.alt, modifiers.ctrl, modifiers.shift, modifiers.meta);
            xreEvent.prototype.setName(eName);
            this.prototype.eventEmitter.emit('Event', xreEvent, this, res && res.isKeySink()); //If this resource processes key presses only send preview phase
        }
    };



    /**
     * Description : Sets the Default value of XREview.
     * Parameter   : scene of application canvas
     **/

    var setDefaults = function(sceneRect, scene) {

        if (viewId == Constants.XRE_ID_ROOT_VIEW) {
            setRect(sceneRect);
            if (!app.getRootXREApplication()) {
                viewObj.parent = scene.root;
            }
        } else {
            setRect(0, 0, 300, 300);
        }
    };


    /**
     * Description  : Set the XREview with dimensions x,y,w,h
     * Parameter    : x,y,w,h
     **/

    var setRect = function(x, y, w, h) {
        if (typeof x == "number") {
            rect = {
                "X": x,
                "Y": y,
                "W": w,
                "H": h
            };
        } else {
            rect = x;
        }

        viewObj.x = rect.X;
        viewObj.y = rect.Y;
        viewObj.w = rect.W;
        viewObj.h = rect.H;

    };
    /** function to generate event **/
    var generateEvent = function(params) {
        var source = params.source;
        if (source !== _this.getId() && source !== resourceId) {
            return RESULT.XRE_E_INVALID_ID;
        }

        var event = new COpaqueEvent(params);
        _this.prototype.eventEmitter.emit('Event', event, _this);
        return RESULT.XRE_S_OK;
    };
    /** Function to measure resource **/
    var measureResource = function() {
        if (resourceItem === Constants.XRE_ID_NULL) {
            return;
        }
        //var resource = app.getResource(resourceId);
        var itemRect = resourceItem.getObjectRect();
        var event = new COnResourceMeasuredEvent(itemRect);
        event.prototype.setHandlerID(viewId);
        event.prototype.setSourceID(viewId);
        _this.prototype.eventEmitter.emit('Event', event, _this);
    };
    var quirk5553Enabled = function() {
        return true;
    };
    this.setJsonProperties = function(params, obj) {
//        clog("setJsonProperties===========");
//        clog(params);
        for (var prop in params) {
//            clog("Inside for-------------------" + prop + params[prop]);
//            clog(obj);
            if (obj.hasOwnProperty(prop)) {
//                clog("Inside hasOwnProperty-------------------" + params[prop]);
                obj.prop = params[prop];
//                clog("obj.prop" + obj.prop);
            }
        }

    };


    /** Function to animate view **/
    var animate = function(id) {
        var resource = _this.prototype.GetApp().getResource(id);
        if (resource instanceof XREAnimation) {
            var obj = viewObj;
            var json = resource.toJson(obj);
            if (resource.getType() === "XREColorAnimation") {
                obj = resourceItem.getSceneObject();
                if (resourceItem instanceof PXTextItem) {
                    var txtColor = json.fillColor;
                    json = {};
                    json["textColor"] = txtColor;
                    animateTextColor(obj, json, resource);
                } else {
                    animateRectColor(obj, json, resource);
                }

            } else if (resource.getType() === "XRETransformAnimation") {
                var sM11 = json.sx;
                var sM12 = 0;
                var sM21 = 0;
                var sM22 = json.sy;
                var sDx = 0;
                var sDy = 0;

                var tM11 = 1;
                var tM12 = 0;
                var tM21 = 0;
                var tM22 = 1;
                var tDx = json.x + obj.m41;
                var tDy = json.y + obj.m42;

                var sMatrix = [
                    [sM11, sM12, 0],
                    [sM21, sM22, 0],
                    [sDx, sDy, 1]
                ];
                var tMatrix = [
                    [tM11, tM12, 0],
                    [tM21, tM22, 0],
                    [tDx, tDy, 1]
                ];
                if (json.r !== 0) {
                    //Doing rotation for 360 degree only
                    var rotateMatrix1 = createRotationMatrix(1.57079633);
                    var rotateMatrix2 = multiply(rotateMatrix1, createRotationMatrix(1.57079633));
                    var rotateMatrix3 = multiply(rotateMatrix2,createRotationMatrix(1.57079633));
                    var rotateMatrix4 = multiply(rotateMatrix3, createRotationMatrix(1.57079633));
                    var promise = obj.animateTo({
                        m11: sMatrix[0][0],
                        m12: sMatrix[0][1],
                        m21: sMatrix[1][0],
                        m22: sMatrix[1][1],
                    }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                        var promise = obj.animateTo({
                            m11: rotateMatrix1[0][0],
                            m12: rotateMatrix1[0][1],
                            m21: rotateMatrix1[1][0],
                            m22: rotateMatrix1[1][1],


                        }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                            var promise = obj.animateTo({
                                m11: rotateMatrix2[0][0],
                                m12: rotateMatrix2[0][1],
                                m21: rotateMatrix2[1][0],
                                m22: rotateMatrix2[1][1],

                            }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                                var promise = obj.animateTo({
                                    m11: rotateMatrix3[0][0],
                                    m12: rotateMatrix3[0][1],
                                    m21: rotateMatrix3[1][0],
                                    m22: rotateMatrix3[1][1],
                                }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                                    var promise = obj.animateTo({
                                        m11: rotateMatrix4[0][0],
                                        m12: rotateMatrix4[0][1],
                                        m21: rotateMatrix4[1][0],
                                        m22: rotateMatrix4[1][1],

                                    }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                                        resource.notifyComplete(_this.prototype.GetId());
                                    });
                                });
                            });
                        });
                    });

                } else {
                    var product = multiply(sMatrix, tMatrix);

                    var newJson = {};
                    newJson["m11"] = product[0][0];
                    newJson["m12"] = product[0][1];
                    newJson["m21"] = product[1][0];
                    newJson["m22"] = product[1][1];
                    if (product[2][0] !== 0) {
                        newJson["m41"] = product[2][0];
                    }
                    if (product[2][1] !== 0) {
                        newJson["m42"] = product[2][1];
                    }
                    obj.animateTo(newJson, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                        resource.notifyComplete(_this.prototype.GetId());

                    });
                }
            } else {
                if (resource.getType() === "XREDimensionsAnimation") {
                    if (resourceItem) {
                        obj = resourceItem.getSceneObject();
                    }
                }
                animateObject(json, resource, obj);
            }
        }
    };
    var createRotationMatrix = function(rad) {
        var rM11 = Math.cos(rad);
        var rM12 = Math.sin(rad);
        var rM21 = -rM12;
        var rM22 = rM11;
        var rDx = 0;
        var rDy = 0;
        var rMatrix = [
            [rM11, rM12, 0],
            [rM21, rM22, 0],
            [rDx, rDy, 1]
        ];
        return rMatrix;
    };
    var multiply = function(a, b) {
        var aNumRows = a.length,
            aNumCols = a[0].length,
            bNumRows = b.length,
            bNumCols = b[0].length,
            m = new Array(aNumRows); // initialize array of rows
        for (var r = 0; r < aNumRows; ++r) {
            m[r] = new Array(bNumCols); // initialize the current row
            for (var c = 0; c < bNumCols; ++c) {
                m[r][c] = 0; // initialize the current cell
                for (var i = 0; i < aNumCols; ++i) {
                    m[r][c] += a[r][i] * b[i][c];
                }
            }
        }
        clog(m);
        return m;
    }
    var animateObject = function(json, resource, obj) {
        var promise = obj.animateTo(json, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
            defaultLogger.log("info", "animation done =====================================" + id);
            resource.notifyComplete(_this.prototype.GetId());
            // _this.setJsonProperties(json, obj);
        });
    };
    var animateRectColor = function(obj, json, resource) {
        obj.animateTo({
            a: .1
        }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
            obj.fillColor = json.fillColor;
            obj.lineColor = json.lineColor;
            obj.animateTo({
                a: 1
            }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                defaultLogger.log("info", " Color animation for rectangle done=====================================" + id);
                resource.notifyComplete(_this.prototype.GetId());

            });
        });
    };

    var animateTextColor = function(obj, json, resource) {
        //obj.a = 0;
        obj.animateTo({
            a: .7
        }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
            obj.textColor = json.textColor;
            obj.animateTo({
                a: 1
            }, resource.getDuration(), resource.getEasing(), scene.PX_END).then(function() {
                defaultLogger.log("info", " Color animation for text done=====================================" + id);
                resource.notifyComplete(_this.prototype.GetId());

            });

        });

    };
    /*
     * Function to Set the Resource Options.
     */
    var setOptions = function(optParam) {
        resourceOptions = {
            "textWrap": optParam.textWrap,
            "verticalAlign": optParam.verticalAlign,
            "horizontalAlign": optParam.horizontalAlign,
            "stretch": optParam.stretch,
            "textTruncStyle": optParam.textTruncStyle,
            "textStartPos": optParam.textStartPos,
            "textStopPos": optParam.textStopPos,
            "textStartChar": optParam.textStartChar,
            "textStopChar": optParam.textStopChar
        };
        if (optParam.textStartPos) {
            resourceOptions["textStartPos"][0] = evaluateNumericProperty(optParam.textStartPos[0]);
            resourceOptions["textStartPos"][1] = evaluateNumericProperty(optParam.textStartPos[1]);
        }
        if (optParam.textStopPos) {
            resourceOptions["textStopPos"][0] = evaluateNumericProperty(optParam.textStopPos[0]);
            resourceOptions["textStopPos"][1] = evaluateNumericProperty(optParam.textStopPos[1]);
        }
    };


    /* 
     * Scaling and positioning the Resource with
     * respect to view.
     */

    var scaleResource = function(w, h) {
        var targetImageSize = {
            "W": w,
            "H": h
        };

        var newSize = {};

        if (resourceOptions.stretch != "NONE") {
            if (resourceOptions.stretch == "FIT_HEIGHT") {
                defaultLogger.log("info", " Entered inside FIT height ");
                newSize.H = viewObj.h;
                newSize.W = viewObj.h * (targetImageSize.W / targetImageSize.H);
                targetImageSize = newSize;

            } else if (resourceOptions.stretch == "FILL") {

                targetImageSize.W = viewObj.w;
                targetImageSize.H = viewObj.h;

            } else if (resourceOptions.stretch == "FILL_WITH_CLIP") {
                targetImageSize.W = viewObj.w;
                targetImageSize.H = viewObj.h;

            } else if (resourceOptions.stretch == "FIT_WIDTH") {
                newSize.W = viewObj.w;
                newSize.H = viewObj.w * (targetImageSize.H / targetImageSize.W);
                targetImageSize = newSize;

            } else if (resourceOptions.stretch == "FIT_BEST") {
                var  area = viewObj.w * viewObj.h;
                targetImageSize.W = Math.sqrt(area * (targetImageSize.W / targetImageSize.H));
                targetImageSize.H = area / targetImageSize.W;


            } else {
                return targetImageSize;

            }
            return targetImageSize;

        }
        return targetImageSize;
    };
    /*
     * Returns the offset value of the resource with respect
     * to the view.
     */
    var getOffset = function(targetImageSize) {
        var offset = {};
        offset.X = getOffsetPosition(resourceOptions.horizontalAlign, viewObj.w, targetImageSize.W);
        offset.Y = getOffsetPosition(resourceOptions.verticalAlign, viewObj.h, targetImageSize.H);
        return offset;
    };
    /*
     * Get the offset value according to the alignment of
     * resource option.
     */
    var getOffsetPosition = function(align, v, r) {
        if (align == "LEFT" || align == "TOP") {
            return 0;
        } else if (align == "CENTER") {
            return (v - r) / 2;
        } else if (align == "RIGHT" || align == "BOTTOM") {
            return v - r;
        }
        return 0;
    };
    /* Function to set mask for the view */

    var setMask = function(mask) {
        maskId = mask;
        if (mask == Constants.XRE_ID_NULL || mask == 'null') {
            /* Sets the mask to null if maskUrl is not set */
            if (!maskUrl) {
                viewObj.mask = Constants.XRE_ID_NULL;
            }
            return;
        }
        if (maskId != Constants.XRE_ID_NULL) {
            maskUrl = getMask();
        }
        updateMask();
    };

    /* Function to update mask for all child views of the view */

    var updateMask = function() {
        if (maskUrl != Constants.XRE_ID_NULL) {
            /*  Current receiver implementation scales the pixmap from 
              mask image before adding it as mask  */
            if (childViews.length > 0) {
                for (var idx = 0; idx < childViews.length; idx++) {
                    if (childViews[idx].getResourceItem()) {

                        childViews[idx].setMaskToView(maskId, maskUrl);
                    }
                }
            } else {
                viewObj.mask = maskUrl;
            }
        }
    };

    /* Get the mask url for the view */

    var getMask = function() {
        var resource = app.getResource(maskId);
        if (resource instanceof CImageResource && resource != Constants.XRE_ID_NULL && resource.getUrl())
            return resource.getUrl();
        else
            return Constants.XRE_ID_NULL;
    };
    /** Function to add all child views to parent **/
    this.addAllChildViews = function() {
        for (var idx = 0; idx < childViews.length; idx++) {
            childViews[idx].setParentObject(_this);
        }
    };
    /* Function to set resource to the view */
    var setResource = function(id) {
        resource = app.getResource(id);
        if (resource) {
            _this.removeResource();
            viewObj.removeAll();
            resourceId = id;
            resourceItem = resource.createPXItem(_this);
            _this.addAllChildViews();
            if (resourceItem) {
                defaultLogger.log("info", "setResource : INFO : Got resource item");
                // TODO set z value of mResourceItem
                // Add view to view array of resource
            } else {
                defaultLogger.log("info", "setResource : ERROR : No resource item obtained");
            }
        } else {
            defaultLogger.log("info", "setResource : ERROR : No resource with id " + id);
        }
    };
    /** 
     *Function to remove child from the view
     */
    var removeChild = function(id) {
        var childView = pCanvas.getView(id);
        var parentView;
        if (childView) {
            parentView = childView.getParentObject().getViewObj();
            if (childView.getParentObject() !== this) {
                childView.getParentObject().removeChildItem(childView);
                parentView.removeAll();
                _this.addAllChildViews();
            }

            childView.setParentObject(null);
        }
    };
    /** Function to set child index **/
    var setChildIndex = function(viewId, id) {
        //TODO
        var view = pCanvas.getView(viewId);
        for (var idx = 0; idx < childViews.length; idx++) {
            if (childViews[idx] === view) {
                childViews.splice(idx, 1);
            }
        }
        childViews.splice(id, 0, view);
        viewObj.removeAll();
        _this.addAllChildViews();
        //UpdateZOrder
    };
    /** Function to get id of the view ***/
    this.getId = function() {
        return viewId;
    };
    /*** Function to get viewObject of the view ***/
    this.getViewObj = function() {
        return viewObj;
    };
    /** Function to get resource item **/
    this.getResourceItem = function() {
        return resourceItem;
    };
    /** Function to get canvas object **/
    this.getCanvasObject = function() {
        return pCanvas;
    };
    /** Function to get resource options **/
    this.getResourceOptions = function() {
        return resourceOptions;
    };
    this.getResourceId = function() {
            return resourceId;
        }
        /* 
         * If the view object w,h is not defined,then w,h
         * of Resource will be set to the viewObject.
         */

    this.getResourceRect = function(w, h) {
        if ((viewObj.w === 0) && (viewObj.h === 0)) {

            return {
                "X": 0,
                "Y": 0,
                "W": w,
                "H": h
            };
        }

        var targetImageSize = scaleResource(w, h);
        var offset = getOffset(targetImageSize);
        var resourceRect = {
            "X": offset.X,
            "Y": offset.Y,
            "W": targetImageSize.W,
            "H": targetImageSize.H
        };
        return resourceRect;
    };
    /*
     * This method will process the params.
     */

    this.setProperties = function(params) {
        if (params.hasOwnProperty('matrix')) {
            viewObj.useMatrix = true;
            viewObj.m11 = evaluateNumericProperty(params.matrix[0]);
            viewObj.m12 = evaluateNumericProperty(params.matrix[1]);
            viewObj.m13 = evaluateNumericProperty(params.matrix[2]);
            viewObj.m14 = evaluateNumericProperty(params.matrix[3]);
            viewObj.m21 = evaluateNumericProperty(params.matrix[4]);
            viewObj.m22 = evaluateNumericProperty(params.matrix[5]);
            viewObj.m23 = evaluateNumericProperty(params.matrix[6]);
            viewObj.m24 = evaluateNumericProperty(params.matrix[7]);
            viewObj.m31 = evaluateNumericProperty(params.matrix[8]);
            viewObj.m32 = evaluateNumericProperty(params.matrix[9]);
            viewObj.m33 = evaluateNumericProperty(params.matrix[10]);
            viewObj.m34 = evaluateNumericProperty(params.matrix[11]);
            viewObj.m41 = evaluateNumericProperty(params.matrix[12]);
            viewObj.m42 = evaluateNumericProperty(params.matrix[13]);
            viewObj.m43 = evaluateNumericProperty(params.matrix[14]);
            viewObj.m44 = evaluateNumericProperty(params.matrix[15]);
        }

        if (params.hasOwnProperty('dimensions')) {
            viewObj.w = evaluateNumericProperty(params.dimensions[0]);
            viewObj.h = evaluateNumericProperty(params.dimensions[1]);
        }

        if (params.hasOwnProperty('alpha')) {
            viewObj.a = evaluateNumericProperty(params.alpha);
        }

        if (params.hasOwnProperty('visible')) {
            viewObj.draw = params.visible;
        }

        if (params.hasOwnProperty('clip')) {
            viewObj.clip = params.clip;
        }

        if (params.hasOwnProperty('painting')) {
            viewObj.painting = params.painting;
        }

        if (params.hasOwnProperty('ignoreMouse')) {
            viewObj.interactive = params.ignoreMouse;
        }
        if (params.hasOwnProperty('mask')) {
            setMask(params.mask);
        }
        if (params.hasOwnProperty('parent')) {
            var parentView = pCanvas.getView(params.parent);
            if (parentView) {
                parentView.addChild(this);
            }
        }
        if (params.hasOwnProperty('resourceOptions')) {
            setOptions(params.resourceOptions);
        }
        if (params.hasOwnProperty('resource')) {
            setResource(params.resource);
        }
        this.prototype.eventHandlers.install(params);
        //this.prototype.getEmitter().emit('Changed', this);
        if (resourceItem) {
            resourceItem.updateResource();
        }
//        clog("Exiting set prop");
    };
    /** Function to set resource item **/

    this.setResourceItem = function(item) {
        resourceItem = item;
    };

    /** Function to set mask url and mask id to a view **/
    this.setMaskToView = function(id, url) {

        maskId = id;
        maskUrl = url;
        viewObj.mask = url;
    };
    /** Method to set parent of current view **/
    this.setParentObject = function(view) {
        parentObject = view;
        parentId = (view !== null) ? view.getId() : 0;
        viewObj.parent = (view !== null) ? view.getViewObj() : {};
    };
    /*** Function to destroy a view ***/
    this.destroyView = function(view) {
        if (!view) {
            view = this;
        }
        view.removeResource();
        view.getCanvasObject().removeView(view.getId());
        clog("destroy view 4");
        if(view.getId() != Constants.XRE_ID_ROOT_VIEW){
        view.getParentObject().removeChildItem(view);
    }
        clog("destroy view 5");
        view.removeChildren();
        if (app.activeView === view) {
            //TODO
            app.activeViewDeleting = true;
            app.call(Constants.XRE_ID_ROOT_VIEW, "activate", params, "");
        }
    };
    /** 
     *Function to remove all child views from the view
     */
    this.removeChildren = function() {
        for (var idx = 0; idx < childViews.length; idx++) {
            this.destroyView(childViews[idx]);
            childViews.splice(idx, 1);
        }
    };
    /**
     *  Remove the child from current view
     **/
    this.removeChildItem = function(view) {
        for (var idx = 0; idx < childViews.length; idx++) {
            if (childViews[idx] === view) {
                childViews.splice(idx, 1);
            }

        }
        view.getViewObj().parent = {};
    };
    /* Function to remove resource from the view */

    this.removeResource = function() {
        // Remove view from view array of resource
        resourceId = Constants.XRE_ID_NULL;
        if (resourceItem) {
            resourceItem.getSceneObject().parent = {};
            viewObj.removeAll();
            _this.addAllChildViews();
            resourceItem.getSceneObject().remove();
        }
        resourceItem = Constants.XRE_ID_NULL;
    };
    /** Function to activate a resource item **/
    this.activate = function() {
        if (resourceItem) { //ensure input consumers like FlashItem and HTMLViewItem take focus when their view is actived
            scene.setFocus(resourceItem.getSceneObject());
        }
        //TODO registered for flashItem and htmlviewItem only in current code 
        //emit activated();
    };
    /** 
     *Function to add child to the view
     */
    this.addChild = function(arg) {
        var childView;
        if (typeof arg === 'number') {
            childView = pCanvas.getView(arg);
            if (childView) {
                var currParentView = childView.getParentObject();
                if (currParentView) {
                    currParentView.removeChildItem(childView);
                }
            }
        } else if (arg instanceof XREView) {
            childView = arg;
        }
        for (var idx = 0; idx < childViews.length; idx++) {
            if (childViews[idx] == childView) {
                defaultLogger.log("info", "Attempt to violate singularity of child-parent relationship view id :", childview.getID());
                return;
            }
        }
        childView.setParentObject(this);

        childViews.push(childView);

        if (maskUrl) {
            childView.setMask(maskUrl);
        }
        //TODO Set z index 
    };

    /** Function to call diffrent methods based on the params **/
    this.callMethod = function(method, params) {
        // params is not json i guess
        defaultLogger.log("info", "callMethod : Method : " + method + " Params ");
        defaultLogger.log("info", params);
        if (params !== null) {
            switch (method) {
                case "animate":
                    var animId = params[0];
                    animate(animId);
                    return RESULT.XRE_S_OK;
                case "addChild":
                    this.addChild(params[0]);
                    return RESULT.XRE_S_OK;
                case "removeChild":
                    removeChild(params[0]);
                    return RESULT.XRE_S_OK;
                case "setChildIndex":
                    setChildIndex(params[0], params[1]);
                    return RESULT.XRE_S_OK;
                case "measureResource":
                    measureResource();
                    return RESULT.XRE_S_OK;

                case "generateEvent":
                    return generateEvent(params[0].params);
                default:
                    defaultLogger.log("info", "callMethod : Method : " + method + "not implemented");
                    break;
            }
        }
        return RESULT.XRE_E_OBJECT_DOES_NOT_IMPLEMENT_METHOD;
    };
    this.getEventEmitter = function() {
        return eventEmitter;
    };
    this.getNumericPropertyByName = function(property) {
        if (resourceItem) {
            var item = resourceItem.getSceneObject();
        }
        var val = 0.0;
        switch (property) {
            case "measuredHorizontalExtent":
            case "measuredPos.x":
                if (item) {
                    //val = item.w;
                    val = resourceItem.getWidth();
                    //TODO proper quirk check
                    if (!quirk5553Enabled())
                    //Horizontal scale factor of resource
                        val *= item.m11;
                    //Tranform dx value of resource
                    val += item.m41;
                    clog(val);
                }
                break;
            case "measuredVerticalExtent":
            case "measuredPos.y":
                if (item) {
                    clog("height " + resourceItem.getHeight());
                    val = resourceItem.getHeight();
                    //val = item.h;
                    //TODO proper quirk check
                    if (!quirk5553Enabled())
                    //Vertical scale factor of resource
                        val *= item.m22;
                    //Tranform dy value of resource
                    val += item.m42;
                }
                break;
            case "width":
                val = viewObj.w;
                break;
            case "viewX":
                //Transform dx value of view
                val = viewObj.m41;
                break;
            case "viewY":
                //Transform dy value of view
                val = viewObj.m42;
                break;

            case "height":
                val = viewObj.h;
                break;
            case "resourceX":
                if (item) {
                    //Tranform dx value of resource   
                    val = item.m41;
                } else {
                    val = 0;
                }

                break;
            case "resourceY":
                if (item) {
                    //Tranform dy value of resource  
                    val = item.m42;
                }

                break;

            case "resourceWidth":

                if (item) {
                    val = resourceItem.getWidth();
                    if (!quirk5553Enabled()) {
                        //Horizontal scale factor of resource
                        val *= item.m11;
                    }
                }
                break;
            case "resourceHeight":

                if (item) {
                    val = resourceItem.getHeight();
                    if (!quirk5553Enabled()) {
                        //Vertical scale factor of resource
                        val *= item.m22;
                    }
                }
                break;
            case "boundingBox.x":

                if (item) {
                    if (resourceItem instanceof PXTextItem) {
                        val = resourceItem.getBoundingBoxX();
                    } else {
                        //TODO Confirm
                        val = item.x;
                    }
                    val += item.m41;
                }
                break;
            case "boundingBox.y":
                if (item) {
                    if (resourceItem instanceof PXTextItem) {
                        val = resourceItem.getBoundingBoxY();
                    } else {
                        //TODO Confirm
                        val = item.x;
                    }
                    val += item.m42;
                }
                break;
            case "boundingBox.width":

                if (item) {
                    if (resourceItem instanceof PXTextItem) {
                        val = resourceItem.getBoundingBoxWidth();
                    } else {
                        //TODO Confirm
                        val = item.w;
                    }
                    val += resourceItem.getWidth();
                }
                break;
            case "boundingBox.height":
                if (item) {
                    if (resourceItem instanceof PXTextItem) {
                        val = resourceItem.getBoundingBoxHeight();
                    } else {
                        //TODO Confirm
                        val = item.h;
                    }
                    val += resourceItem.getHeight();
                }
                break;
            case "textStartPos.x":
                val = resourceOptions.textStartPos[0];
                break;
            case "textStartPos.y":
                val = resourceOptions.textStartPos[1];
                break;
            case "textStopPos.x":
                val = resourceOptions.textStopPos[0];
                break;
            case "textStopPos.y":
                val = resourceOptions.textStopPos[1];
                break;
            case "textStartChar":
                val = resourceOptions.textStartChar;
                break;
            case "textStopChar":
                val = resourceOptions.textStopChar;
                break;
            default:
                if (property.substring(0, 17) === "textMeasuredChar(") {
                    var txt = property.substring(16, property.length);

                    var rightParenIdx = txt.indexOf(")");
                    var index = parseInt(txt.substring(0, rightParenIdx));
                    txt = txt.substring(2 + rightParenIdx, txt.length); // skip ")."
                    //ICharMeasurer *item = dynamic_cast<ICharMeasurer*>( this->m_resourceItem );
                    if (resourceItem instanceof PXTextItem && item) {
                        if (txt === 'x') {
                            //return m_resourceItem->transform().dx() + item->measuredCharX(index);
                            val = item.x; // + resourceItem.measuredCharX(index);
                        } else if (txt === 'y') {
                            //return m_resourceItem->transform().dy() + item->measuredCharY(index);
                            val = item.y; // + measuredCharY(index);
                        }

                    }
                    //return 0;
                } else {
                    defaultLogger.log("info", "can't get property " + property + " from view. It doesn't exist");
                    break;
                }

        }
        return val;
    };
    var evaluateNumericProperty = function(property) {
        //Assuming only object need to be
        if (typeof property === 'string') {
            clog("evaluateExpression---------------");
            clog(property);
            return ExpressionEvaluator.evaluateExpression(pCanvas, property);
        } else {
            //Not considering data type 
            return property;
        }

    };
    setDefaults(pcanvas.getSceneRect(), scene);

    if (params) {
        this.setProperties(params);
    }
};
module.exports = XREView;
