px.import("px:scene.1.js").then( function ready(scene) {

var root = scene.root;

var eventHandlers = {};

var text = scene.create({t:'text',text:'My text value',parent:root,w:200,h:200});

var emitEvent = function(name, value) {
  if(eventHandlers.name !== undefined) {
    for( var func of eventHandlers.name)
      func(value);
  }
}
module.exports.setText = function(value) {
  console.log("test_apiEventChild setText()");
  text.text = value;
  emitEvent("onTextChange", value);
}

module.exports.getText = function() {
  console.log("This message is from the simpleTestApiChild's getText API!");
  return text.text;
}

module.exports.on = function(name,callback) {
  if( eventHandlers.name === undefined) {
    eventHandlers.name = [];
  }
  eventHandlers.name.push(callback);
}

 
  }).catch( function importFailed(err){
  console.error("Import for test_apiEventChild.js failed: " + err)
});
