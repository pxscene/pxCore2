/*
 * File Name - xreAnimation.js
 *
 *  This file defines classes for xre animation
 *
 */
var XREResource = require('./xreresource.js');
var RESULT = require('../utils/enum').RESULT;
var AnimationType = require("../utils/config.js");
var XREObject = require('./xreObject.js');
var Constants = require('../events/xreprotocol.js');
var ResourceEvent = require('../events/resourceEvent.js').ResourceEvent;
var ExpressionEvaluator = require('./xreexpressionevaluator.js');
var Common = require("../utils/common.js");
/**
 * Class XREAnimation
 **/
var XREAnimation = function (id, app, params) {
    XREResource.call(this, id, app);
    var duration = 0;
    this.sendOnComplete = false;

    if (params.hasOwnProperty('duration')) {
        duration = (ExpressionEvaluator.evaluateNumericProperty(app, params.duration))/1000;
    }
    if (params.hasOwnProperty('easing')) {
        var easing = params.easing;
        if (easing == Constants.EASING_QUAD_IN) {
            this.easingCurve = Constants.EASING_QUAD_IN_VALUE;
        } else if (easing == Constants.EASING_QUAD_OUT) {
            this.easingCurve = Constants.EASING_QUAD_OUT_VALUE;
        } else if (easing == Constants.EASING_QUAD_IN_OUT) {
            this.easingCurve = Constants.EASING_QUAD_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_CUBIC_IN) {
            this.easingCurve = Constants.EASING_CUBIC_IN_VALUE;
        } else if (easing == Constants.EASING_CUBIC_OUT) {
            this.easingCurve = Constants.EASING_CUBIC_OUT_VALUE;
        } else if (easing == Constants.EASING_CUBIC_IN_OUT) {
            this.easingCurve = Constants.EASING_CUBIC_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_QUART_IN) {
            this.easingCurve = Constants.EASING_QUART_IN_VALUE;
        } else if (easing == Constants.EASING_QUART_OUT) {
            this.easingCurve = Constants.EASING_QUART_OUT_VALUE;
        } else if (easing == Constants.EASING_QUART_IN_OUT) {
            this.easingCurve = Constants.EASING_QUART_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_EXPO_IN) {
            this.easingCurve = Constants.EASING_EXPO_IN_VALUE;
        } else if (easing == Constants.EASING_EXPO_OUT) {
            this.easingCurve = Constants.EASING_EXPO_OUT_VALUE;
        } else if (easing == Constants.EASING_EXPO_IN_OUT) {
            this.easingCurve = Constants.EASING_EXPO_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_SINE_IN) {
            this.easingCurve = Constants.EASING_SINE_IN_VALUE;
        } else if (easing == Constants.EASING_SINE_OUT) {
            this.easingCurve = Constants.EASING_SINE_OUT_VALUE;
        } else if (easing == Constants.EASING_SINE_IN_OUT) {
            this.easingCurve = Constants.EASING_SINE_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_ELASTIC_IN) {
            this.easingCurve = Constants.EASING_ELASTIC_IN_VALUE;
        } else if (easing == Constants.EASING_ELASTIC_OUT) {
            this.easingCurve = Constants.EASING_ELASTIC_OUT_VALUE;
        } else if (easing == Constants.EASING_ELASTIC_IN_OUT) {
            this.easingCurve = Constants.EASING_ELASTIC_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_CIRC_IN) {
            this.easingCurve = Constants.EASING_CIRC_IN_VALUE;
        } else if (easing == Constants.EASING_CIRC_OUT) {
            this.easingCurve = Constants.EASING_CIRC_OUT_VALUE;
        } else if (easing == Constants.EASING_CIRC_IN_OUT) {
            this.easingCurve = Constants.EASING_CIRC_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_BOUNCE_IN) {
            this.easingCurve = Constants.EASING_BOUNCE_IN_VALUE;
        } else if (easing == Constants.EASING_BOUNCE_OUT) {
            this.easingCurve = Constants.EASING_BOUNCE_OUT_VALUE;
        } else if (easing == Constants.EASING_BOUNCE_IN_OUT) {
            this.easingCurve = Constants.EASING_BOUNCE_IN_OUT_VALUE;
        } else if (easing == Constants.EASING_BACK_IN) {
            this.easingCurve = Constants.EASING_BACK_IN_VALUE;
        } else if (easing == Constants.EASING_BACK_OUT) {
            this.easingCurve = Constants.EASING_BACK_OUT_VALUE;
        } else if (easing == Constants.EASING_BACK_IN_OUT) {
            this.easingCurve = Constants.EASING_BACK_IN_OUT_VALUE;
        } else {
            this.easingCurve = Constants.EASING_LINEAR_IN_VALUE;
        }
    }

    /*
     *function to set the properties of XreAnimation class
     */
    this.setProperties = function(params) {
        if (params.hasOwnProperty('onComplete')) {
            this.sendOnComplete = true;
        }
        return RESULT.XRE_S_OK;
    };

    /*
     *function to get the duration
     */
    this.getDuration = function(){
    	return duration;
    };

    /*
     *function to get the easing
     */
    this.getEasing = function(){
    	return this.easingCurve;
    };

    /*
     *function to return particular property
     */
    this.getProperty = function(name) {
        var return_val;
        if ("onComplete" == name) {
            return_val = this.sendOnComplete;
        } else if ("duration" == name) {
            return_val = duration;
        } else if ("easing" == name) {
            switch (this.easingCurve) {
                case 0:
                    return_val = Constants.EASING_LINEAR_IN_OUT;
                    break;
                case Constants.EASING_QUAD_IN_VALUE:
                    return_val = Constants.EASING_QUAD_IN;
                    break;
                case Constants.EASING_QUAD_OUT_VALUE:
                    return_val = Constants.EASING_QUAD_OUT;
                    break;
                case Constants.EASING_QUAD_IN_OUT_VALUE:
                    return_val = Constants.EASING_QUAD_IN_OUT;
                    break;
                case Constants.EASING_CUBIC_IN_VALUE:
                    return_val = Constants.EASING_CUBIC_IN;
                    break;
                case Constants.EASING_CUBIC_OUT_VALUE:
                    return_val = Constants.EASING_CUBIC_OUT;
                    break;
                case Constants.EASING_CUBIC_IN_OUT_VALUE:
                    return_val = Constants.EASING_CUBIC_IN_OUT;
                    break;
                case Constants.EASING_QUART_IN_VALUE:
                    return_val = Constants.EASING_QUART_IN;
                    break;
                case Constants.EASING_QUART_OUT_VALUE:
                    return_val = Constants.EASING_QUART_OUT;
                    break;
                case Constants.EASING_QUART_IN_OUT_VALUE:
                    return_val = Constants.EASING_QUART_IN_OUT;
                    break;
                case Constants.EASING_EXPO_IN_VALUE:
                    return_val = Constants.EASING_EXPO_IN;
                    break;
                case Constants.EASING_EXPO_OUT_VALUE:
                    return_val = Constants.EASING_EXPO_OUT;
                    break;
                case Constants.EASING_EXPO_IN_OUT_VALUE:
                    return_val = Constants.EASING_EXPO_IN_OUT;
                    break;
                case Constants.EASING_SINE_IN_VALUE:
                    return_val = Constants.EASING_SINE_IN;
                    break;
                case Constants.EASING_SINE_OUT_VALUE:
                    return_val = Constants.EASING_SINE_OUT;
                    break;
                case Constants.EASING_SINE_IN_OUT_VALUE:
                    return_val = Constants.EASING_SINE_IN_OUT;
                    break;
                case Constants.EASING_ELASTIC_IN_VALUE:
                    return_val = Constants.EASING_ELASTIC_IN;
                    break;
                case Constants.EASING_ELASTIC_OUT_VALUE:
                    return_val = Constants.EASING_ELASTIC_OUT;
                    break;
                case Constants.EASING_ELASTIC_IN_OUT_VALUE:
                    return_val = Constants.EASING_ELASTIC_IN_OUT;
                    break;
                case Constants.EASING_CIRC_IN_VALUE:
                    return_val = Constants.EASING_CIRC_IN;
                    break;
                case Constants.EASING_CIRC_OUT_VALUE:
                    return_val = Constants.EASING_CIRC_OUT;
                    break;
                case Constants.EASING_CIRC_IN_OUT_VALUE:
                    return_val = Constants.EASING_CIRC_IN_OUT;
                    break;
                case Constants.EASING_BOUNCE_IN_VALUE:
                    return_val = Constants.EASING_BOUNCE_IN;
                    break;
                case Constants.EASING_BOUNCE_OUT_VALUE:
                    return_val = Constants.EASING_BOUNCE_OUT;
                    break;
                case Constants.EASING_BOUNCE_IN_OUT_VALUE:
                    return_val = Constants.EASING_BOUNCE_IN_OUT;
                    break;
                case Constants.EASING_BACK_IN_VALUE:
                    return_val = Constants.EASING_BACK_IN;
                    break;
                case Constants.EASING_BACK_OUT_VALUE:
                    return_val = Constants.EASING_BACK_OUT;
                    break;
                case Constants.EASING_BACK_IN_OUT_VALUE:
                    return_val = Constants.EASING_BACK_IN_OUT;
                    break;
                default:
                    return false;
            }
            return return_val;
        }
        //return this.GetProperty(name);
    };
    this.GetPosition = function(time) {

    };

    this.notifyComplete = function(viewID) {
        var _this = this;
        
        var event = new ResourceEvent(viewID, Constants.EVT_ON_COMPLETE);
       
        event.prototype.setHandlerID(this.GetId());
       
        event.prototype.setSourceID(this.GetId());
       
        this.getEmitter().emit('Event', event, this);
    };

};
XREAnimation.prototype = Object.create(XREResource.prototype);
XREAnimation.prototype.constructor = XREAnimation;

