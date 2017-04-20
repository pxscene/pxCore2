import shutil
import os
import datetime
external_dir = "../examples/pxScene2d/external"
curl_dir = os.path.join(external_dir, "curl-7.40.0")
curl_include_dir = os.path.join(curl_dir, "include", "curl")
libpng_dir = os.path.join(external_dir, "libpng-1.6.28")
jpeg_dir = os.path.join(external_dir, "jpeg-9a")
libjpeg_turbo_dir = os.path.join(external_dir, "libjpeg-turbo-1.5.1")
libjpeg_turbo_win_dir = os.path.join(libjpeg_turbo_dir, "win")
# mock make file to replace placeholder.
libjpeg_turbo_dict = {
   "@VERSION@": "1.5.1",
   "@BUILD@": datetime.date.today().strftime('%Y%m%d'),
   "@CMAKE_PROJECT_NAME@": "libjpeg-turbo",
   "@JPEG_LIB_VERSION@": "62",
   "@LIBJPEG_TURBO_VERSION_NUMBER@": "1005001",
   "#cmakedefine": "#define",
   "@BITS_IN_JSAMPLE@": "8"
}
# read source file, replace string content and write to target file
def replaceFile(sourceFile, targetFile):
    with open(sourceFile) as fin:
      content = fin.read()
      with open(targetFile, "w") as fout:
        for k,v in libjpeg_turbo_dict.iteritems():
            content = content.replace(k, v)
        fout.write(content)
    
# generate curlbuild.h, previous codes need to use make to generate curlbuild.h under linux but no such tool under windows directly
shutil.copy(os.path.join(curl_include_dir, "curlbuild-win.h"), os.path.join(curl_include_dir, "curlbuild.h"))

# generate pnglibconf.h for libpng
shutil.copy(os.path.join(libpng_dir, "scripts", "pnglibconf.h.prebuilt"), os.path.join(libpng_dir, "pnglibconf.h"))

# generate jconfig.h for jpeg-9a
shutil.copy(os.path.join(jpeg_dir, "jconfig.vc"), os.path.join(jpeg_dir, "jconfig.h"))

# generate jconfig.h for libjpeg-turb
replaceFile(os.path.join(libjpeg_turbo_win_dir, "jconfig.h.in"), os.path.join(libjpeg_turbo_dir, "jconfig.h"))

# generate jconfigint.h for libjpeg-turb
replaceFile(os.path.join(libjpeg_turbo_win_dir, "jconfigint.h.in"), os.path.join(libjpeg_turbo_dir, "jconfigint.h"))