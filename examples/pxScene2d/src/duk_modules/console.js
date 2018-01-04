"use strict";

function trace(msg) {
    print("[LOG TRACE] " + msg);
}

function warn(msg) {
    print("[LOG WARN] " + msg);
}

function error(msg) {
    print("[LOG ERROR] " + msg);
}

function log(msg) {
    print("[LOG LOG] " + msg);
}

function info(msg) {
    print("[LOG INFO] " + msg);
}

module.exports = {
    trace: trace,
    warn: warn,
    error: error,
    log: log,
    info: info,
}
