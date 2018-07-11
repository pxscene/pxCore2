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
 * the rt remote lib export entry
 *
 * @author      TCSCODER
 * @version     1.0
 */


const helper = require('./common/helper');
const logger = require('./common/logger');
const RTConst = require('./RTConst');
const RTEnvironment = require('./RTEnvironment');
const RTException = require('./RTException');
const RTMessageHelper = require('./RTMessageHelper');
const RTRemoteClientConnection = require('./RTRemoteClientConnection');
const RTRemoteConnectionManager = require('./RTRemoteConnectionManager');
const RTRemoteMessageType = require('./RTRemoteMessageType');
const RTRemoteMulticastResolver = require('./RTRemoteMulticastResolver');
const RTRemoteObject = require('./RTRemoteObject');
const RTRemoteProtocol = require('./RTRemoteProtocol');
const RTRemoteProxy = require('./RTRemoteProxy');
const RTRemoteSerializer = require('./RTRemoteSerializer');
const RTRemoteServer = require('./RTRemoteServer');
const RTRemoteTask = require('./RTRemoteTask');
const RTRemoteTCPTransport = require('./RTRemoteTCPTransport');
const RTStatusCode = require('./RTStatusCode');
const RTValueHelper = require('./RTValueHelper');
const RTValueType = require('./RTValueType');


module.exports = {
  common: {
    helper,
    logger,
  },
  RTConst,
  RTEnvironment,
  RTException,
  RTMessageHelper,
  RTRemoteClientConnection,
  RTRemoteConnectionManager,
  RTRemoteMessageType,
  RTRemoteMulticastResolver,
  RTRemoteObject,
  RTRemoteProtocol,
  RTRemoteSerializer,
  RTRemoteServer,
  RTRemoteTask,
  RTRemoteTCPTransport,
  RTStatusCode,
  RTValueHelper,
  RTValueType,
  ...{ ...RTRemoteProxy }, // make rtRemote proxy in export context root path
};

