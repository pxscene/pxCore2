var initModule = require("rcvrcore/initGL")
var ESMLoader = require('rcvrcore/ESMLoader')

var loadAppUrl = function(url, _beginDrawing, _endDrawing, _view, _frameworkURL, _options) {
  var params = {}
  params.url = url;
  params._beginDrawing = _beginDrawing;
  params._endDrawing = _endDrawing;
  params._view = _view
  params._frameworkURL = _frameworkURL;
  params._options = _options;

  params.sparkscene = getScene("scene.1")
  params.makeReady = this.makeReady


  var llapp = new initModule.app(params)
  llapp.loadUrl();
}
