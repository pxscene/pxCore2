'use strict';

var https = require('https');

module.exports.globalAgent = https.globalAgent;
module.exports.Agent = https.Agent;
module.exports.request = https.request;
module.exports.get = https.get;
