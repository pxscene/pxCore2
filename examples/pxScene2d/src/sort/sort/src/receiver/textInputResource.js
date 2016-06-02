var RESULT = require('../utils/enum').RESULT;
var XREResource = require('./xreresource.js');
var Common = require("../utils/common.js");
var FontResource = require('./xrefontresource.js');
var XRElogger = require('../utils/xrelogger.js');
var COnChangedEvent = require("../events/resourceEvent.js").COnChangedEvent;
var XREEvent = require("./xreEvent.js");
var XREConnection = require("../protocol/xreconnection.js");

var TextInputResource = function(id, app, params, scene) {
    clog("Inside===");
    XREResource.call(this, id, app);
    clog("After Inside===")
    var _this = this;
    var font = 0;
    var color = 0xFF000000;
    var size = 12;
    var mask = false;
    var text = "";
    var count = 1;
    var prompt = scene.create({
        t: "text2"
    });
    clog("Before setProperties");
    this.setProperties = function(params) {
        clog("setProperties===")
        if (params.hasOwnProperty('text')) {
            text = params.text;
        }
        if (params.hasOwnProperty('maskText')) {
            mask = params.maskText;

        }
        if (params.hasOwnProperty('font')) {
            font = params.font;
        }
        if (params.hasOwnProperty('color')) {
            color = params.color;
        }
        if (params.hasOwnProperty('size')) {
            size = params.size;
        }
        RESULT.XRE_S_OK;
    };

    this.getProperty = function(name) {
        var returnVal;
        switch (name) {
            case "text":
                returnVal = text;
                break;
            case "maskText":
                returnVal = mask;
                break;
            case "font":
                returnVal = font;
                break;
            case "color":
                returnVal = color;
                break;
            case "size":
                returnVal = size;
                break;
            default:
                returnVal = this.GetProperty(name);
                break;
        }
        return returnVal;
    };

    this.createPXItem = function(view) {
        var inputbg = scene.createRectangle({
            fillColor: 0xffffffff,
            w: view.getViewObj().w,
            h: view.getViewObj().h
        });

        prompt.pixelSize = parseInt(this.getProperty('size'));
        prompt.textColor = Common.processColor(this.getProperty('color'));
        prompt.mask = this.getProperty('maskText');
        prompt.xStartPos = view.getViewObj().x;
        prompt.xStopPos = view.getViewObj().x;

        if (prompt.mask == "false") {
            prompt.text = this.getProperty('text');
            text = prompt.text;
        } else {
            var promptText = this.getProperty('text');
            for (var index = 0; index < promptText.length; index++) {
                promptText = promptText.replace(promptText.charAt(index), '\u25CF');
            }
            prompt.text = promptText;
            text = prompt.text;
        }
        if (font = this.getProperty('font')) {
            var font = this.GetApp().getResource(font);
            if (font instanceof FontResource) {
                prompt.faceURL = font.getFontUrl();
            }
        }
        prompt.on("onChar", function(e) {
            clog("Inside onChar=======");
            setText(e.charCode);
        });
        var item = new PXTextInputItem(view, prompt, inputbg, this);
        return item;
    };

    var setText = function(inputText) {
        if (count == 1) {
            text = "";
            count++;
        }
        /*Backspace*/
        if (inputText == 8) {
            prompt.text = (prompt.text).substring(0, (prompt.text).length - 1);
            text = text.substring(0, text.length - 1);
        } else if (inputText == 13 || inputText == 9) {

        } else {
            if (prompt.mask == "false") {
                text = text + String.fromCharCode(inputText);
                prompt.text = text;
            } else {
                text = text + String.fromCharCode(inputText);
                prompt.text = "";
                for (var i = 0; i < text.length; i++) {
                    prompt.text = prompt.text + "\u25CF";
                }
            }
        }
        notifyChanged(text);
    };
    var notifyChanged = function(text) {
        clog("Inside Notify changed=====");
        clog(text);
        var event = new COnChangedEvent(text);
        _this.getEmitter().emit('Event', event, _this);

    };
    this.setProperties(params);
};
TextInputResource.prototype = Object.create(XREResource.prototype);
TextInputResource.prototype.constructor = TextInputResource;

var PXTextInputItem = function(view, prompt, inputbg) {
    clog("Inside PXTextInputItem...")
    this.setProperties = function() {
        var viewObj = view.getViewObj();
        prompt.w = viewObj.w;
        prompt.h = viewObj.h;
        prompt.parent = inputbg;
        inputbg.parent = view.getViewObj();
    };
    this.getSceneObject = function() {
        return prompt;
    };
    this.updateResource = function() {
        return;
    };
    this.setProperties();
}

module.exports = {
    TextInputResource: TextInputResource,
    PXTextInputItem: PXTextInputItem
}
