
### For STB server
* `cd remote` 
 * `cd build && cmake .. -DBUILD_STB_SERVER=ON ` 
 * `make`
 * `cd ..`
* `./stbRemoteServer` 

For Node client

* `cd remote/js`
 * `npm i`
 * `node repl`
 * `rtRemote.connect('224.10.10.12', 10004, 'DisplayManager').then(rtObj => stb = rtObj)` (replace 224.10.10.12 with the STB server IP address)
4. `var r = stb.getResolution();` and then enter `r` to see result.
5. `stb.setResolution(1920,1080)` to update resolution
