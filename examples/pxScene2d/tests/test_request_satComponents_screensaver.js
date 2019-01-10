/** This version of screensaver uses external json files for images and text */
"use strict";
px.import({scene:"px:scene.1.js",ws:'ws'}).then( function ready(imports) {


  var scene = imports.scene;
  var root = scene.root;
  var ws = imports.ws;

process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0";

  var dataService = "wss://px-wss.sys.comcast.net:8443";  // default 
  if( px.appQueryParams.dataServ !== undefined) {
    dataService = px.appQueryParams.dataServ;
  }

  var basePackageUri = px.getPackageBaseFilePath();

  var sat = px.appQueryParams.sat;
  
  
  // Vars for socket paths and requests
  var SOCKET_PATH_SCREENSAVER = "/screensaver";
  var TYPE_NAME_URL = "url";
  
  // DataService
  var DataService = function(socketPath,typeName, value) {
      var promise = new Promise(function(resolve,reject) {
        
        //console.log("about to try opening ws for url "+url);
        var mySocket = new ws(dataService+socketPath); 
        console.log("done opening ws");
        mySocket.on('open', function() {
            console.log("Received open");
            var sat = px.appQueryParams.sat;
            var newJson =  {};
            newJson["token"] = sat;
            newJson[typeName] = value;
            
            mySocket.send(JSON.stringify(newJson));
        });
        mySocket.on('message', function(message) {
            console.log('received: %s', message);
            //mySocket.close();
            resolve( message);
            
        });
        mySocket.on('close', function() {
          console.log('closing socket');
        });
        mySocket.on('error', function(msg) {
          console.log('error on socket: '+msg);
        });
    });

    return promise;
  }


  
  var startClock = scene.clock();

  var requestPromise = DataService(SOCKET_PATH_SCREENSAVER, TYPE_NAME_URL, "http://ips-qa.apps.xcal.tv/image-service/v2/screensaver/images/");

  /** Wait for promises to get user, account and device info. Then use that
   *  to get the facebook photos 
   * */
    requestPromise.then(function(data) {
      console.log("got promises for requestPromise!");
      var reqResponse = JSON.parse(data);
      //accountId = reqResponse.acct;
      //deviceId = reqResponse.device;
      //userId = reqResponse.user;
      var message = scene.create({t:"text", parent:root, x:250, y:250, text:"DONE: \n"+startClock+" START TIME  \n"+scene.clock()+" FINISH TIME"});
      console.log("data received: "+data);

    });


  
}).catch( function importFailed(err){
  console.error("Imports failed for screensaver_sliding_wss_launch.js: " + err)
});
