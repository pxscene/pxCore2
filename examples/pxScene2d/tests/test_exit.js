"use strict";

/**
 * Spark instance termination test.
 *
 * Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
 * Version: 1.0
 **/
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then(
  function ready(imports) {
    const scene          = imports.scene;
    const root           = imports.scene.root;
    const assert         = imports.assert.assert;
    const manual         = imports.manual;
    const manualTest     = manual.getManualTestValue();
    const basePackageUri = px.getPackageBaseFilePath();

    const tests = {
      exitTest: function() {
        console.log("Running exitTest");

        return new Promise(function(resolve, reject) {
          const timeout = 10000;
          console.log("Finishing exitTest within ~" + timeout + " [ms]");

          setInterval(function() {
            console.log("Finishing exitTest now");
            process.exit(0);
          }, timeout);

          resolve("SUCCESS");

        });
      }
    }

    module.exports.tests = tests;

    if(manualTest === true) {
      manual.runTestsManually(tests);
    }

}).catch( function importFailed(err) {
  console.trace();
  console.error("Import for test_exit.js failed: " + err);
});