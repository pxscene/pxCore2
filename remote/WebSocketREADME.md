
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
* `stb = yield rtRemote.connect('224.10.0.12', 10004, 'DisplayManager')`
* `res = yield stb.getResolution()`
* `res.w`
* `res.h`
5. `stb.setResolution(1920,1080)` to update resolution
