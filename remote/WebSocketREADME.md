
### For STB server
* `cd remote` 
 * `cd build && cmake .. -DBUILD_STB_SERVER=ON ` 
 * `make`
 * `cd ..`
* `./stbRemoteServer` 

*NOTE* - On Ubuntu you'll want to open up port 10004:  `sudo ufw allow 10004`
For Node client

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
