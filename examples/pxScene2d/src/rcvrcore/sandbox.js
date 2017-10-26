
// console.log("\n\n############### SAND BOX stuff - Only once !\n\n");

// These vars are populated/copied into the Contexified Context... aka Clone !
//

var vm            = require('vm.js');
var require       = this.require;

var _sandboxStuff = [ "console", "vm", "require" ];
