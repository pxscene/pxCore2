



# pxScene debugging with Visual Studio Code



**Tested on OS X**



1. Download and install Visual Studio Code

https://code.visualstudio.com/



2.  In Visual Studio Code use file->open menu to open the local directory where your pxscene js code resides and select the file that you want to debug.



3. Click on the debug icon in Visual Studio Code (along the left-hand side) and then click on the gear (top of the left-most panel) this should open a file called launch.json.  Replace the contents of launch.json with the code block below.  **Note: "Attach pxscene" no worky yet**



~~~~
// Reference
// https://code.visualstudio.com/Docs/editor/debugging
// https://github.com/Microsoft/vscode/issues/102

{
  "version": "0.2.0",
  "configurations": [

    {
      "name": "Debug pxscene",
      "type": "node",
      "request": "launch",
      "cwd": "/Applications/pxscene.app/Contents/MacOS",
      "runtimeExecutable": "/Applications/pxscene.app/Contents/MacOS/pxscene",
      "program": "/Applications/pxscene.app/Contents/MacOS/browser.js",
      "env" : {
      "LD_LIBRARY_PATH":"/Applications/pxscene.app/Contents/MacOS/lib",
      "DYLD_LIBRARY_PATH":"/Applications/pxscene.app/Contents/MacOS/lib"
      }
    },
    {
    "name": "Attach pxscene",
    "type": "node",
    "request": "attach",
    // TCP/IP address. Default is "localhost".
    "address": "localhost",
    // Port to attach to.
    "port": 5858,
    "sourceMaps": false
    }
  ]
}
~~~~



4.  Go back to the file that you'd like to debug (click on the explorer icon top left-hand side).  Set a breakpoint by clicking to the left of the appropriate source line.



5. Go back to the debugger.  Choose the "Debug pxscene" configuration and then click the green "debug arrow".  You should see the pxscene application launch and in the browser url bar enter the file url that cooresponds to the js file that you want to debug.  

eg.

~~~~
file:///Users/johnrobinson/Sites/pxscene/examples/px-reference/gallery/fancy.js
~~~~



6.  You should now be able to set and hit breakpoints in the debugger.





## See Also

https://code.visualstudio.com/Docs/editor/debugging
