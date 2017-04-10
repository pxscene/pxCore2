# pxscene2d windows build

## env
- win 10
- vs 2015
- python 2.7.x , make sure python can work in cmd.
- git for windows , make sure git can work in cmd.
-  Inno setup Compiler 5.5.9 , http://www.jrsoftware.org/isdl.php

## build
### build pxscene  basic deps
- open`pxCore\examples\pxScene2d\external\vc.build\external.sln`  , if popup some confirm dialog , please click ok.
- don't modify any config(make sure to use Release mode, similar for below projects) , use `ctrl+shift+b` or `build->build solution` to build static libs.
- after build success , you can close vs2015(if missing some header files you have to copy missing headers from exist folder).

### build node 6.9.0
- open cmd , and `cd pxCore\examples\pxScene2d\external\libnode-v6.9.0`
- use `vcbuild.bat x86 nosign` to build nodejs static lib and exe.

### build pxscene
- open `pxCore\pxCore.vsbuild\pxscene.sln` ,  pxcore is vs2013 project, when open it, you need upgrade it to vs2015.
- make sure to use Release mode, use `ctrl+shift+b` or `build->build solution` to build pxscene
- `cd pxCore\pxCore.vsbuild`
- `python pxScene2d/afterBuild.py` , after success run this, you can get a full pxscene2d program at `pxCore\pxCore.vsbuild\pxScene2d\exe`.


## package setup.exe
-  Download 32 bit version of these libs [VSC++2015 SP3](https://www.microsoft.com/en-us/download/details.aspx?id=53840), [VSC++2010Sp1](https://www.microsoft.com/en-us/download/details.aspx?id=26999) and put in folder `pxCore\pxCore.vsbuild\pxScene2d`.
-  use inno setup Compiler open `pxCore\pxCore.vsbuild\pxScene2d\make_package.iss`
-  modify `Files` section,  `Source` to your exe path.
-  use `build->Compile` , after that , you can find `setup.exe` at `pxCore\pxCore.vsbuild\pxScene2d\Output` ,you can double click to install pxscene

## run
### run by desktop
- double click pxScene on desktop enter some http url or local file to test. the local file path must in disk c partition(see issues).

### run by cmd
  - open `cmd` ,  run `cd pxCore\pxCore.vsbuild\pxScene2d\exe` or `C:\Program Files (x86)\pxScene`.
  -  run pxorg online exampes `pxScene.exe http://www.pxscene.org/examples/px-reference/gallery/gallery.js` or `pxScene.exe http://www.pxscene.org/examples/px-reference/gallery/fonts.js`
  -  you also can run pixi online example(from forum), but is need a good network  because of the pixi.js is so big , my network often return download error. so i download it ,and start http-server local.
    - goto submission folder, and  `cd examples`
    - `python -m SimpleHTTPServer`
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
	
	if your network is well you could also use online samples 
	- Animated Sprite: http://pxscene-pixi-examples.herokuapp.com/demos/animatedsprite-demo.js
	- Alpha Mask: http://pxscene-pixi-examples.herokuapp.com/demos/alpha-mask.js
	- Particle Container: http://pxscene-pixi-examples.herokuapp.com/demos/batch.js
	- Blend Modes: http://pxscene-pixi-examples.herokuapp.com/demos/blendmodes.js
	- Cache as Bitmap: http://pxscene-pixi-examples.herokuapp.com/demos/cacheAsBitmap.js
	- Dragging: http://pxscene-pixi-examples.herokuapp.com/demos/dragging.js
	- Graphics: http://pxscene-pixi-examples.herokuapp.com/demos/graphics-demo.js
	- Interactivity: http://pxscene-pixi-examples.herokuapp.com/demos/interactivity.js
	- Render Texture: http://pxscene-pixi-examples.herokuapp.com/demos/render-texture-demo.js
	- Text: http://pxscene-pixi-examples.herokuapp.com/demos/text-demo.js
	- Texture Rotate: http://pxscene-pixi-examples.herokuapp.com/demos/texture-rotate.js
	- Texture Swap: http://pxscene-pixi-examples.herokuapp.com/demos/texture-swap.js
	- Tinting: http://pxscene-pixi-examples.herokuapp.com/demos/tinting.js
	- Transparent Background: http://pxscene-pixi-examples.herokuapp.com/demos/transparent-background.js


# vscode debug
- copy `examples` to C partition or any driver, use vscode open it.
- follow https://github.com/johnrobinsn/pxCore/blob/master/examples/pxScene2d/VSCODE_DEBUG.md config vscode , use those debug config
``` javascript
{
"version": "0.2.0",
"configurations": [
  {
    "name": "DBG file in pxscene",
    "type": "node",
    "request": "launch",
    "cwd": "C://Program Files (x86)//pxScene",
    "runtimeExecutable": "C://Program Files (x86)//pxScene//pxScene.exe",
    "args":["${file}"]
  },
  {
    "name": "DBG pxscene",
    "type": "node",
    "request": "launch",
     "cwd": "C://Program Files (x86)//pxScene",
    "runtimeExecutable": "C://Program Files (x86)//pxScene//pxScene.exe"
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
- open any files in `examples\demos` folder and start `DBG file in pxscene`, you will be break on any breakpoint to test.
- start `DBG pxscene` ,input `files://C:\examples\fancy.js` in pxscene, you will be break on any previous breakpoint to test.
- `cd C:\Program Files (x86)\pxScene`, `pxScene.exe --debug=5858`. and use `Attach pxscene` can attach pxscene.

# additional
1. Clipboard in browser.js does not work , because the base code cpp code didn't implement , I implement it .