/**
 * Class XREAbsoluteTranslationAnimation
 **/
var XREAbsoluteTranslationAnimation = function(id, app, params) {
    var x = 0;
    var y = 0;
    XREAnimation.call(this, id, app, params);

    if (params.hasOwnProperty("x")) {
        x = ExpressionEvaluator.evaluateNumericProperty(app, params.x);
    }
    if (params.hasOwnProperty("y")) {
        y = ExpressionEvaluator.evaluateNumericProperty(app, params.y);
    }

    /*
     *function to return x value
     */
    this.getX = function() {
        return x;
    };
    /*
     *function to return y value
     */
    this.getY = function() {
        return y;
    };

    /*
     *function to return type of animation
     */
    this.getType = function() {
       return AnimationType.ks_XREAbsoluteTranslationAnimation;
    };
    /*
     *function to return particular property
     */
    this.getProperty = function(name) {
        var returnVal;
        switch (name) {
            case "x":
                returnVal = x;
                break;
            case "y":
                returnVal = y;
                break;
            default:
                returnVal = XREAnimation.getProperty(name);
                break;
        }
        return returnVal;
    };
    /*
     *function to return properties in json format
     */
    this.toJson = function(){
    	var json =  {};
    	json["m41"] = x; 
    	json["m42"] = y; 
    	return json;
    };

};


