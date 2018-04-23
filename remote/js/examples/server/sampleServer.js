/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the sample rt remote server
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteServer = require('../../lib/RTRemoteServer');
const SampleObject = require('./SampleObject');
const logger = require('../../lib/common/logger');

// create server and register 4 objects
RTRemoteServer.create('224.10.10.12', 10004, '127.0.0.1').then((rtRemoteServer) => {
  rtRemoteServer.registerObject('host_object', new SampleObject());
  rtRemoteServer.registerObject('obj2', new SampleObject());
  rtRemoteServer.registerObject('obj3', new SampleObject());
  rtRemoteServer.registerObject('obj4', new SampleObject());
}).catch((err) => {
  logger.error(err);
});
