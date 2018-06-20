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
 * Created by tcarro004 on 7/18/15.
 */

var loggingLevel = 1;

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

Logger.prototype.message = function(levelNum, message) {
  if( levelNum > loggingLevel ) {
   return;
  }
  console.log(this.fullMessage('MESSAGE:'+levelNum, message));
};

function setLoggingLevel(level) {
  loggingLevel = level;
}

module.exports = {Logger:Logger, setLoggingLevel:setLoggingLevel};
