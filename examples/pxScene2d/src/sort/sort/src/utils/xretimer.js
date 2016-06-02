/**
	Timer class to get time interval
**/
var XRETimer = function(name) {
    var _this = this;
    var startTime;
    var stopTime;
    //Start timer
    this.startTimer = function(name) {
        if (!XRETimer.isEnabled) {
            return false;
        }
        if (XRETimer.timer[name]) {
            return false;
        }
        var date = new Date();
        startTime = date.getTime();
        XRETimer.timer[name] = this;
        return true;
    };
    //To stop timer
    this.stopTimer = function() {
        var date = new Date();
        stopTime = date.getTime();
    };

    this.getDuration = function() {
        return stopTime - startTime;
    };
};

XRETimer.getTimer = function(name) {
    return XRETimer.timer[name];
};

XRETimer.timer = {};
//To enable Timer
XRETimer.isEnabled = false;

module.exports = XRETimer;
