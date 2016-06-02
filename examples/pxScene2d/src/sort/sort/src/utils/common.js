/**
 * FileName    : common.js
 * Created     : 06/18/2015
 * Description : Defines class to handle all common functionalities
 **/
var Constants = require("./config.js");
var common = {
    /**
     * This method process the colorValue obtained and return the processed color
     **/
    processColor: function(colorValue) {
        var color = colorValue >>> 0;
        var alpha = (color & 0xff000000) >>> 24;
        if (alpha.toString().length == 1) {
            alpha = "0" + alpha;
        }
        var rgb = (color & 0x00ffffff);
        while (rgb.toString().length < 6) {
            rgb = "0" + rgb;
        }
        var colorCheck = '0x' + rgb.toString(16) + alpha.toString(16);
        return (parseInt(colorCheck, 16));
    },
    /**
     *  Method to get default font Url
     **/
    getDefaultFontURL: function() {
        return "DejaVuSerif.ttf";
    }
};
module.exports = common;
