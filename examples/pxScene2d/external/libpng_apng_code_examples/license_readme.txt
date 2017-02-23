License for all code examples : zlib license
--------------------------------------------

--------------------------------------------
load1png.c  : loads static PNG file, using high-level png_read_png() function.

load2png.c  : loads static PNG file, using low-level libpng functions.

load3apng.c : loads APNG frames "as is", no composition.

load4apng.c : loads APNG with frames composition (blend and dispose operations)

load5apng.cpp : loads APNG file, saves all frames as PNG, no frames composition.
                (!) works without apng patch (!)

load6apng.cpp : loads APNG file, saves all frames as PNG (32bpp), with frames composition.
                (!) works without apng patch (!)

load7png.c    : progressive loading of PNG.

load8apng.c   : progressive loading of APNG.

resave2gray.c : converts any APNG to grayscale APNG: loads the whole animation, then saves it all.

resave2rgba.c : converts any APNG to 32bpp APNG, loads and saves animation frame by frame.

save1png.c  : saves static PNG file, using regular unpatched libpng.

save2apng.c : saves animated PNG file, using apng-patched libpng.

--------------------------------------------
To compile and test everything:

make

make test
