/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
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

