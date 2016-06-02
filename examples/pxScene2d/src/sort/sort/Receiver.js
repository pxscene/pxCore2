global.clog = function() {};

var xreApp = require('./src/protocol/xreapplication.js');
var fs = require('fs');
//TODO set settings for the Receiver
//TODO get arg's for the XREapp from a json file

var args = JSON.parse(fs.readFileSync('receiverConfig.json', 'utf8'));


var token = "";
var launchParams = {
    //"authVersion": "12",
    "mac": "14:D4:FE:A4:03:92",
    "heartbeatRequest": 1,
    "networkBuffer": {
        "min": 150,
        "max": 150,
        "step": 0
    },
    "PHPSESSID": "undefined"
};

var rootApp = new xreApp(scene);
xreApp.rootApp = rootApp;

if (true) {
    xreApp.enableWhiteboxAuthentication("http://localhost:50050/device/generateAuthToken");
}
rootApp.start(args.serverUrl, token, launchParams, 0);

scene.root.on("onKeyDown", function(e) {
    console.log("on key down");
    var modifiers = {
        "ctrl": false,
        "alt": false,
        "shift": false,
        "meta": false
    };
    if(e.flags === 16){
        modifiers.ctrl = true;
    }
    if(e.flags === 8){
        modifiers.alt = true;
    }
    if(e.flags === 8){
        modifiers.shift = true;
    }
    if(e.flags === 48){
        modifiers.alt = true;
        modifiers.ctrl = true;
    }
    console.log("going to raise event....");
    var activeView = rootApp.getActiveView();
    console.log("Current view Id------------------ " + rootApp.Aname);
    console.log(activeView.prototype.GetId());
    console.log("going to raise event....");
    activeView.raiseEvent(e, modifiers);
    console.log("Event raised....");
});



scene.root.on("onKeyUp", function(e) {
    console.log("on key UP");

    var modifiers = {
        "ctrl": false,
        "alt": false,
        "shift": false,
        "meta": false
    };
    if(e.flags === 16){
        modifiers.ctrl = true;
    }
    if(e.flags === 8){
        modifiers.alt = true;
    }
    if(e.flags === 8){
        modifiers.shift = true;
    }
    if(e.flags === 48){
        modifiers.alt = true;
        modifiers.ctrl = true;
    }
    console.log("going to raise event....");
    var activeView = rootApp.getActiveView();
    console.log("Current view Id------------------ " + rootApp.Aname);
    console.log(activeView.prototype.GetId());
    console.log("going to raise event....");
    activeView.raiseEvent(e, modifiers);
    console.log("Event raised....");
});
