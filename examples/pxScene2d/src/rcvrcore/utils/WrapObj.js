module.exports = function (orig, override) {
  var wrapper = {};
  for (var prop in orig) {
    (function(prop) {
      if (override && override.hasOwnProperty(prop)) {
        wrapper[prop] = override[prop];
      } else if (typeof(orig[prop]) === 'function') {
        wrapper[prop] = function () {
          return orig[prop].apply(this, arguments);
        }
      } else {
        Object.defineProperty(wrapper, prop, {
          'get': function () {
            if (orig[prop] === orig) {
              return wrapper;
            } else {
              return orig[prop];
            }
          },
          'set': function (value) {
            orig[prop] = value;
          }
        });
      }
    })(prop);
  }
  return wrapper;
};
