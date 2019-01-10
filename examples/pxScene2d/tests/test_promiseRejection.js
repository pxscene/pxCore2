/**
 * Test for Promise rejections
 *
 * All sub-tests except 'errorInTestBody' and 'errorInPromise' must succeed
 */

'use strict';

px.import({scene: 'px:scene.1.js'})
.then(function() {

  module.exports.beforeStart = function () {
    return new Promise(function() {
      throw new Error("errorInBeforeStart");
    });
  };

  module.exports.tests = {
    errorInTestBody: function () {
      throw new Error("errorInTestBody");
    },
    errorInPromise: function () {
      return new Promise(function() {
        throw new Error("errorInPromise");
      });
    },
    errorInAsyncFunction: function () {
      return new Promise(function(resolve) {
        setTimeout(function () {
          setTimeout(function () {
            resolve("SUCCESS");
          }, 0);
          throw new Error("errorInAsyncFunction");
        }, 0);
      });
    },
    errorInPromiseToBeCatchedByCallback: function () {
      return new Promise(function(resolve) {
        new Promise(function() {
          throw new Error("errorInPromiseToBeCatchedByCallback");
        }).then(function () {
          resolve("FAILURE");
        }, function () {
          resolve("SUCCESS");
        });
      });
    },
    errorInPromiseToBeCatchedByCatch: function () {
      return new Promise(function(resolve) {
        new Promise(function() {
          throw new Error("errorInPromiseToBeCatchedByCatch");
        }).catch(function () {
          resolve("SUCCESS");
        });
      });
    },
    errorInThenToBeCatchedByCallback: function () {
      return new Promise(function(resolve) {
        Promise.resolve().then(function () {
          throw new Error("errorInThenToBeCatchedByCallback");
        }).then(function () {
          resolve("FAILURE");
        }, function () {
          resolve("SUCCESS");
        });
      });
    },
    errorInThenToBeCatchedByCatch: function () {
      return new Promise(function(resolve) {
        Promise.resolve().then(function () {
          throw new Error("errorInThenToBeCatchedByCatch");
        }).catch(function () {
          resolve("SUCCESS");
        });
      });
    },
    errorInThenUncaught: function () {
      return new Promise(function(resolve) {
        Promise.resolve().then(function () {
          setTimeout(function () {
            resolve("SUCCESS");
          }, 0);
          throw new Error("errorInThenUncaught");
        });
      });
    },
    errorInCatchUncaught: function () {
      return new Promise(function(resolve) {
        new Promise(function() {
          throw new Error("errorInPromise");
        }).catch(function () {
          setTimeout(function () {
            resolve("SUCCESS");
          }, 0);
          throw new Error("errorInCatchUncaught");
        });
      });
    }
  };
});
