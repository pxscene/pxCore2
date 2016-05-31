var AppSceneContext = require('rcvrcore/AppSceneContext');
var RPCController = require('rcvrcore/rpcController');

function loadUrl(url) {
  var ctx = new AppSceneContext({scene:getScene("scene.1"),makeReady:makeReady,packageUrl:url,rpcController:new RPCController()});
  ctx.loadScene();
}
