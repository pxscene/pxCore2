#include <cstring>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <stdio.h>

#include "../gles2impl.h"

#include <GL/glew.h>
//#include <GLFW/glfw3.h>

using namespace std;

namespace gles2impl {

//GLFWwindow* window;

string init(int width, int height, bool fullscreen, std::string title) {
  printf("JRJR initializing gles2\n");
  #if 0
  printf("initializing GLEW\n");

  if (!glfwInit()) {
    return string("Can't init GLEW\n");
  }

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(width, height, title.c_str(), fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
  if (!window) {
      return string("Can't create window.");
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

#endif
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    return string("Can't init GLEW: ") + std::to_string((int)err);
  }

  return string("");
}

void nextFrame(bool swapBuffers) {
  #if 0
  if (glfwWindowShouldClose(window)) {
    exit(0);
  }

  if (swapBuffers) {
    glfwSwapBuffers(window);
  }

  glfwPollEvents();
  #endif
}

void cleanup() {
  #if 0
  glfwTerminate();

  printf("cleanup\n");
  #endif
}

} // end namespace gles2impl
