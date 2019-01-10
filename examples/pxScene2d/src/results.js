"use strict";

/** Page that displays test-run results */
px.import("px:scene.1.js").then( function ready(scene) {

var root = scene.root;

var basePackageUri = px.getPackageBaseFilePath();

var title = scene.create({t:"text", parent:root, text:"Test Results:",pixelSize:30, textColor:0x3090ccff,x: 25, y: 25});

var totalFailures = 0;
var totalSuccesses = 0;
var textSnippets = "";
var success = scene.create({t:"text", parent:root, x:50, y:100, text:"Number of Successes: ", textColor:0x009900FF});
var successNum = scene.create({t:"text", parent:root, x:250, y:100,text:totalSuccesses, textColor:0x009900FF});
var failures = scene.create({t:"text", parent:root, x:50, y:150, text:"Number of Failures: ",textColor:0x990000FF});
var failuresNum = scene.create({t:"text", parent:root, x:250, y:150,text:totalFailures,textColor:0x990000FF});

var failuresTitle = scene.create({t:"text", parent:root, x:50, y:200,text:"FAILURE DETAIL:",textColor:0x990000FF, pixelSize:25});
var failuresDesc = scene.create({t:"textBox",parent:root,x: 150, y:250, 
                                 w: scene.w, h: scene.h, textColor:0x990000FF, clip:false});


module.exports.setResults = function(results) {
  
  
      for(var propertyName in results) {
        console.log("calculating..... for prop name "+propertyName);
        var result = (results[propertyName] != undefined)?results[propertyName].toString():'FAILURE: Test result is empty';
        var pos;
        // Look for FAILURE
        pos = result.indexOf("FAILURE");
        while(pos != -1) {
          if(result.indexOf(",",pos) === -1){
            textSnippets += propertyName+":  "+result.slice(pos)+"\n";
          } else {
            textSnippets += propertyName+":  "+result.slice(pos,result.indexOf(",",pos))+"\n";
          }
          //console.log(propertyName+": "+results[propertyName]+"\n");
          totalFailures++;
          result = result.slice((pos+"FAILURE".length));
          if( result.length <= 0) {
            pos = -1; 
          }
          else {
            pos = result.indexOf("FAILURE");
          }
        }
        // Look for SUCCESS now
        result = (results[propertyName] != undefined)?results[propertyName].toString():'FAILURE: Test result is empty';
        pos = result.indexOf("SUCCESS");
        while(pos != -1) {
          //console.log(propertyName+": "+results[propertyName]+"\n");
          totalSuccesses++;
          result = result.slice((pos+"SUCCESS".length-1));
          if( result.length <= 0) {
            pos = -1; 
          }
          else {
            pos = result.indexOf("SUCCESS");
          }

        }
    }
    console.log("FINISHED LOOPING and should set results "+totalSuccesses+" and "+totalFailures);
    successNum.text = totalSuccesses;
    failuresNum.text = totalFailures;
    if( totalFailures === 0) 
      failuresTitle.a = 0;
    else 
      failuresDesc.text = textSnippets;
      
    console.log("TEST RESULTS: ");
    console.log("Successes: "+totalSuccesses);
    console.log("Failures: "+totalFailures);
    console.log("Failure Details: \n"+textSnippets.toString());
    
    
}

}).catch( function importFailed(err){
  console.error("Import for tests.js failed: " + err)
});
