"use strict";

function get(url, cb) {
    return httpGet(url, cb);
}

module.exports = {
    'get': get,
}
