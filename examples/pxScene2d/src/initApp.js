/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

var initModule = require("rcvrcore/initGL")
var ESMLoader = require('rcvrcore/ESMLoader')

var loadAppUrl = function(url, _beginDrawing, _endDrawing, _view, _frameworkURL, _options, _sparkHttp) {
  var params = {}
  params.url = url;
  params._beginDrawing = _beginDrawing;
  params._endDrawing = _endDrawing;
  params._view = _view
  params._frameworkURL = _frameworkURL;
  params._options = _options;
  params._sparkHttp = _sparkHttp
  params.sparkscene = getScene("scene.1")
  params.makeReady = this.makeReady


  var llapp = new initModule.app(params)
  llapp.loadUrl();
}
