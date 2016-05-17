console.log("top init.js");
var AppSceneContext = require('rcvrcore/AppSceneContext');
var RPCController = require('rcvrcore/rpcController');

var init = function(url) {
  var ctx = new AppSceneContext({scene:getScene(),packageUrl:url,rpcController:new RPCController()});
  ctx.loadScene();
}

if (_url) {
  console.log("Initalizing js ", _url);
  init(_url);
}

