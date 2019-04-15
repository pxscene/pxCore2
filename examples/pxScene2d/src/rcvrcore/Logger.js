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
 * Created by tcarro004 on 7/18/15, updated by sgladk001c on 3/07/19.
 */

var loggingLevel = 1;

var rtLogLevels = [
  'fatal'   // 0
  , 'error' // 1
  , 'warn'  // 2
  , 'info'  // 3
  , 'debug' // 4
];

//translates RT_LOG_LEVEL string to a numerical logging level
function rtl(rtLogLevel) {

  var result = rtLogLevels.indexOf(rtLogLevel);

  if (result === -1) {
    result = rtLogLevels.indexOf('warn');
  }
  return result;
}

// accepts a numerical value or RT_LOG_LEVEL string
function setLoggingLevel(level) {
  var levelNum;

  if (typeof(level) === 'string') {
    levelNum = rtl(level);
  } else if (typeof (level) === 'number') {
    levelNum = level;
  } else {
    levelNum = rtl('warn');
  }

  loggingLevel = levelNum;
}

function Logger(name) {
  this.name = name;
}

Logger.prototype.getHeading = function(level) {
  return level + ' [' + this.name + "]: ";
};

Logger.prototype.fullMessage = function(level, message) {
  return this.getHeading(level) + message;
};

Logger.prototype.error = function(message) {
  console.error(this.fullMessage('ERROR', message));
};

Logger.prototype.warn = function(message) {
  console.warn(this.fullMessage('WARN', message));
};

Logger.prototype.info = function(message) {
  this.message(1, message);
};

Logger.prototype.message = function(level, message) {
  var levelNum;

  if (typeof(level) === 'string') {
    levelNum = rtl(level);
  } else if (typeof (level) === 'number') {
    levelNum = level;
  } else {
    levelNum = rtl('warn');
  }

  if( levelNum > loggingLevel ) {
   return;
  }
  console.log(this.fullMessage('MESSAGE:'+levelNum, message));
};

module.exports = {Logger:Logger, setLoggingLevel:setLoggingLevel};
