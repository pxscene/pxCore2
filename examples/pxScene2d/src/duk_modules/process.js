"use strict";

var schedule = null;

function setScheduler(arg) {
    schedule = arg;
}

function nextTick(cb) {
    schedule(cb);
}

function _tickCallback() {
}

module.exports = {
    setScheduler: setScheduler,
    nextTick: nextTick,
    _tickCallback: _tickCallback,
}
