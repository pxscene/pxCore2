/************************************************************************
 * Name         : xreresource.js
 * Created      : 6/24/2015
 * Description  : handles common functionalities of a resource
 *************************************************************************/
var KEY = require('../utils/enum').KEY;
var XREObject = require("./xreObject.js");
var XREMouseEvent = require("../events/xreMouseEvent.js").XREMouseEvent;
var XREDragEvent = require("../events/xreMouseEvent.js").XREDragEvent;

var XREResource = function(id, app) {

    XREObject.call(this, id, app);
    var letter = "/^[a-zA-Z]+$/";
    /** Sets keysWithLatinChars with param values **/
    var keysWithLatinChars = function(nonShiftedKeyParam, nonShiftedLatinCharParam, shiftedKeyParam, shiftedLatinCharParam) {
        var nonShiftedKey = nonShiftedKeyParam;
        var nonShiftedLatinChar = null;
        var shiftedKey = null;
        var shiftedLatinChar = null;
        if (nonShiftedLatinCharParam !== undefined) {
            nonShiftedLatinChar = nonShiftedLatinCharParam;
        }
        if (shiftedKeyParam !== undefined) {
            shiftedKey = shiftedKeyParam;
        }
        if (shiftedLatinCharParam !== undefined) {
            shiftedLatinChar = shiftedLatinCharParam;
        }
    };
    /** Create KeyChar Struct **/
    var createKeyCharStruct = function(nonShiftedKey, nonShiftedLatinChar, shiftedKey, shiftedLatinChar) {
        var result = new keysWithLatinChars(nonShiftedKey, nonShiftedLatinChar, shiftedKey, shiftedLatinChar);
        return result;
    };
    /** Method to initialise key hash object **/
    var initKeysHash = function() {
        var hash = {};
        hash.F1 = createKeyCharStruct(KEY.F1);
        hash.F2 = createKeyCharStruct(KEY.F2);
        hash.F3 = createKeyCharStruct(KEY.F3);
        hash.F4 = createKeyCharStruct(KEY.F4);
        hash.F5 = createKeyCharStruct(KEY.F5);
        hash.F6 = createKeyCharStruct(KEY.F6);
        hash.F7 = createKeyCharStruct(KEY.F7);
        hash.F8 = createKeyCharStruct(KEY.F8);
        hash.F9 = createKeyCharStruct(KEY.F9);
        hash.F10 = createKeyCharStruct(KEY.F10);
        hash.F11 = createKeyCharStruct(KEY.F11);
        hash.F12 = createKeyCharStruct(KEY.F12);
        hash.F13 = createKeyCharStruct(KEY.F13);
        hash.F14 = createKeyCharStruct(KEY.F14);
        hash.F15 = createKeyCharStruct(KEY.F15);

        hash.LEFT = createKeyCharStruct(KEY.Left);
        hash.RIGHT = createKeyCharStruct(KEY.Right);
        hash.UP = createKeyCharStruct(KEY.Up);
        hash.DOWN = createKeyCharStruct(KEY.Down);
        hash.ENTER = createKeyCharStruct(KEY.Return);
        hash.BACKSPACE = createKeyCharStruct(KEY.Backspace);
        hash.SPACE = createKeyCharStruct(KEY.Space);
        hash.PREV = createKeyCharStruct(KEY.Backspace);
        hash.DELETE = createKeyCharStruct(KEY.Delete);
        hash.PAGE_UP = createKeyCharStruct(KEY.PageUp);
        hash.PAGE_DOWN = createKeyCharStruct(KEY.PageDown);

        hash.MUTE = createKeyCharStruct(KEY.VolumeMute);
        hash.VOLUME_DOWN = createKeyCharStruct(KEY.VolumeDown);
        hash.VOLUME_UP = createKeyCharStruct(KEY.VolumeUp);
        hash.CHANNEL_UP = createKeyCharStruct(KEY.MediaNext);
        hash.CHANNEL_DOWN = createKeyCharStruct(KEY.MediaPrevious);
        hash.STOP = createKeyCharStruct(KEY.MediaStop);
        hash.PLAY = createKeyCharStruct(KEY.MediaTogglePlayPause);
        hash.PAUSE = createKeyCharStruct(KEY.MediaTogglePlayPause);
        hash.REWIND = createKeyCharStruct(KEY.AudioRewind);
        hash.FAST_FORWARD = createKeyCharStruct(KEY.AudioForward);

        hash.NUMPAD = createKeyCharStruct(KEY.NumLock);
        hash.NUMPAD_ENTER = createKeyCharStruct(KEY.Enter);
        hash.NUMPAD_ADD = createKeyCharStruct(KEY.Plus, '+');
        hash.NUMPAD_DECIMAL = createKeyCharStruct(KEY.Period, '.');
        hash.NUMPAD_DIVIDE = createKeyCharStruct(KEY.Slash, '/');
        hash.NUMPAD_MULTIPLY = createKeyCharStruct(KEY.Asterisk, '*');
        hash.NUMPAD_SUBTRACT = createKeyCharStruct(KEY.Minus, '-');

        var qtKey = KEY.Zero;
        var symbol = 0;
        while (QtKey <= KEY.Nine) {
            hash["NUMPAD_" + symbol.toString()] = createKeyCharStruct(qtKey, symbol.toString());
            ++QtKey;
            ++symbol;
        }

        hash.NUMBER_0 = createKeyCharStruct(KEY.Zero, '0', KEY.ParenRight, ')');
        hash.NUMBER_1 = createKeyCharStruct(KEY.One, '1', KEY.Exclam, '!');
        hash.NUMBER_2 = createKeyCharStruct(KEY.Two, '2', KEY.At, '@');
        hash.NUMBER_3 = createKeyCharStruct(KEY.Three, '3', KEY.NumberSign, '#');
        hash.NUMBER_4 = createKeyCharStruct(KEY.Four, '4', KEY.Dollar, '$');
        hash.NUMBER_5 = createKeyCharStruct(KEY.Five, '5', KEY.Percent, '%');
        hash.NUMBER_6 = createKeyCharStruct(KEY.Six, '6', KEY.AsciiCircum, '^');
        hash.NUMBER_7 = createKeyCharStruct(KEY.Seven, '7', KEY.Ampersand, '&');
        hash.NUMBER_8 = createKeyCharStruct(KEY.Eight, '8', KEY.Asterisk, '*');
        hash.NUMBER_9 = createKeyCharStruct(KEY.Nine, '9', KEY.ParenLeft, '(');

        hash.PERIOD = createKeyCharStruct(KEY.Period, '.', KEY.Greater, '>');
        hash.QUOTE = createKeyCharStruct(KEY.Apostrophe, '\'', KEY.QuoteDbl, '\"');
        hash.SEMICOLON = createKeyCharStruct(KEY.Semicolon, ';', KEY.Colon, ':');
        hash.SLASH = createKeyCharStruct(KEY.Slash, '/', KEY.Question, '\?');
        hash.BACKQUOTE = createKeyCharStruct(KEY.QuoteLeft, '`', KEY.AsciiTilde, '~');
        hash.BACKSLASH = createKeyCharStruct(KEY.Backslash, '\\', KEY.Bar, '|');
        hash.COMMA = createKeyCharStruct(KEY.Comma, ',', KEY.Less, '<');
        hash.EQUAL = createKeyCharStruct(KEY.Equal, '=', KEY.Plus, '+');
        hash.LEFTBRACKET = createKeyCharStruct(KEY.BracketLeft, '[', KEY.BraceLeft, '{');
        hash.RIGHTBRACKET = createKeyCharStruct(KEY.BracketRight, ']', KEY.BraceRight, '}');
        hash.MINUS = createKeyCharStruct(KEY.Minus, '-', KEY.Underscore, '_');

        hash.NUMBER_0 = createKeyCharStruct(KEY.Zero, '0', KEY.ParenRight, ')');
        hash.NUMBER_1 = createKeyCharStruct(KEY.One, '1', KEY.Exclam, '!');
        hash.NUMBER_2 = createKeyCharStruct(KEY.Two, '2', KEY.At, '@');
        hash.NUMBER_3 = createKeyCharStruct(KEY.Three, '3', KEY.NumberSign, '#');
        hash.NUMBER_4 = createKeyCharStruct(KEY.Four, '4', KEY.Dollar, '$');
        hash.NUMBER_5 = createKeyCharStruct(KEY.Five, '5', KEY.Percent, '%');
        hash.NUMBER_6 = createKeyCharStruct(KEY.Six, '6', KEY.AsciiCircum, '^');
        hash.NUMBER_7 = createKeyCharStruct(KEY.Seven, '7', KEY.Ampersand, '&');
        hash.NUMBER_8 = createKeyCharStruct(KEY.Eight, '8', KEY.Asterisk, '*');
        hash.NUMBER_9 = createKeyCharStruct(KEY.Nine, '9', KEY.ParenLeft, '(');

        hash.PERIOD = createKeyCharStruct(KEY.Period, '.', KEY.Greater, '>');
        hash.QUOTE = createKeyCharStruct(KEY.Apostrophe, '\'', KEY.QuoteDbl, '\"');
        hash.SEMICOLON = createKeyCharStruct(KEY.Semicolon, ';', KEY.Colon, ':');
        hash.SLASH = createKeyCharStruct(KEY.Slash, '/', KEY.Question, '\?');
        hash.BACKQUOTE = createKeyCharStruct(KEY.QuoteLeft, '`', KEY.AsciiTilde, '~');
        hash.BACKSLASH = createKeyCharStruct(KEY.Backslash, '\\', KEY.Bar, '|');
        hash.COMMA = createKeyCharStruct(KEY.Comma, ',', KEY.Less, '<');
        hash.EQUAL = createKeyCharStruct(KEY.Equal, '=', KEY.Plus, '+');
        hash.LEFTBRACKET = createKeyCharStruct(KEY.BracketLeft, '[', KEY.BraceLeft, '{');
        hash.RIGHTBRACKET = createKeyCharStruct(KEY.BracketRight, ']', KEY.BraceRight, '}');
        hash.MINUS = createKeyCharStruct(KEY.Minus, '-', KEY.Underscore, '_');

        hash.COLOR_KEY_0 = createKeyCharStruct(KEY.Yellow);
        hash.COLOR_KEY_1 = createKeyCharStruct(KEY.Blue);
        hash.COLOR_KEY_2 = createKeyCharStruct(KEY.Red);
        hash.COLOR_KEY_3 = createKeyCharStruct(KEY.Green);

        return hash;

    };
    /** Method to get key hash */
    var getKeysHash = function() {
        if (Object.keys(keys2LatinCharsHash).length === 0) {
            keys2LatinCharsHash = initKeysHash();
        }
        return keys2LatinCharsHash;
    };
    /** Method to map key value from xre **/
    this.mapKeyFromXRE = function(xreKey, shifted) {
        var key = undefined;
        var text = "";
        var retObj = {};
        if (xreKey.length() == 1 && xreKey.charAt(0).value.match(letter)) {
            text = shifted ? xreKey.toUpperCase() : xreKey.toLowerCase();
            //   return Qt::Key(Qt::Key_A + (xreKey.at(0).toUpper().unicode() - 'A'));
            key = "Need to check";
        } else {
            var keyHash = getKeysHash();
            if (xreKey in keyHash) {
                key = shifted ? keyHash[xreKey].shiftedKey : keyHash[xreKey].nonShiftedKey;
                text = shifted ? keyHash[xreKey].shiftedLatinChar : keyHash[xreKey].nonShiftedLatinChar;
            } else {
                clog("key not remaped for web app: %s", xreKey);
            }
        }
        retObj.key = key;
        retObj.text = text;
        return retObj;
    };
    /** Returns flag whether resource handles view events **/
    this.handlesViewEvents = function() {
        return false;
    };
    /** Returns flag whether resource is keysink **/
    this.isKeySink = function() {
        return false;
    };
    /** Method to install event handlers **/
    this.installEventHandlers = function(params) {
        this.eventHandlers.install(params);
    };
    /**  Method to remap key event **/
    this.remapKeyEvent = function(keyEvent) {
        //TODO Functionality depends on a list variable which is only updated in flashresource and htnlviewresource
    };
    /** Method to get numeric property by name **/
    this.getNumericPropertyByName = function(){
        return 0.0;
    };

    this.registerMouseEvent = function(res, resObj, view) {
        clog("Inside mouse register");
        var isDragStarted = false;
        var startVerticalPosition = 0;
        var startHorizontalPosition = 0;
        resObj.on("onMouseDown", function(e) {
            var button = "unknown";
            if (e.flags == 4) {
                button == "RIGHT";
            } else if (e.flags == 1) {
                button = "LEFT";
            }
            var onMouseEvent = new XREMouseEvent(e.name, button, e.x, e.y, 0, 0x2);
            res.getEmitter().emit('Event', onMouseEvent, view, false);
        });

        resObj.on("onMouseUp", function(e) {
            if (isDragStarted) {
                var onDragEvent = new XREDragEvent("onMouseDragEnd", startHorizontalPosition, startVerticalPosition);
                res.getEmitter().emit('Event', onDragEvent, view, false);
                isDragStarted = false;
            } else {
                var button = "unknown";
                if (e.flags == 4) {
                    button == "RIGHT";
                } else if (e.flags == 1) {
                    button = "LEFT";
                }
                var onMouseEvent = new XREMouseEvent(e.name, button, e.x, e.y, 0, 0x2);
                res.getEmitter().emit('Event', onMouseEvent, view, false);
            }
        });


        resObj.on("onMouseEnter", function(e) {

            var onMouseEvent = new XREMouseEvent("onMouseIn", "LEFT", e.x, e.y, 0, 0x2);
            res.getEmitter().emit('Event', onMouseEvent, view, false);
        });
        resObj.on("onMouseLeave", function(e) {

            var onMouseEvent = new XREMouseEvent("onMouseOut", "LEFT", e.x, e.y, 0, 0x2);
            res.getEmitter().emit('Event', onMouseEvent, view, false);
        });

        resObj.on("onMouseDrag", function(e) {
            var onDragEvent;
            if (!isDragStarted) {
                clog("started========  " +e.startX + "  "+e.startY);
                startVerticalPosition = e.startX;
                startHorizontalPosition = e.startY;
                onDragEvent = new XREDragEvent("onMouseDragStart", e.startX, e.startY);
                res.getEmitter().emit('Event', onDragEvent, view, false);
                isDragStarted = true;
            } else {
                var deltaV = e.y - startVerticalPosition;
                var deltaH = e.x - startHorizontalPosition;
                clog("delta=== "+deltaV+ "  "+ deltaH);
                if (deltaV !== 0 && deltaH !== 0) {
                    onDragEvent = new XREDragEvent("onMouseDrag", deltaH, deltaV);
                    res.getEmitter().emit('Event', onDragEvent, view, false);
                    startVerticalPosition += deltaV;
                    startHorizontalPosition += deltaH;
                }
            }

        });

    };
};
XREResource.prototype = Object.create(XREObject.prototype); 
XREResource.prototype.constructor = XREResource;
module.exports = XREResource;