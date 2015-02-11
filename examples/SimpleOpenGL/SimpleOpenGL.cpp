// Simple OpenGL Example CopyRight 2007-2009 John Robinson
// Demonstrates filling rendering with OpenGL

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxOffscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>

#ifdef PX_PLATFORM_X11
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <math.h>
#include <sys/time.h>
#else
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

GLuint glRotationUniform;
GLuint glPos;
GLuint glCol;

static const char *vert_shader_text =
    "uniform mat4 rotation;\n"
    "attribute vec4 pos;\n"
    "attribute vec4 color;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_Position = rotation * pos;\n"
    "  v_color = color;\n"
    "}\n";

static const char *frag_shader_text =
#ifndef PX_PLATFORM_X11
    "precision mediump float;\n"
#endif
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  gl_FragColor = v_color;\n"
    "}\n";

static GLuint create_shader(const char *source, GLenum shader_type)
{
    GLuint shader;
    GLint status;

    shader = glCreateShader(shader_type);
    assert(shader != 0);

    glShaderSource(shader, 1, (const char **) &source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[1000];
        GLsizei len;
        glGetShaderInfoLog(shader, 1000, &len, log);
        fprintf(stderr, "Error: compiling %s: %*s\n",
            shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
            len, log);
        exit(1);
    }

    return shader;
}

static void initializeGl()
{
    GLuint frag, vert;
    GLuint program;
    GLint status;

    frag = create_shader(frag_shader_text, GL_FRAGMENT_SHADER);
    vert = create_shader(vert_shader_text, GL_VERTEX_SHADER);

    program = glCreateProgram();
    glAttachShader(program, frag);
    glAttachShader(program, vert);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char log[1000];
        GLsizei len;
        glGetProgramInfoLog(program, 1000, &len, log);
        fprintf(stderr, "Error: linking:\n%*s\n", len, log);
        exit(1);
    }

    glUseProgram(program);

    glPos = 0;
    glCol = 1;

    glBindAttribLocation(program, glPos, "pos");
    glBindAttribLocation(program, glCol, "color");
    glLinkProgram(program);

    glRotationUniform = glGetUniformLocation(program, "rotation");
}

static void redraw(pxSurfaceNative nativeSurface)
{
    //struct window *window = data;
    static const GLfloat verts[3][2] = {
        { -0.5, -0.5 },
        {  0.5, -0.5 },
        {  0,    0.5 }
    };
    static const GLfloat colors[3][3] = {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 }
    };
    GLfloat angle;
    GLfloat rotation[4][4] = {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 }
    };
    static const uint32_t speed_div = 5;
    struct timeval tv;
    uint32_t time;

    gettimeofday(&tv, NULL);
    time = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    angle = (time / speed_div) % 360 * M_PI / 180.0;
    rotation[0][0] =  cos(angle);
    rotation[0][2] =  sin(angle);
    rotation[2][0] = -sin(angle);
    rotation[2][2] =  cos(angle);

    glViewport(0, 0, nativeSurface->windowWidth, nativeSurface->windowHeight);

    glUniformMatrix4fv(glRotationUniform, 1, GL_FALSE,
               (GLfloat *) rotation);

    glClearColor(0.0, 0.0, 0.0, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);

    glVertexAttribPointer(glPos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(glCol, 3, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(glPos);
    glEnableVertexAttribArray(glCol);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(glPos);
    glDisableVertexAttribArray(glCol);
}

pxEventLoop eventLoop;

class myWindow: public pxWindow
{
private:
    // Event Handlers - Look in pxWindow.h for more
    void onCloseRequest()
    {
        // When someone clicks the close box no policy is predefined.
        // so we need to explicitly tell the event loop to exit
	eventLoop.exit();
    }

    void onSize(int newWidth, int newHeight)
    {
        //todo
    }

    void onDraw(pxSurfaceNative s)
    {
        redraw(s);
    }
};

int pxMain()
{
    myWindow win;

    win.init(10, 64, 250, 250);
    initializeGl();
    win.setTitle("Simple OpenGL");
    win.setVisibility(true);

    eventLoop.run();

    return 0;
}


