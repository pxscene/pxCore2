/**
 * FileName    : stylesheetResource.js
 * Created     : 15/07/2015
 * Description : Defines class to handle Stylesheet resource
 **/

var XREResource = require('./xreresource.js');
var RESULT = require('../utils/enum').RESULT;

var StylesheetResource = function(id, app, params) {
    clog("Inside StylesheetResource");
    XREResource.call(this, id, app);
    var stylesheet;
    this.setProperties = function() {
        stylesheet = params.css;
        stylesheet.replace("<em>", "<strong>");
        stylesheet.replace("</em>", "</strong>");
        return RESULT.XRE_S_OK;
    };
    this.getProperty = function(name) {
        var returnVal;
        if ("css" == name) {
            returnVal = stylesheet;
            return returnVal;
        }
        returnVal = XREResource.getProperty(name);
        return returnVal;

    };
    this.setProperties(params);
};
StylesheetResource.prototype = Object.create(XREResource.prototype);
StylesheetResource.prototype.constructor = StylesheetResource;
module.exports = StylesheetResource;
