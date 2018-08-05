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

/**
 * A Sample program to test proxy(rtRemoteProxy.js)
 * To run use cmd -  node testproxy.js.
 * Before running this program run the server 
 * using cmd - node server/sampleServer.js.
 * Above server resgister object "host_object", 
 * used here.
 */

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const RTRemoteProxy = require('../lib/RTRemoteProxy');
const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');
const helper = require('../lib/common/helper');
const logger = require('../lib/common/logger');

const resolve = new RTRemoteMulticastResolver('224.10.10.12', 10004);

resolve.start()
  .then(() => { resolve.locateObject('host_object')
  .then((uri) => RTRemoteConnectionManager.getObjectProxy(uri))	
  .then((rtHostObject) => {
    const proxyTest = () => {
      /*Creating a proxy object of rtHostObject*/
      var obj = new RTRemoteProxy(rtHostObject);
      
      Promise.resolve()
        .then(() => {
          /*Getting the property int32 of Object*/
          var result = obj.int32;
	        result().then(function(result) {
            logger.debug(`Get Value = ${result.value}`);
	        })
          .then(() => {
	        /*Calling twoIntNumberSum method*/
            var res = obj.twoIntNumberSum(RTValueHelper.create(20, RTValueType.INT32), RTValueHelper.create(10, RTValueType.INT32));
 	          res.then(function(res) {
              logger.debug(`Sum of two number : ${res.value}`);
	          })
            .then(() => {
	          /**Setting the property int32 and 
	           *checking wheather it is set or 
	           *not by getting the value
              */	
              var val = 10;
              obj.int32 = val;
	            setTimeout((function() {
                var result = obj.int32;
	              result().then(function(result) {
                 logger.debug(`Get Value = ${result.value}`);
	              })
              }), 1000);
            })
          })
        })
	      setTimeout((function() {return process.exit(0);}), 5000);
    }
    proxyTest();  
  }).catch(err => logger.error(err));
}).catch(err => logger.error(err));
