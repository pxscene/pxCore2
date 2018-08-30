
# Spark debugging with IntelliJ


**NOTE: The preferred environment for debugging spark applications is VSCode.  Instructions are here: https://github.com/johnrobinsn/pxCore/blob/master/examples/pxScene2d/VSCODE_DEBUG.md**


**Tested on OS X**


IntelliJ is an IDE to write and execute applications written in Java, Javascript etc. It has NodeJS plugin support, through which any nodejs applications can be launched through IntelliJ.

*NOTE: Support for debugging with IntelliJ is in Spark as of release OSX 0.23.*

> Prerequisites

> 1. **Install IntelliJ**
>   1. Download the latest ultimate version of IntelliJ from https://www.jetbrains.com/idea/download/#section=linux
>   1. Install the package as application in Applications folder

> 1. **Install Spark for OSX**
>   1. Install release 0.23 or later by following the directions here:  Installing Spark on a Mac (OSX)  


## Debugging by running Spark within IntelliJ (preferred method)

1. Start IntelliJ by double clicking the icon in Applications folder

2. In the IntelliJ dialog that opens, click Create New Project and follow the prompts to create new Empty Project 
   1. Press Finish to create the new project.
   1. IntelliJ will open a Project Structure dialog.  It is safe just to close it. 

3. Install nodejs plugin as per the instructions in https://www.jetbrains.com/help/idea/2016.3/installing-updating-and-uninstalling-repository-plugins.html

4. Create a Run Configuration for Spark.  There are two ways to do this: 
   1. Option 1: Copy the xml snippet below and save as pxscene.xml file into ~/IdeaProjects/<project name>/.idea/runConfigurations directory (or wherever you created your project in step 2). You will need to create the "runConfigurations" subdirectory for new, empty projects.

   1. OR Option 2: Select Run->Edit Configurations (as per link https://www.jetbrains.com/help/idea/2016.3/running-and-debugging-node-js.html#Node.js_run) and set the following fields:
     * **Node Interpreter** = /Applications/spark.app/Contents/MacOS/pxscene
     * **Working directory** = /Applications/spark.app/Contents/MacOS
     * **Environment variables:**
       * LD_LIBRARY_PATH = /Applications/spark.app/Contents/MacOS/lib
       * DYLD_LIBRARY_PATH = /Applications/spark.app/Contents/MacOS/lib
     * **Javascript file** = Leave this blank to launch browser that allows typing/pasting urls, OR <path of javascript file>

5. Run the pxscene configuration:
  1. Run the project in run mode as per https://www.jetbrains.com/help/idea/2016.3/running-and-debugging-node-js.html#running to run pxscene without any breakpoints or debugging
  1. Run the project in debug mode as per https://www.jetbrains.com/help/idea/2016.3/running-and-debugging-node-js.html#running  to run pxscene and be able to set breakpoints and debug javascript.  
  **Note that if you wish to break at the start of each JavaScript page when running in debug mode, set environment variable `BREAK_ON_SCRIPTSTART=1`**
  
6. See the "Setting Breakpoints and Viewing Variables" section below for more debugging details.
 
## Debugging with IntelliJ by attaching to an already running instance of spark 

1. Follow the steps in Creating a Node.js remote debug configuration section from link https://www.jetbrains.com/help/idea/2016.3/running-and-debugging-node-js.html#Node.js_remote_debug. Use the default port = 9998

2. Run spark separately on command line with either of below two command sets from /Applications/spark.app/Contents/MacOS/:
    ~~~~
./spark.sh --debug=9998 <javascript file name>
    ~~~~
 (or)
     ~~~~
./spark.sh --debug-brk=9998 <javascript file name> (This option will break the javascript execution on start/first line)
    ~~~~
    
3. Run the remote debug configuration created in step 1, and it will list the script files in lower windows scripts tab. If running with debug-brk mode, shell.js (which is part of spark) will open in the editor showing the breakpoint in first line.  Click the Resume button to continue to your javascript breakpoints.

## Setting Breakpoints and Viewing Variables

**NOTE:  Setting breakpoints via mouse clicks is only available when running local javascript files.  If running remote javascript files (e.g., from https://px-apps.sys.comcast.net, for instance), you must have inserted the keyword "debugger" within the javacript file for breakpoints to be active.**

**For running spark within IntelliJ and debugging local javascript files:**

1. If running local javascript (e.g., file://), you can open that file(s) and set breakpoints by clicking in front of the line number(s) in the local javascript file (https://www.jetbrains.com/help/idea/2016.3/breakpoints-2.html).
1. Run the Debug configuration by selecting Run->Debug <your spark Run Configuration name>
1. If you have set environment variable `BREAK_ON_SCRIPTSTART=1`, execution will get stopped as the javascript file loads; otherwise, execution will stop at breakpoints only.  Click the Resume button to continue.  The stack frames, local and global variables are displayed in the bottom Variables window.
1. Apart from setting breakpoints, other options are available for line by line execution, such as step in, step over, etc.  See the IntelliJ Run menu for stepping options while debugging. 

**For running IntelliJ with remote spark:**

1. If running local javascript (file://):
  1. you can open that file(s) and set breakpoints by clicking in front of the line number(s) in the local javascript file (https://www.jetbrains.com/help/idea/2016.3/breakpoints-2.html).
  1. Run as per the instructions in "Debugging with IntelliJ by attaching to an already running instance of spark".
1. If running remotely hosted javascript file (e.g., http://): 
  1. Edit the javascript file by inserting a line "debugger;" before the line on which we need breakpoint.
  1. Run as per the instructions in "Debugging with IntelliJ by attaching to an already running instance of spark".
  1. Execution will stop at the lines on which we have lines with "debugger;" statement and IntelliJ will display the stack frames, local and global variables on the bottom window.
1. Apart from setting breakpoints, we can also step line by line execution.

##Current issues
Unable to set breakpoint with mouse when running hosted javascript files.  Breakpoints are only supported where "debugger" statement has been added in the javascript file.  Also, we are unable to edit file in remote debug mode and it is showing as read-only mode.


##pxscene.xml: Run configuration to use with IntelliJ:

```
<component name="ProjectRunConfigurationManager">
  <configuration default="false" name="pxscene" type="NodeJSConfigurationType" factoryName="Node.js" path-to-node="/Applications/spark.app/Contents/MacOS/pxscene" working-dir="/Applications/spark.app/Contents/MacOS">
    <envs>
      <env name="LD_LIBRARY_PATH" value="/Applications/spark.app/Contents/MacOS/lib" />
      <env name="DYLD_LIBRARY_PATH" value="/Applications/spark.app/Contents/MacOS/lib" />
    </envs>
    <profiling v8-profiler-path="" />
    <EXTENSION ID="com.jetbrains.nodejs.run.NodeJSProfilingRunConfigurationExtension">
      <profiling v8-profiler-path="" />
    </EXTENSION>
    <method />
  </configuration>
</component>
```
