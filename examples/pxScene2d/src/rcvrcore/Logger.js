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
  //if( levelNum > loggingLevel ) {
  // return;
  //}
  console.log(this.fullMessage('MESSAGE:'+levelNum, message));
};

function setLoggingLevel(level) {
  loggingLevel = level;
}

module.exports = {Logger:Logger, setLoggingLevel:setLoggingLevel};
