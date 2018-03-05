"use strict";

function get(url, cb) {
    return httpGet(url, cb);
}

function request(options) {
    return _httpRequest(options);
}

module.exports = {
    'get': get,
    'request': request,
}
