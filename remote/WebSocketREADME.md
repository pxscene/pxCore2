# Overview

For websocket support in pxCore / rtRemote, we were asked to do a couple different things:

1.  The client asked us to support standard websockets as a transport option in rtRemote
2.  The client asked us to test ECMA / browser support for the new websockets, using the browser as an rtRemote client to an rtRemote server that has published objects via websocket
3.  The client asked us to build a simple REPL demo that could be used to show off how this might work in production, with the ability to connect to and manage a sample set top box right from a Node console.  

This app has a few pieces to it:

1.  The websocket integration and changes
2.  A sample website that runs through a variety of tests after being connected to a remote rtRemote server through websockets
3.  A demo application and client, described below

The initial websocket integration was done through a challenge:

https://github.com/topcoderinc/pxCore/issues/171 

The REPL demo was built via direct tickets with jiangliwu based on calls we had with the client.

After the initial integration, we discussed further with the client and the client asked us to do some changes to the implementation, including integrate in a specific C library for websocket support.  These changes were implemented through these tickets:

#### Bugs found during review

* https://github.com/topcoderinc/pxCore/issues/174
* https://github.com/topcoderinc/pxCore/issues/175
* https://github.com/topcoderinc/pxCore/issues/176
* https://github.com/topcoderinc/pxCore/issues/177
* https://github.com/topcoderinc/pxCore/issues/178
* https://github.com/topcoderinc/pxCore/issues/182
* https://github.com/topcoderinc/pxCore/issues/197
* https://github.com/topcoderinc/pxCore/issues/198

#### Ability to host the test website remotely

* https://github.com/topcoderinc/pxCore/issues/179
* It is now published on Heroku:  http://rtremote-websocket.herokuapp.com/

#### Update to use uWebSockets lib, as requested

* https://github.com/topcoderinc/pxCore/issues/192

#### Build out the demo app, described below

* https://github.com/topcoderinc/pxCore/issues/194
* https://github.com/topcoderinc/pxCore/issues/207

# Build rtRemote with websocket support

This describes how to build rtRemote with general websocket support, assuming that pxScene and all external dependencies have already been built.  If you get errors about not being able to link in rtCore (-lrtCore not found), you need to make sure that pxScene has been built properly first.  The build steps for pxScene will built the rtCore library.

### Build the sample app
* `cd remote` 
* `mkdir build`
 * `cd build && cmake .. -DSUPPORT_WEBSOCKET_TRANSPORT=YES -DBUILD_RTREMOTE_SAMPLE_APP_SIMPLE=YES` 
 * `make`
 * `cd ..`
* `./rtSampleServer` 

*NOTE* - On Ubuntu you'll want to open up port 10005:  `sudo ufw allow 10005`.  Port 10005 is where the websocket connection is served from the rtRemote server.

### Test the sample app in a browser

* In a browser, either on the same computer as the server or a remote computer, open up the sample test app:  https://rtremote-websocket.herokuapp.com
* In the `rtRemote server address` field, enter the IP address and port of the rtRemoteServer, like `192.168.86.20:10005`.
* Click `Do initialize`
 * You will see it connect, the examples will all go to `PENDING`, and it will say `Type Examples - Connected`
 * You will also see a message in the console of the rtRemote server saying `new websocket connection from <client IP address>`
* You can now click `Run All Examples` to test all the various rtRemote type examples.  It will run through them serially, and `PENDING` will change to `PASSED`
* You can also clcik `Method Examples` and `MultiObject Examples` to get different lists of tests to run

This shows:

1.  rtRemote objects served via a C++ app over websocket transport
2.  Javascript in a browser using the rtRemote objects and manipulating them

# REPL Demo Setup / build

The steps below describe how to build a sample mock STB server that publishes a display manager service that we can connect to on the node REPL console and manage the display resolution.  Note that this is only a mock, but it matches the eventual expectation of what the service manager rtRemote service on the set top box will look like, once that's completed and available.

The goal is to eventually have a set top box running rtRemote with the service manager objects exposed on it that we can connect to via the Node console on a laptop and manage the set top box directly from the laptop, changing settings and viewing current account and setup information on the STB.

### For STB server
* `cd remote` 
 * `cd build && cmake .. -DBUILD_STB_SERVER=ON ` 
 * `make`
 * `cd ..`
* `./stbRemoteServer` 

*NOTE* - On Ubuntu you'll want to open up port 10004:  `sudo ufw allow 10004`

### For Node client

Note that you can run the Node client on a separate computer from the server, but the client will need to be on the same subnet due to the multi-cast resolution.  Multi-cast packets won't route outside the immediate subnet.

* `cd remote/js`
 * `npm i`
 * `node repl`

## Example:

```
var serviceManager = yield rtRemote.locateObject('rtServiceManager');
var displayManager = yield serviceManager.createService("org.sa")
var res = yield displayManager.getResolution()
res = yield displayManager.setResolution(1920,1080)
yield displayManager.set("resolutionW",2000)
res = yield displayManager.get("resolutionW")

ret = displayManager.getProperty("resolutionW");
ret.then((returnValue) => {
   // setProperty likely returns NULL
  console.log("Return value:" + returnValue); 
});
```
