/**
 * Verifies env variable
 * is set by LoggingPreferences' setKeystrokeMaskEnabled
 */

px.import({scene:"px:scene.1.js", manual:"../test-run/tools_manualTests.js"})
.then(function (imports) {
  var scene = imports.scene
    , manual = imports.manual
    , manualTest = manual.getManualTestValue()
    ;

  var tests = {
    printInitialValue: function () {
      return new Promise(function (resolve) {
        try {
          var val = process.env.PXSCENE_KEY_LOGGING_DISABLED;
          console.log("initial value is: " + val);
          resolve("SUCCESS");
        } catch (e) {
          resolve("FAILURE: " + e);
        }
      });
    },
    keystrokeMaskEnabledTest: function () {
      return new Promise(function (resolve) {
        try {
          scene.getService("org.rdk.LoggingPreferences")
          .callMethod("setKeystrokeMaskEnabled", JSON.stringify({"params": [true]}));
          var val = process.env.PXSCENE_KEY_LOGGING_DISABLED;
          console.log("actual value is: " + val);
          resolve(val === "1" ? "SUCCESS" : "FAILURE");
        } catch (e) {
          resolve("FAILURE: " + e);
        }
      });
    },
    keystrokeMaskDisabledTest: function () {
      return new Promise(function (resolve) {
        try {
          scene.getService("org.rdk.LoggingPreferences")
          .callMethod("setKeystrokeMaskEnabled", JSON.stringify({"params": [false]}));
          var val = process.env.PXSCENE_KEY_LOGGING_DISABLED;
          console.log("actual value is: " + val);
          resolve(val === "0" ? "SUCCESS" : "FAILURE");
        } catch (e) {
          resolve("FAILURE: " + e);
        }
      });
    }
  };

  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }
})
.catch(function (err) {
  console.error("Import failed: " + err);
});
