
/*

Small redis wrapper, not really needed anymore but might come in handy if anyone wants to override the default data store (redis) */

module.exports = function(host, port) {
  var redis = require('redis');
  var self = this;
  self.client = redis.createClient(port, host);

  self.client.on('error', function (err) {
    console.log('Error ' + err);
  });

  self.get = function(key, callback) {
    self.client.get(key, function (err, data) {
      callback(null, data);
    });
  };

  self.set = function(key, value, callback) {
    self.client.set(key, value, redis.print);
  };

  self.expire = function(key, seconds) {
    self.client.expire(key, seconds);
  };
  return self;
};


