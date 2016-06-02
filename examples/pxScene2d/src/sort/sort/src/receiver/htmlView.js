var TEXT_TRUNC_STYLE = require("../utils/enum.js").TEXT_TRUNC_STYLE;
var HORIZONTAL_ALIGN_STYLE = require("../utils/enum.js").HORIZONTAL_ALIGN_STYLE;
var VERTICAL_ALIGN_STYLE = require("../utils/enum.js").VERTICAL_ALIGN_STYLE;

var XREHTMLView = function(scene) {
    var textObjArr = [];
    var viewObj = [];
    this.setResourceItem = function(index, textObj, color) {
        textObjArr[index] = textObj;
        //textObjArr[index].textColor = color;
        viewObj[index] = scene.create({
            t: "object"
        });
        textObjArr[index].parent = viewObj[index];
    };

    this.getView = function(view) {
        var parentView = scene.create({
            t: "object"
        });

        this.getResourceItem(view);
        for (var i = 0; i < textObjArr.length; i++) {
            viewObj[i].parent = parentView;
        }

        return parentView;
    };

    this.getResourceItem = function(view) {
        var options = view.getResourceOptions();
        var xpos = 0;

        textObjArr.forEach(function(textItem, j) {
            var targetText = textItem.text;
            if (options.textStopChar !== -1 && options.textStopChar !== -1 && options.textStartChar !== undefined && options.textStopChar !== undefined) {
                targetText = targetText.substring(options.textStartChar, options.textStopChar - options.textStartChar + 1);
            } else {
                targetText = targetText.substring(options.textStartChar);
            }
            textItem.text = targetText;
            if (options.textStartPos) {
                textObjArr[j].xStartPos = options.textStartPos[0];
            }
            /* Not checking width of view and text resource before doing wrap and truncate */
            if (options.textWrap === "WRAP") {
                textItem.wordWrap = true;
            } else {
                textItem.wordWrap = false;
            }

            switch (options.textTruncStyle) {
                case TEXT_TRUNC_STYLE.NONE:
                    textItem.truncation = 0;
                    break;
                case TEXT_TRUNC_STYLE.ELLIPSIS:
                    textItem.truncation = 1;
                    break;
                case TEXT_TRUNC_STYLE.ELLIPSIS_AT_WORD_BOUNDARY:
                    textItem.truncation = 2;
                    break;
                default:
                    break;
            }
            if (options.textTruncStyle !== TEXT_TRUNC_STYLE.NONE) {
                textItem.ellipsis = true;
            }
            textItem.verticalAlign = VERTICAL_ALIGN_STYLE[options.verticalAlign];
            textItem.horizontalAlign = HORIZONTAL_ALIGN_STYLE[options.horizontalAlign];
            var fontMetrics = textItem.getFontMetrics();
            var parentView = view.getViewObj();
            // TODO Need to confirm this setting
            textItem.clip = false;

            textItem.w = parentView.w;
            textItem.parent.w = parentView.w;
            textItem.h = parentView.h;
            textItem.parent.h = parentView.h;
            if (j == 0) {
                //textItem.x = parentView.x;
                //textItem.y = parentView.y;
                textItem.parent.x = xpos;

            } else {
                textObjArr[j - 1].ready.then(function() {
                    clog("Inside ready======");
                    clog("J value=======");
                    clog(j);
                    clog("Measure text====");
                    // clog(textObjArr[j - 1].measureText().lastChar.x);

                    var textMeasurement = textObjArr[j - 1].measureText();
                    var textStopPos = textMeasurement.lastChar;
                    var bound = textMeasurement.bounds.x2 - textMeasurement.bounds.x1;
                    clog("Last char position ");
                    clog("Width=========");
                    clog(bound);
                    clog(textMeasurement.bounds.x1);
                    clog("================");
                    clog(textStopPos.x + textObjArr[j - 1].x);
                    textObjArr[j].parent.x = textStopPos.x + textObjArr[j - 1].parent.x;
                    textObjArr[j - 1].parent.w = bound;

                });
            }

        });
    };

};
module.exports = XREHTMLView;