XREAbsoluteTranslationAnimation.prototype = Object.create(XREAnimation.prototype);
XREAbsoluteTranslationAnimation.prototype.constructor = XREAbsoluteTranslationAnimation;


/**
 * Class XREAlphaAnimation
 **/
var XREAlphaAnimation = function(id, app, params) {
    XREAnimation.call(this, id, app, params);
    var alpha = 0;
    if (params.hasOwnProperty("alpha")) {
        alpha = ExpressionEvaluator.evaluateNumericProperty(app, params.alpha);
    }

    /*
     *function to return alpha value
     */
    this.getAlpha = function() {
        return alpha;
    };
    /*
     *function to return type of animation
     */
    this.getType = function() {
       return AnimationType.ks_XREAlphaAnimation;
    };
    /*
     *function to return particular property
     */
    this.getProperty = function(name) {
        if ("alpha" == name) {
            return alpha;
        } else {
            return XREAnimation.getProperty(name);
        }
    };
    /*
     *function to return properties in json format
     */
    this.toJson = function(){
    	var json =  {};
    	json["a"] = alpha; 
    	return json;
    };

};
XREAlphaAnimation.prototype = Object.create(XREAnimation.prototype);
XREAlphaAnimation.prototype.constructor = XREAlphaAnimation;


/*Class Name: XREDimensionsAnimation*/

