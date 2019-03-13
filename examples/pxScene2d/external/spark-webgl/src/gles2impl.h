#ifndef GLES2_IMPL_H_
#define GLES2_IMPL_H_

#include <cstring>

namespace gles2impl {

	std::string init(int width, int height, bool fullscreen, std::string title);
	void nextFrame(bool drawBuffers);
	void cleanup();

}

#endif /* GLES2_IMPL_H_ */
