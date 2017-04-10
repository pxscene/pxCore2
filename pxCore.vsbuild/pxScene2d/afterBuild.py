import shutil
import os
build_dir=os.path.join(".","pxScene2d")
external_dir="../examples/pxScene2d/external"
pxscene2d_srd_dir="../examples/pxScene2d/src"

def remove(dirStr):
    try:
        shutil.rmtree(dirStr)
    except:
        pass

def copyRes(baseDir):
    for f in ["browser.js","init.js","shell.js","FreeSans.ttf", "package.json"]:
        shutil.copy(os.path.join(pxscene2d_srd_dir,f),os.path.join(baseDir,f))
    for d in ["rcvrcore","images","node_modules"]:
        shutil.copytree(os.path.join(pxscene2d_srd_dir,d),os.path.join(baseDir,d))
    
    shutil.copy(os.path.join(external_dir,"libnode-v6.9.0","Release","node.exe"),os.path.join(baseDir,"node.exe"))
    shutil.copy(os.path.join(external_dir,"gles\\prebuilt\\glew32.dll"),baseDir)
    shutil.copy(os.path.join(external_dir,"pthread-2.9","prebuild\\dll\\x86\\pthreadVC2.dll"),baseDir)


remove(build_dir+"\\exe")
try:
    os.mkdir(build_dir+"\\exe")
except:
    pass
shutil.copy(build_dir+"\\build\\pxScene.exe",build_dir+"\\exe")
copyRes(build_dir+"\\exe")