var XREDimensionsAnimation = function(id, app, params) {
    clog("Inside XREDimensionsAnimation");
    var width = 0;
    var height = 0;
    clog("before XREAnimation.call");
    XREAnimation.call(this, id, app, params);
    clog("after XREAnimation.call");
    if (params.hasOwnProperty('width')) {
        width = ExpressionEvaluator.evaluateNumericProperty(app, params.width);
    }
    if (params.hasOwnProperty('height')) {
        height = ExpressionEvaluator.evaluateNumericProperty(app, params.height);
    }
    this.getProperty = function(name) {
        var returnVal;
        switch (name) {
            case "width":
                returnVal = width;
                break;
            case "height":
                returnVal = height;
                break;
            default:
                returnVal = XREAnimation.getProperty(name);
                break;
        }
        return returnVal;
    };
    /*
     *function to return type of animation
     */
    this.getType = function() {
       return AnimationType.ks_XREDimensionsAnimation;
    };
    /*
     *function to return properties in json format
     */
    this.toJson = function() {
        var json = {};
        json["h"] = height;
        json["w"] = width;
        return json;
    };
};
XREDimensionsAnimation.prototype = Object.create(XREAnimation.prototype);
XREDimensionsAnimation.prototype.constructor = XREDimensionsAnimation;

var XREAbsoluteScaleAnimation = function(id, app, params) {
    clog("Inside XREAbsoluteScaleAnimation");
    var sx = 0;
    var sy = 0;
    clog("before XREAbsoluteScaleAnimation.call");
    XREAnimation.call(this, id, app, params);
    clog("After XREAbsoluteScaleAnimation.call");
    if (params.hasOwnProperty('sx')) {
        sx = params.sx;
    }
    if (params.hasOwnProperty('sy')) {
        sy = params.sy;
    }
    this.getProperty = function(name) {
        var returnVal;
        switch (name) {
            case "sx":
                returnVal = sx;
                break;
            case "sy":
                returnVal = sy;
                break;
            default:
                returnVal = XREAnimation.getProperty(name);
                break;
        }
        return returnVal;
    };
    /*
     *function to return type of animation
     */
    this.getType = function() {
       return AnimationType.XREAbsoluteScaleAnimation;
    };
    /*
     *function to return properties in json format
     */
    this.toJson = function() {
        var json = {};
        json["m11"] = sx;
        json["m22"] = sy;
        return json;
    };

};
XREAbsoluteScaleAnimation.prototype = Object.create(XREAnimation.prototype);
XREAbsoluteScaleAnimation.prototype.constructor = XREAbsoluteScaleAnimation;

var XREColorAnimation = function(id, app, params) {
    var color = 0xffffffff;
    var borderColor= 0xffffffff;
    XREAnimation.call(this, id, app, params);
    if (params.hasOwnProperty('color')) {
        color = Common.processColor(params.color);
    }
    if (params.hasOwnProperty('borderColor')) {
        borderColor = Common.processColor(params.borderColor);
    }
    this.getProperty = function(name) {
        var returnVal;
        if ("color" == name) {
            returnVal = color;
            return returnVal;
        }
        if ("borderColor" == name) {
            returnVal = borderColor;
            return returnVal;
        }

        return this.getProperty(name);

    };
    /*
     *function to return type of animation
     */
    this.getType = function() {
       return AnimationType.ks_XREColorAnimation;
    };
    /*
/*
 *function to return properties in json format
 */
this.toJson = function(){
        var json =  {};
        json["fillColor"] = color; 
        json["lineColor"] = borderColor;
        return json;
    };
};
XREColorAnimation.prototype = Object.create(XREAnimation.prototype);
XREColorAnimation.prototype.constructor = XREColorAnimation;


