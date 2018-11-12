



# pxScene javascript debugging with Visual Studio Code



**Tested on OS X**



1. Download and install Visual Studio Code

  https://code.visualstudio.com/

2.  In Visual Studio Code use file->open menu to open the local directory where your pxscene js code resides and select the file that you want to debug.

> Note: Open the directory rather than just a single file; opening the directory will create a ./vscode subdirectory within your js directory, and this is needed for the debug configuration in step #3.  



3. Click on the debug icon in Visual Studio Code (along the left-hand side) and then click on the gear (top of the left-most panel) this should open a file called launch.json.  Replace the contents of launch.json with the code block below.

NOTE:  From VSCode 1.25 and greater ... it is necessary to include `"protocol": "legacy",` in *launch.json*.

  ~~~~
// Reference
// https://code.visualstudio.com/Docs/editor/debugging
// https://github.com/Microsoft/vscode/issues/102

{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "DBG file in pxscene",
      "type": "node",
      "request": "launch",
      "protocol": "legacy",
      "cwd": "/Applications/Spark.app/Contents/MacOS",
      "runtimeExecutable": "/Applications/Spark.app/Contents/MacOS/Spark",
      "args":["${file}"],
      "env" : {
      "LD_LIBRARY_PATH":"/Applications/Spark.app/Contents/MacOS/lib",
      "DYLD_LIBRARY_PATH":"/Applications/Spark.app/Contents/MacOS/lib"
      //,"BREAK_ON_SCRIPTSTART":1    
      }
    },
    {
      "name": "DBG pxscene",
      "type": "node",
      "request": "launch",
      "protocol": "legacy",
      "cwd": "/Applications/Spark.app/Contents/MacOS",
      "runtimeExecutable": "/Applications/Spark.app/Contents/MacOS/Spark",
      "env" : {
      "LD_LIBRARY_PATH":"/Applications/Spark.app/Contents/MacOS/lib",
      "DYLD_LIBRARY_PATH":"/Applications/Spark.app/Contents/MacOS/lib"
      //,"BREAK_ON_SCRIPTSTART":1      
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

5. Go back to the debugger.  Choose the "DBG pxscene" configuration and then click the green "debug arrow".  You should see the pxscene application launch and in the browser url bar enter the file url that cooresponds to the js file that you want to debug.  Alternatively, choose the "DBG file in pxscene" configuration, make sure the js file you want to launch is the focused file in the IDE, then click the green "debug arrow". This will launch the focused file directly without displaying the pxscene browser url bar.

  ~~~~
  eg. file:///Users/johnrobinson/Sites/pxscene/examples/px-reference/gallery/fancy.js
  ~~~~

6.  You should now be able to set and hit breakpoints in the debugger.

> Note: If you have set environment variable `BREAK_ON_SCRIPTSTART=1`, execution will get stopped as the javascript file loads; otherwise, execution will stop at breakpoints only.


## Attaching to a running instance of spark


> Note: You must have started spark with the --debug=5858 (5858 being the ip port specified in your .vscode/launch.json 
> config file) command line option in order for this to work.  eg. ./spark.sh --debug=5858

1.  Launch pxscene as described above.

2.  Please set up your visual studio code launch.json as described above.  Go to the debugger and choose the "Attach pxscene" config option and click the debugger "green arrow".

3.  Within pxscene enter the file: url of the js file that you want to debug.

4.  Within visual studio code navigate to the source for the js file that you want to debug.  You should now be able to set and hit breakpoints.

## See Also

https://code.visualstudio.com/Docs/editor/debugging
