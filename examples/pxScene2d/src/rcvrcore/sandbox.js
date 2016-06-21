
// console.log("\n\n############### SAND BOX stuff - Only once !\n\n");

// These vars are populated/copied into the Contexified Context... aka Clone !
//


// Alias vars
var urlModule         = require("url");
var queryStringModule = require("querystring");
var xmodule           = require('rcvrcore/XModule').XModule;

var require     = require;
var runtime     = this;

var setTimeout  = this.setTimeout;
var setInterval = this.setInterval;

var _sandboxStuff = [ "console", "Buffer", "setTimeout", "setInterval",
                      "clearInterval", "urlModule", "queryStringModule", "xmodule", "runtime", "require" ];
