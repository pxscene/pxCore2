"use strict";

var schedule = null;

function setScheduler(arg) {
    schedule = arg;
}

function nextTick(cb) {
    schedule(cb);
}

module.exports = {
    setScheduler: setScheduler,
    nextTick: nextTick,
}
