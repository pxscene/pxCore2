/*

pxCore Copyright 2005-2020 John Robinson

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

function Thunder() {
  var nativeScene = null;

  this._setScene = function(scene) {
    if( nativeScene === null ) {
      nativeScene = scene;
    }
  };

  this.token = function() {
    return nativeScene.thunderToken();
  };

  this.close = function() {
    //nativeScene = null;
  }

  return this;
}

module.exports = Thunder;