var XRETransformAnimation = function(id, app, params) {
    clog("Inside XRETransformAnimation");
    var scaleX = 1;
    var scaleY = 1;
    var rotation = 0;
    var positionsetX = 0;
    var positionsetY = 0;
    var actionPointSetX = 0;
    var actionPointSetY = 0;
    XREAnimation.call(this, id, app, params);
    if (params.hasOwnProperty('x')) {
        positionsetX = params.x;
    }
    if (params.hasOwnProperty('y')) {
        positionsetY = params.y;
    }
    if (params.hasOwnProperty('actionPointX')) {
        actionPointSetX = params.actionPointX;
    }
    if (params.hasOwnProperty('actionPointY')) {
        actionPointSetY = params.actionPointY;
    }
    if (params.hasOwnProperty('scaleX')) {
        scaleX = params.scaleX;
    }
    if (params.hasOwnProperty('scaleY')) {
        scaleY = params.scaleY;
    }

    if (params.hasOwnProperty("rotation")) {
        //Convert to degrees
        var PI = 3.141592653589793;
        //rotation =(ExpressionEvaluator.evaluateNumericProperty(app, params.rotation ) )* (180.0 / PI);
        rotation =(ExpressionEvaluator.evaluateNumericProperty(app, params.rotation )) ;
    }
   // var transMatrix = [[1,0,0,1,positionsetX,positionsetY],[],[]]
    this.getProperty = function(name) {
        var returnVal;
        switch (name) {
            case "x":
                returnVal = positionsetX;
                break;
            case "y":
                returnVal = positionsetY;
                break;
            case "scaleX":
                returnVal = scaleX;
                break;
            case "scaleY":
                returnVal = scaleY;
                break;
            case "rotation":
                returnVal = rotation;
                break;
            case "actionPointX":
                returnVal = actionPointSetX;
                break;
            case "actionPointY":
                returnVal = actionPointSetY;
                break;
            default:
                returnVal = XREAnimation.getProperty(name);
                break;
        }
        return returnVal;

    };
    /*
     *function to return type of animation
     */
    this.getType = function() {
       return AnimationType.ks_XRETransformAnimation;
    };
    /*
     *function to return properties in json format
     */
    this.toJson = function(viewObj) {
        var json = {};
        /*if (scaleX != 1 || scaleY != 1 || rotation !== 0)
        {
                if (scaleX != 1 || scaleY != 1)
                    json["m11"] = scaleX;
                    json["m22"] = scaleY;
                if (rotation !== 0)
                    json["r"] = scaleX;
        }
        json["m41"] = viewObj.m41 + positionsetX;
        json["m42"] = viewObj.m42 + positionsetY;*/
        json["sx"] = scaleX;
        json["sy"] = scaleY;
        json["x"] = positionsetX;
        json["y"] = positionsetY;
        json["cx"] = actionPointSetX;
        json["cy"] = actionPointSetY;
        json["r"] = rotation;
        return json;
    };
};
XRETransformAnimation.prototype = Object.create(XREAnimation.prototype);
XRETransformAnimation.prototype.constructor = XRETransformAnimation;

module.exports = {
    XREAnimation: XREAnimation,
    XREDimensionsAnimation: XREDimensionsAnimation,
    XREAbsoluteScaleAnimation: XREAbsoluteScaleAnimation,
    XREColorAnimation: XREColorAnimation,
    XRETransformAnimation: XRETransformAnimation,
    XREAbsoluteTranslationAnimation: XREAbsoluteTranslationAnimation,  
    XREAlphaAnimation: XREAlphaAnimation 
};