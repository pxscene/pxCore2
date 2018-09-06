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
 * A Sample program to test different method 
 * of servicemanager in box using proxy object.	
 */

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const RTRemoteProxy = require('../lib/RTRemoteProxy');
const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');
const helper = require('../lib/common/helper');
const logger = require('../lib/common/logger');
const ip = require('ip');

const resolve = new RTRemoteMulticastResolver('224.10.0.12', 10004);

resolve.start()
  .then(() => { resolve.locateObject('rtServiceManager')
  .then((uri) => RTRemoteConnectionManager.getObjectProxy(uri))	
  .then((rtServiceManager) => {
    const serviceTest = () => {
      Promise.resolve()
        .then(() => {
          /*Creating a proxy object of rtServiceManger object*/ 
	        var rtServiceManagerProxyObj = new RTRemoteProxy(rtServiceManager);
	  
          /*Creating a service object using create service method of servicemanager*/ 
	        var displaySettingServiceObj = rtServiceManagerProxyObj.createService_2(
        		RTValueHelper.create(ip.address(), RTValueType.STRING),
        		RTValueHelper.create('org.openrdk.DisplaySettings', RTValueType.STRING));
          displaySettingServiceObj.then((displaySettingServiceObj) => {
          
            /*Creating a proxy object of service object*/ 
            var service = new RTRemoteProxy(displaySettingServiceObj.value);
               
            /*Calling method base on user input*/
	          if(process.argv.length == 3) {
              switch(process.argv[2]) {
                case "getName" :
	                var name = service.getName();	
	                name.then(function (name) {logger.debug(`Name of the service : ${name.value}`);});
                  break;
                case "getApiVersionNumber" :
	                var version = service.getApiVersionNumber();	
	                version.then(function (version) {logger.debug(`Version of the service : ${version.value}`);});
                  break;
	              case "unregisterEvents" :
                  var result = service.unregisterEvents();
	                result.then(function (result) {logger.debug(`Event unresgistered ? ${result.value}`);});
                  break;
	              case "registerForAsyncResponses" :
                  var result = service.registerForAsyncResponses();
	                result.then(function (result) {logger.debug(`Register For Async Responses ? ${result.value}`);});
                  break;
	              case "unregisterForAsyncResponses" :
                  var result = service.unregisterForAsyncResponses();
	                result.then(function (result) {logger.debug(`Unregister For Async Responses ? ${result.value}`);});
                  break;
                case "getSharedObjectsList" :
	                var list = service.getSharedObjectsList();	
	                list.then(function (list) {logger.debug(`List of Shared object : ${list.value}`);});
                  break;
                case "getQuirks" :
	                var quirks = service.getQuirks();	
	                quirks.then(function (quirks) {logger.debug(`Quirks : ${quirks.value}`);});
                  break;
                case "setApplicationManagerDelegate" :
	                var result = service.setApplicationManagerDelegate();	
	                result.then(function (result) {logger.debug(`Result : ${result.value}`);});
                  break;
                case "applicationManagerDelegate" :
	                var result = service.applicationManagerDelegate();	
	                result.then(function (result) {logger.debug(`Result : ${result.value}`);});
                  break;
                case "getConnectedVideoDisplays" :
	                var displays = service.getConnectedVideoDisplays();
	                displays.then(function (displays) {logger.debug(`Displays : ${displays.value}`);});
                  break;
	              case "getConnectedAudioPorts" :
	                var ports = service.getConnectedAudioPorts();
	                ports.then(function (ports) {logger.debug(`Ports : ${ports.value}`);});
                  break;
	              case "getSupportedResolutions" :
                  var resolution = service.getSupportedResolutions();	
	                resolution.then(function (resolution) {logger.debug(`Resolution : ${resolution.value}`);});
                  break;
	              case "getSupportedVideoDisplays" :
	                var displays = service.getSupportedVideoDisplays();
	                displays.then(function (displays) {logger.debug(`Displays : ${displays.value}`);});
                  break;
	              case "getSupportedAudioPorts" :
	                var ports = service.getConnectedAudioPorts();
	                ports.then(function (ports) {logger.debug(`Ports : ${ports.value}`);});
                  break;
	              case "getZoomSetting" :
	                var zoom = service.getZoomSetting();
	                zoom.then(function (zoom) {logger.debug(`Zoom : ${zoom.value}`);});
                  break;
	              case "getSoundMode" :
	                var mode = service.getSoundMode();
	                mode.then(function (mode) {logger.debug(`Mode : ${mode.value}`);});
                  break;
	              case "readEDID" :
	                var edid = service.readEDID();
	                edid.then(function (edid) {logger.debug(`EDID : ${ edid.value}`);});
                  break;
	              case "readHostEDID" :
	                var edid = service.readHostEDID();
	                edid.then(function (edid) {logger.debug(`Host EDID : ${edid.value}`);});
                  break;
	              case "getSupportedSettopResolutions" :
                  var resolution = service.getSupportedSettopResolutions();	
	                resolution.then(function (resolution) {logger.debug(`Resolution : ${resolution.value}`);});
                  break;
	              case "getSettopHDRSupport" :
                  var resolution = service.getSettopHDRSupport();	
	                resolution.then(function (resolution) {logger.debug(`Resolution : ${resolution.value}`);});
                  break;
	              case "getTvHDRSupport" :
                  var resolution = service.getTvHDRSupport();	
	                resolution.then(function (resolution) {logger.debug(`Resolution : ${resolution.value}`);});
                  break;
	              default:	
	                logger.debug("Invalid argument or no such method");
                  break;
              }
	          } else if(process.argv.length == 4) {
              switch(process.argv[2]) {
	              case "setApiVersionNumber" :
                  service.setApiVersionNumber(RTValueHelper.create(process.argv[3], RTValueType.STRING));
                  logger.debug("Api Version number set");
                  break;
	              case "setProperties" :
                  var result = service.setProperties(RTValueHelper.create(process.argv[3], RTValueType.STRING));
	                result.then(function (result) {logger.debug(`Properties set ? ${result.value}`);});
                  break;
	              case "getProperties" :
                  var properties = service.getProperties(RTValueHelper.create(process.argv[3], RTValueType.STRING));
	                properties.then(function (properties) {logger.debug(`Properties : ${properties.value}`);});
                  break;
	              case "registerForEvents" :
                  var result = service.registerForEvents(RTValueHelper.create(process.argv[3], RTValueType.STRING));
	                result.then(function (result) {logger.debug(`Event resgistered ? ${result.value}`);});
                  break;
	              case "unregisterEventsList" :
                  var list = service.unregisterEventsList(RTValueHelper.create(process.argv[3], RTValueType.STRING));
	                list.then(function (list) {logger.debug(`List of unresgistered Event : ${list.value}`);});
                  break;
	              case "getCurrentResolution" :
                  var resolution = service.getCurrentResolution(RTValueHelper.create(process.argv[3], RTValueType.STRING));
                  resolution.then(function (resolution) {logger.debug(`Resolution : ${resolution.value}`);});
                  break;
	              case "getSupportedResolutions" :
                  var resolution = service.getSupportedResolutions(RTValueHelper.create(process.argv[3], RTValueType.STRING));
                  resolution.then(function (resolution) {logger.debug(`Resolution : ${resolution.value}`);});
                  break;
	              case "getSupportedTvResolutions" :
                  var resolution = service.getSupportedTvResolutions(RTValueHelper.create(process.argv[3], RTValueType.STRING));
                  resolution.then(function (resolution) {logger.debug(`Resolution : ${resolution.value}`);});
                  break;
	              case "getSupportedAudioModes" :
                  var modes = service.getCurrentResolution(RTValueHelper.create(process.argv[3], RTValueType.STRING));	
	                modes.then(function (modes) {logger.debug(`Resolution : ${modes.value}`);});
                  break;
	              case "setZoomSetting" :
	                var result = service.setZoomSetting(RTValueHelper.create(process.argv[3], RTValueType.STRING));
	                result.then(function (result) {logger.debug(`Zoom set ? ${result.value}`);});
                  break;
	              case "getActiveInput" :
	                var activeInput = service.getActiveInput(RTValueHelper.create(process.argv[3], RTValueType.STRING));
	                activeInput.then(function (activeInput) {logger.debug(`Zoom : ${activeInput.value}`);});
                  break;
	              default:
	                logger.debug("Invalid argument or no such method");
                  break;
              }
	          } else if(process.argv.length == 5) {
              switch(process.argv[2]) {
	              case "setCurrentResolution" :
                  var result = service.setCurrentResolution(
                               RTValueHelper.create(process.argv[3], RTValueType.STRING),
                               RTValueHelper.create(process.argv[4], RTValueType.STRING));	
	                result.then(function (result) {logger.debug(`Resolution set ?  ${result.value}`);});
                  break;
	              case "setSoundMode" :
                  var result = service.setSoundMode(
                               RTValueHelper.create(process.argv[3], RTValueType.STRING),
                               RTValueHelper.create(process.argv[4], RTValueType.STRING));	
	                result.then(function (result) {logger.debug(`Resolution set ?  ${result.value}`);});
                  break;
	              default:
	                logger.debug("Invalid argument or no such method");
                  break;
              }
            } else {
	            logger.debug("Invalid argument");
	            return process.exit(0); 
            }
            setTimeout((function() {return process.exit(0);}), 1000);
	        });
        }); 
    }
    serviceTest();  
  }).catch(err => logger.error(err));
}).catch(err => logger.error(err));
