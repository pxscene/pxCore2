#include "pxSharedContext.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <map>
#include "rtLog.h"
#include "rtMutex.h"


// TODO webgl won't work until this is wired up

pxSharedContextNative::pxSharedContextNative(bool depthBuffer):context(NULL) {}

pxSharedContextNative::~pxSharedContextNative() {}

void pxSharedContext::makeCurrent(bool f) {}