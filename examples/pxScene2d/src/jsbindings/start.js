var setLoggingLevel = require('rcvrcore/Logger').setLoggingLevel;

var argDefinitions = {screenWidth:{required:false, default:1280, help:"Specifies the screen width"},
  screenHeight:{required:false, default:720, help:"Specifies the screen height"},
  logLevel:{required:false, default:1, help:"Specifies the logging level (0-N)"},
  url:{required:true, help:"Specifies JavaScript file to load"}
};

var argProcessor = require("rcvrcore/utils/ArgProcessor");
var processArgs = argProcessor(process.argv, argDefinitions);

if( processArgs.hasOwnProperty('logLevel')) {
  setLoggingLevel(processArgs['logLevel']);
}

// Create root object and send it the base URI of the xre2 framework modules
var pxRoot = require('rcvrcore/PxRoot')(0, 0, processArgs['screenWidth'], processArgs['screenHeight']);

pxRoot.addScene({url:processArgs['url'],w:processArgs['screenWidth'],h:processArgs['screenHeight']});
pxRoot.setOriginalUrl(processArgs['url']);
