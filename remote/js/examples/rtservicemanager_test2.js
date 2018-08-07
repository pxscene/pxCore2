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

/*
 * Commands to test: 
 * To get name of the service : node rtservicemanager.js getName
 * To get current resolution  : node rtservicemanager.js getCurrentResolution HDMI0
 * To set current resolution  : node rtservicemanager.js setCurrentResolution HDMI0 480p/720p
*/

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');
const helper = require('../lib/common/helper');
const logger = require('../lib/common/logger');
const ip = require('ip');

const resolve = new RTRemoteMulticastResolver('224.10.0.12', 10004);

resolve.start()
  .then(() => { resolve.locateObject('rtServiceManager')
  .then(uri => RTRemoteConnectionManager.getObjectProxy(uri))
  .then((rtServiceManagerObj) => {
      const doDisplaySettingTest = () => {  
        Promise.resolve()
        .then(() => createService(
        rtServiceManagerObj, 'createService',
        RTValueHelper.create(ip.address(), RTValueType.STRING),
        RTValueHelper.create('org.openrdk.DisplaySettings', RTValueType.STRING)
        ))
        .then((displaySettingObj) => {
          if(process.argv[2] == "getName") {
            getName(displaySettingObj.value, 'getName')
	    .then((serviceName) => {
              console.log("Name of the service : %s", serviceName.value);
            });
            setTimeout((function() {return process.exit(0);}), 1000);
          } else if(process.argv[2] == "getCurrentResolution") {
 	    if(process.argv[3].length != 0) {
              getResolution(
	      displaySettingObj.value, 'getCurrentResolution',
              RTValueHelper.create(process.argv[3], RTValueType.STRING))
	      .then((resolution) => {
                console.log("Resolution for %s : %s", process.argv[3], resolution.value);
	      });
            }
            setTimeout((function() {return process.exit(0);}), 1000);
          } else if(process.argv[2] == "setCurrentResolution") {
 	    if(process.argv[3].length != 0 && process.argv[4].length != 0) {
              setResolution(
	      displaySettingObj.value, 'setCurrentResolution',
              RTValueHelper.create(process.argv[3], RTValueType.STRING),
              RTValueHelper.create(process.argv[4], RTValueType.STRING))
	      .then((res) => {
	        if(res.value) {	
                  console.log("Successfully set %s for %s.", process.argv[4], process.argv[3]);
	        } else {
                  console.log("Failed to set %s for %s", process.argv[4], process.argv[3]);
                }
              });
            setTimeout((function() {return process.exit(0);}), 5000);
	    }
          } else {
            console.log("Please enter valid argument");
            console.log("Exiting.....");
            return process.exit(0);
           }
          })
          .catch(err => logger.error(err));
    }
    doDisplaySettingTest();
  })
  .catch(err => logger.error(err));
})
.catch(err => logger.error(err));

/**
 * create sevice method
 * @param rtObject the remote object
 * @param methodName the method name
 * @param args the call function args
 * @return {Promise<void>} the promise when done
 */

function createService(rtObject, methodName, ...args) {
  return rtObject.sendReturns(methodName, ...args)
}

function getName(rtObject, methodName, ...args) {
  return rtObject.sendReturns(methodName, ...args)
}

function getResolution(rtObject, methodName, ...args) {
  return rtObject.sendReturns(methodName, ...args)
}

function setResolution(rtObject, methodName, ...args) {
  return rtObject.sendReturns(methodName, ...args)
}

