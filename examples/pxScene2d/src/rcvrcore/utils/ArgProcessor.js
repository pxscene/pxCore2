var Logger = require('../Logger.js').Logger;
var log = new Logger('XModule');


function ArgProcessor() {
}

function evalArgument(arg, processArgs) {
  var index = arg.indexOf("=");
  if( index != -1 ) {
    var parts = [];
    parts.push(arg.substring(0,index));
    parts.push(arg.substr(index+1));
    if( parts.length == 2 ) {
      log.message(2, "key=" + parts[0] + ", value=" + parts[1]);
      processArgs[parts[0]] = parts[1];
    } else {
      console.error("Expected key/value pair: " + arg)
    }
  } else {
    console.error("Unknown single argument.  Expected key/value pair: " + arg);
  }
}
module.exports = function(argv, argDefinitions) {
  var processArgs = {};
  for(var k=2; k < argv.length; ++k) {
    log.message(2, "Eval argument[" + k + "]: " + argv[k]);
    evalArgument(argv[k], processArgs);
  }

  if( argDefinitions != null && argDefinitions != 'undefined' ) {
    for(var key in argDefinitions) {
      var definition = argDefinitions[key];
      log.message(2, "Definition[" + key + "]");
      if( !processArgs.hasOwnProperty(key) ) {
        if( definition.hasOwnProperty('required') && definition.required===true ) {
          var errMsg = "\n\nError: Missing argument named " + key + ": " + definition.help;
          throw new Error(errMsg);
        } else {
          if( definition.hasOwnProperty('default') ) {
            processArgs[key] = definition.default;
            log.message(2, "Setting [" + key + "] to default vaue: " + definition.default);
          }
        }
      }
    }
  }

  return processArgs;

};
