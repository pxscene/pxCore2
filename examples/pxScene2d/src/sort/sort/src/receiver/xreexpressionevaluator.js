/**
 * FileName    : xreexpressionevaluator.js
 * Created     : 07/03/2015
 * Description : Defines object to manipulate different expressions
 **/
/**
 * Constant values used for expression evaluation
 **/
var ks_rightParen = "}";
var ks_plus = "+";
var ks_minus = "-";
var ks_asterisk = "*";
var ks_forwardSlash = "/";
var ks_percent = "%";
var ks_MIN = "MIN";
var ks_MAX = "MAX";
var ks_CEIL = "CEIL";
var ks_FLOOR = "FLOOR";
var kc_dollarSign = '$';
var kc_leftParen = '{';
var kc_rightParen = '}';
var kc_hashSign = '#';
var kc_dot = '.';
var space = ' ';
var XRElogger = require('../utils/xrelogger.js');
var defaultLogger = XRElogger.getLogger("default");
var PXTextItem = require("./xretextresource.js").PXTextItem;
/**
 * Object holding different element type that can come in an expression
 **/
var elementType = {
    OPERATOR_ADD: 0,
    OPERATOR_SUB: 1,
    OPERATOR_MUL: 2,
    OPERATOR_DIV: 3,
    OPERATOR_MOD: 4,
    OPERATOR_MIN: 5,
    OPERATOR_MAX: 6,
    OPERATOR_CEIL: 7,
    OPERATOR_FLOOR: 8,
    OPERAND: 9
};
/**
 * Object expression evaluator
 **/
var ExpressionEvaluator = {

    /**
     *   Function to get element type
     **/
    getElementType: function(element) {
        switch (element) {
            case ks_plus:
                return elementType.OPERATOR_ADD;
            case ks_minus:
                return elementType.OPERATOR_SUB;
            case ks_asterisk:
                return elementType.OPERATOR_MUL;
            case ks_forwardSlash:
                return elementType.OPERATOR_DIV;
            case ks_percent:
                return elementType.OPERATOR_MOD;
            case ks_MIN:
                return elementType.OPERATOR_MIN;
            case ks_MAX:
                return elementType.OPERATOR_MAX;
            case ks_CEIL:
                return elementType.OPERATOR_CEIL;
            case ks_FLOOR:
                return elementType.OPERATOR_FLOOR;
            default:
                return elementType.OPERAND;
        }
    },
    /**
     *   Function to get element value
     **/
    getElementValue: function(canvas, element) {
        if (element.charAt(0) === kc_dollarSign || element.charAt(0) === kc_hashSign) {
            var targetId = 0;
            var property = undefined;
            var dotIdx = element.indexOf(kc_dot);
            if (dotIdx > 1) {
                targetId = parseInt(element.substring(1, dotIdx));
            }
            property = element.substring(dotIdx + 1, element.length);
            if (element.charAt(0) == kc_dollarSign) {
                var targetView = canvas.getView(targetId);
                
                if (targetView) {
                    return targetView.getNumericPropertyByName(property);
                }
            }
            var targetResource = canvas.getApp().getResource(targetId);
            if (targetResource) {
                return targetResource.getNumericPropertyByName(property);
            }

        }
        return parseFloat(element);
    },
    /**
     *   Function to evaluate expression
     **/
    evaluateExpression: function(canvas, exp) {
        var operands = [];
        var cache = {};
        if (exp.charAt(0) != kc_leftParen) {
            defaultLogger.log("debug","invalid expression " + exp + " does not begin with '{'");
            return 0.0;
        }

        var n = 0;
        var begin = 1;
        while (begin < exp.length && exp.charAt(begin) != kc_rightParen) {
            n++;
            var expVal = exp.slice(begin, exp.length);
            var end = exp.indexOf(space, begin);
            var token = exp.substr(begin, (end - begin));

            if (token) {
                if (token == ks_rightParen) {
                    break;
                }

                var eType = this.getElementType(token);
                if (elementType.OPERAND == eType) {
                    var d = 0.0;
                    if (cache[token]) {
                        d = cache[token];
                    } else {
                        d = this.getElementValue(canvas, token);
                        cache[token] = d;
                    }
                    operands.push(d);
                } else {
                    var op1 = parseFloat(operands.pop());

                    switch (eType) {
                        case elementType.OPERATOR_ADD:
                            var op2 = (operands.pop());
                            var result = op2 + op1;
                            operands.push(parseFloat(result));
                            //operands.push(parseFloat(operands.pop()) + op1);
                            break;
                        case elementType.OPERATOR_SUB:
                            op2 = (operands.pop());
                            result = op2 - op1;
                            operands.push(parseFloat(result));
                            //operands.push(parseFloat(operands.pop()) - op1);
                            break;
                        case elementType.OPERATOR_MUL:
                            operands.push(parseFloat(operands.pop()) * op1);
                            break;
                        case elementType.OPERATOR_DIV:
                            if (op1 !== 0) {
                                operands.push(parseFloat(operands.pop()) / op1);
                            } else {
                                operands.pop();
                                operands.push(parseFloat(0));
                            }
                            break;
                        case elementType.OPERATOR_MOD:
                            if (op1 !== 0) {
                                operands.push((parseFloat(operands.pop())) % (op1));
                            } else {
                                operands.pop();
                                operands.push(parseFloat(0));
                            }
                            break;
                        case elementType.OPERATOR_MIN:
                            operands.push(Math.min(parseFloat(operands.pop()), op1));
                            break;
                        case elementType.OPERATOR_MAX:
                            operands.push(Math.max(parseFloat(operands.pop()), op1));
                            break;
                        case elementType.OPERATOR_CEIL:
                            operands.push(Math.ceil(op1));
                            break;
                        case elementType.OPERATOR_FLOOR:
                            operands.push(Math.floor(op1));
                            break;
                        default:
                            defaultLogger.log("debug","Invalid operator" + eType);
                            break;
                    }
                }
            }

            while (exp.charAt(end) == space){
                end++;
            }
            begin = end;
        }
        //Check
        defaultLogger.log("debug","evaluateExpression : Returning value " + operands[0]);
        return operands[0];
    },
    /**
     *   Function  to evaluate numeric property based on type of property
     **/
    evaluateNumericProperty: function(app, property) {
        //Assuming only string need to be evaluated
        if (typeof property === 'string') {
            return this.evaluateExpression(app.getCanvas(), property);
        } else {
            //Not considering data type 
            return property;
        }

    }

};
module.exports = ExpressionEvaluator;