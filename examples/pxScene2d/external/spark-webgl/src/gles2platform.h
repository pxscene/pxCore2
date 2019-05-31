#ifndef GLES2PLATFORM_H_
#define GLES2PLATFORM_H_

#include "common.h"

using namespace node;
using namespace v8;

namespace gles2platform {

void AtExit();

NAN_METHOD(init);
NAN_METHOD(nextFrame);

}

#endif /* GLES2PLATFORM_H_ */
