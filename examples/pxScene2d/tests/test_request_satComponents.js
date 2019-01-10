/** This version of screensaver uses external json files for images and text */
"use strict";
px.import({scene:"px:scene.1.js",ws:'ws'}).then( function ready(imports) {


  var scene = imports.scene;
  var root = scene.root;
  var ws = imports.ws;

  var dataService = "wss://px-wss.sys.comcast.net:8443";  // default 
  if( px.appQueryParams.dataServ !== undefined) {
    dataService = px.appQueryParams.dataServ;
  }

  var basePackageUri = px.getPackageBaseFilePath();

  var sat = px.appQueryParams.sat;
  
  
  // Vars for socket paths and requests
  var SOCKET_PATH_URL = "/websocket";
  var SOCKET_PATH_REQUEST = "/request";
  var TYPE_NAME_URL = "url";
  var TYPE_NAME_REQUEST = "request";
  
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
        mySocket.on('error', function() {
          console.log('error on socket');
        });
    });

    return promise;
  }

  
  var userId;
  var accountId;
  var deviceId;
  
  var startClock = scene.clock();
  var userRequestPromise = DataService(SOCKET_PATH_REQUEST, TYPE_NAME_REQUEST, "user");
  var acctRequestPromise = DataService(SOCKET_PATH_REQUEST, TYPE_NAME_REQUEST, "acct");
  var deviceRequestPromise = DataService(SOCKET_PATH_REQUEST, TYPE_NAME_REQUEST, "device");
  

  /** Wait for promises to get user, account and device info. Then use that
   *  to get the facebook photos 
   * */
    userRequestPromise.then(function(data) {
      console.log("userId promise received: "+data);
      userId = data;
    });
    acctRequestPromise.then(function(data) {
      console.log("accountId promise received: "+data);
      accountId = data;
    });
    deviceRequestPromise.then(function(data) {
      console.log("deviceId promise received: "+data);
      deviceId = data;
    });

    Promise.all([userRequestPromise,acctRequestPromise,deviceRequestPromise]).then(function() {
      console.log("got promises for user, device and account!");
      var message = scene.create({t:"text", parent:root, x:250, y:250, text:"DONE: \n"+startClock+" START TIME  \n"+scene.clock()+" FINISH TIME"});
    });

  
}).catch( function importFailed(err){
  console.error("Imports failed for screensaver_sliding_wss_launch.js: " + err)
});
