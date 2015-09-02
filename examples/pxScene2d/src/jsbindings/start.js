var setLoggingLevel = require('rcvrcore/Logger').setLoggingLevel;

var argDefinitions = {screenWidth:{required:false, default:1280, help:"Specifies the screen width"},
  screenHeight:{required:false, default:720, help:"Specifies the screen height"},
  logLevel:{required:false, default:1, help:"Specifies the logging level (0-N)"},
  url:{required:true, help:"Specifies JavaScript file to load"},
  useNodeBasedImports:{required:false, help:"If true then imports don't need to specify file extensions"}
};

var argProcessor = require("rcvrcore/utils/ArgProcessor");
var processArgs = argProcessor(process.argv, argDefinitions);

if( processArgs.hasOwnProperty('logLevel')) {
  setLoggingLevel(processArgs['logLevel']);
}

// Create root object and send it the base URI of the xre2 framework modules
var pxRoot = require('rcvrcore/pxRoot')(0, 0, processArgs['screenWidth'], processArgs['screenHeight']);

pxRoot.addScene({url:processArgs['url'],w:processArgs['screenWidth'],h:processArgs['screenHeight'],
  useNodeBasedImports:processArgs['useNodeBasedImports']});
pxRoot.setOriginalUrl(processArgs['url']);
