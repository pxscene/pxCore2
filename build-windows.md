#pxscene2d windows build

## env
- win 10
- vs 2015
- python 2.7.x , make sure python can work in cmd.
- git for windows , make sure git can work in cmd.
-  Inno setup Compiler 5.5.9 , http://www.jrsoftware.org/isdl.php

## get code and apply patch
- `git clone https://github.com/johnrobinsn/pxCore.git`
- `cd pxCore`
- `git checkout 0b2d52b48f668c79726d11b5d91c905ce484c2fc`
- `git apply --ignore-space-change --ignore-whitespace pxcore-windows.patch` (windows git some times have some problems, if you cannot apply patch , you can use `pxCore` folder replace the same name files)

## build
### build pxscene  basic deps
- open`pxCore\examples\pxScene2d\external\vc.build\external.sln`  , if popup some confirm dialog , please click ok.
- don't modify any config , use `ctrl+shift+b` or `build->build solution` to build static libs.
- after build success , you can close vs2015.
### build node 6.9.0
- open cmd , and `cd pxCore\examples\pxScene2d\external\libnode-v6.9.0`
- use `vcbuild.bat x86 nosign` to build nodejs static lib and exe.

### build pxscene 2d


- open `pxCore\pxCore.vsbuild\pxscene2d.sln` ,  pxcore is vs2013 project, when open it, you need upgrade it to vs2015.
- use `ctrl+shift+b` or `build->build solution` to build pxscene
- `cd pxCore\pxCore.vsbuild`
- `python pxScene2d/afterBuild.py` , after success run this, you can get a full pxscene2d program at `pxCore\pxCore.vsbuild\pxScene2d\exe`.


##package setup.exe
-  use inno setup Compiler open `pxCore\pxCore.vsbuild\pxScene2d\make_package.iss`
-  modify `Files` section,  `Source` to your exe path.
-  use `build->Compile` , after that , you can find `setup.exe` at `pxCore\pxCore.vsbuild\pxScene2d\Output` ,you can double click to install pxscene2d

##run
###run by desktop
- double click pxScene2d on desktop enter some http url or local file to test. the local file path must in disk c partition(see issues).
### run by cmd
- open `cmd` ,  run `cd pxCore\pxCore.vsbuild\pxScene2d\exe` or `C:\Program Files (x86)\pxCore`.
  -  run pxorg online exampes `pxScene2d.exe http://www.pxscene.org/examples/px-reference/gallery/gallery.js` or `pxScene2d.exe http://www.pxscene.org/examples/px-reference/gallery/fonts.js`
  -  you also can run pixi online example(from forum), but is need a good network  because of the pixi.js is so big , my network often return download error. so i download it ,and start http-server local.
    - goto submission folder, and  `cd examples`
    -  `python MultithreadedSimpleHTTPServer.py`
    -  `pxScene2d.exe http://127.0.0.1:8000/fancy.js`
    - Animated Sprite: http://127.0.0.1:8000/demos/animatedsprite-demo.js
    - Alpha Mask: http://127.0.0.1:8000/demos/alpha-mask.js
    - Particle Container: http://127.0.0.1:8000/demos/batch.js
    - Blend Modes: http://127.0.0.1:8000/demos/blendmodes.js
    - Cache as Bitmap: http://127.0.0.1:8000/demos/cacheAsBitmap.js
    - Dragging: http://127.0.0.1:8000/demos/dragging.js
    - Graphics: http://127.0.0.1:8000/demos/graphics-demo.js
    - Interactivity: http://127.0.0.1:8000/demos/interactivity.js
    - Render Texture: http://127.0.0.1:8000/demos/render-texture-demo.js
    - Text: http://127.0.0.1:8000/demos/text-demo.js
    - Texture Rotate: http://127.0.0.1:8000/demos/texture-rotate.js
    - Texture Swap: http://127.0.0.1:8000/demos/texture-swap.js
    - Tinting: http://127.0.0.1:8000/demos/tinting.js
    - Transparent Background: http://127.0.0.1:8000/demos/transparent-background.js


#vscode debug
- copy `examples` to C partition, use vscode open it.
- follow https://github.com/johnrobinsn/pxCore/blob/master/examples/pxScene2d/VSCODE_DEBUG.md config vscode , use those debug config
``` javascript
{
"version": "0.2.0",
"configurations": [
  {
    "name": "DBG file in pxscene",
    "type": "node",
    "request": "launch",
    "cwd": "C://Program Files (x86)//pxCore",
    "runtimeExecutable": "C://Program Files (x86)//pxCore//pxScene2d.exe",
    "args":["${file}"],
    "env" : {
      "BREAK_ON_SCRIPTSTART":"1"
    }
    
  },
  {
    "name": "DBG pxscene",
    "type": "node",
    "request": "launch",
     "cwd": "C://Program Files (x86)//pxCore",
    "runtimeExecutable": "C://Program Files (x86)//pxCore//pxScene2d.exe",
    "env" : {
    "BREAK_ON_SCRIPTSTART":"1"      
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
```
- open `fancy.js` and start `DBG file in pxscene`, you will be break on first line , and then you can set other breakpoint to test.
- start `DBG pxscene` , you will break on `brower.js` first line ,  press F5 continue , input `files://C:\examples\fancy.js` in pxscene, you will be break on first line , and then you can set other breakpoint to test.
- `cd C:\Program Files (x86)\pxCore`, `pxScene2d.exe --debug-brk=5858`. and use `Attach pxscene` can attach pxscene.
#issues
1. the pxscene windows support local path , but it's need the local file same disk partition  as the pxscene. for example, if your pxscene at `C:\Program Files (x86)\pxCore`, your local test files must in `c` partition.
2. because of windows pxscene2d  local file path got problem, pxscene prase  `C:\examples\fancy.js` as `C:/examples/fancy.js`, so the vscode debugger cannot connect with pxscene. for now, please make sure the `DBG file in pxscene` and `DBG pxscene` must be set`"BREAK_ON_SCRIPTSTART":"1"` , then vs code will auto break on first line.

#additional
1. Clipboard in browser.js does not work , because the base code cpp code didn't implement , i implement it .
#video
https://youtu.be/ofrM7-rAzGY