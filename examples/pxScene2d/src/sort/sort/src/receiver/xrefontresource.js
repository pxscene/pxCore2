/**
 * FileName    : xrefontresource.js
 * Created     : 06/18/2015
 * Description : Defines class to handle font resource
 **/

var Constants = require("../utils/config.js");
var RESULT = require('../utils/enum').RESULT;
var FONT_MAP = require("../utils/enum.js").FONT_MAP;
var FONT_STYLE = require("../utils/enum.js").FONT_STYLE;
var Common = require("../utils/common.js");
var XREResource = require("./xreresource.js");
var XRElogger = require('../utils/xrelogger.js');
/*
 *  Class FontResource
 *  Implements all properties related to font
 **/
var FontResource = function(id, app, params, scene) {
    var defaultLogger = XRElogger.getLogger("default");
    var family = null;
    var style = null;
    var fontURL = null;
    //Calling parent constructor
    XREResource.call(this, id, app);
    

    /** Method sets param values to class properties **/
    this.setProperties = function(params) {
        if (params.hasOwnProperty("family")) {
            family = params.family;
        }
        if (params.hasOwnProperty("style")) {
            style = params.style;
        }
        //Setting font URL
        if (FONT_MAP.hasOwnProperty(family)) {
            defaultLogger.log("debug","Font family available !!!!!!!!!!");
            if (FONT_MAP[family][FONT_STYLE[style]]) {
                defaultLogger.log("debug","Font style available");
                fontURL = FONT_MAP[family][FONT_STYLE[style]];
            } else {
                defaultLogger.log("debug","Font style not available!");
                fontURL = Common.getDefaultFontURL();
            }
        } else {
            defaultLogger.log("debug","Font family not available");
            fontURL = Common.getDefaultFontURL();
        }
        
        return RESULT.XRE_S_OK;
    };
    /** Method to get font url **/
    this.getFontUrl = function() {
        return fontURL;
    };
    //Setting font properties
    this.setProperties(params);
};
//Inherting XREResource
FontResource.prototype = Object.create(XREResource.prototype);
FontResource.prototype.constructor = FontResource;
module.exports = FontResource;