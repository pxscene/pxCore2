#ifndef WEBGL_H_
#define WEBGL_H_

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
    typedef double GLclampd;
#else
    #ifdef IS_GLEW
    #include <GL/glew.h>
    #else
    #include <GLES2/gl2.h>
    #endif
#endif


#include "../common.h"

using namespace node;
using namespace v8;

namespace webgl {

typedef uint8_t BYTE;

class WebGLRenderingContext : public ObjectWrap {
public:
  explicit WebGLRenderingContext();
  static void Initialize (Handle<Object> target);
  static void AtExit();

protected:
  int pixelStorei_UNPACK_FLIP_Y_WEBGL;
  int pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL;
  int pixelStorei_UNPACK_FLIP_BLUE_RED;

  GLint mInitialFrameBuffer;

  void preprocessTexImageData(void * pixels, int width, int height, int format, int type);

  static NAN_METHOD(New);

  static NAN_METHOD(Uniform1f);
  static NAN_METHOD(Uniform2f);
  static NAN_METHOD(Uniform3f);
  static NAN_METHOD(Uniform4f);
  static NAN_METHOD(Uniform1i);
  static NAN_METHOD(Uniform2i);
  static NAN_METHOD(Uniform3i);
  static NAN_METHOD(Uniform4i);
  static NAN_METHOD(Uniform1fv);
  static NAN_METHOD(Uniform2fv);
  static NAN_METHOD(Uniform3fv);
  static NAN_METHOD(Uniform4fv);
  static NAN_METHOD(Uniform1iv);
  static NAN_METHOD(Uniform2iv);
  static NAN_METHOD(Uniform3iv);
  static NAN_METHOD(Uniform4iv);
  static NAN_METHOD(PixelStorei);
  static NAN_METHOD(BindAttribLocation);
  static NAN_METHOD(GetError);
  static NAN_METHOD(DrawArrays);
  static NAN_METHOD(UniformMatrix2fv);
  static NAN_METHOD(UniformMatrix3fv);
  static NAN_METHOD(UniformMatrix4fv);
  static NAN_METHOD(GenerateMipmap);
  static NAN_METHOD(GetAttribLocation);
  static NAN_METHOD(DepthFunc);
  static NAN_METHOD(Viewport);
  static NAN_METHOD(CreateShader);
  static NAN_METHOD(ShaderSource);
  static NAN_METHOD(CompileShader);
  static NAN_METHOD(GetShaderParameter);
  static NAN_METHOD(GetShaderInfoLog);
  static NAN_METHOD(CreateProgram);
  static NAN_METHOD(AttachShader);
  static NAN_METHOD(LinkProgram);
  static NAN_METHOD(GetProgramParameter);
  static NAN_METHOD(GetUniformLocation);
  static NAN_METHOD(ClearColor);
  static NAN_METHOD(ClearDepth);
  static NAN_METHOD(Disable);
  static NAN_METHOD(Enable);
  static NAN_METHOD(CreateTexture);
  static NAN_METHOD(BindTexture);
  static NAN_METHOD(TexImage2D);
  static NAN_METHOD(TexParameteri);
  static NAN_METHOD(TexParameterf);
  static NAN_METHOD(Clear);
  static NAN_METHOD(UseProgram);
  static NAN_METHOD(CreateBuffer);
  static NAN_METHOD(BindBuffer);
  static NAN_METHOD(CreateFramebuffer);
  static NAN_METHOD(BindFramebuffer);
  static NAN_METHOD(FramebufferTexture2D);
  static NAN_METHOD(BufferData);
  static NAN_METHOD(BufferSubData);
  static NAN_METHOD(BlendEquation);
  static NAN_METHOD(BlendFunc);
  static NAN_METHOD(EnableVertexAttribArray);
  static NAN_METHOD(VertexAttribPointer);
  static NAN_METHOD(ActiveTexture);
  static NAN_METHOD(DrawElements);
  static NAN_METHOD(Flush);
  static NAN_METHOD(Finish);

  static NAN_METHOD(VertexAttrib1f);
  static NAN_METHOD(VertexAttrib2f);
  static NAN_METHOD(VertexAttrib3f);
  static NAN_METHOD(VertexAttrib4f);
  static NAN_METHOD(VertexAttrib1fv);
  static NAN_METHOD(VertexAttrib2fv);
  static NAN_METHOD(VertexAttrib3fv);
  static NAN_METHOD(VertexAttrib4fv);

  static NAN_METHOD(BlendColor);
  static NAN_METHOD(BlendEquationSeparate);
  static NAN_METHOD(BlendFuncSeparate);
  static NAN_METHOD(ClearStencil);
  static NAN_METHOD(ColorMask);
  static NAN_METHOD(CopyTexImage2D);
  static NAN_METHOD(CopyTexSubImage2D);
  static NAN_METHOD(CullFace);
  static NAN_METHOD(DepthMask);
  static NAN_METHOD(DepthRange);
  static NAN_METHOD(DisableVertexAttribArray);
  static NAN_METHOD(Hint);
  static NAN_METHOD(IsEnabled);
  static NAN_METHOD(LineWidth);
  static NAN_METHOD(PolygonOffset);

  static NAN_METHOD(SampleCoverage);
  static NAN_METHOD(Scissor);
  static NAN_METHOD(StencilFunc);
  static NAN_METHOD(StencilFuncSeparate);
  static NAN_METHOD(StencilMask);
  static NAN_METHOD(StencilMaskSeparate);
  static NAN_METHOD(StencilOp);
  static NAN_METHOD(StencilOpSeparate);
  static NAN_METHOD(BindRenderbuffer);
  static NAN_METHOD(CreateRenderbuffer);

  static NAN_METHOD(DeleteBuffer);
  static NAN_METHOD(DeleteFramebuffer);
  static NAN_METHOD(DeleteProgram);
  static NAN_METHOD(DeleteRenderbuffer);
  static NAN_METHOD(DeleteShader);
  static NAN_METHOD(DeleteTexture);
  static NAN_METHOD(DetachShader);
  static NAN_METHOD(FramebufferRenderbuffer);
  static NAN_METHOD(GetVertexAttribOffset);

  static NAN_METHOD(IsBuffer);
  static NAN_METHOD(IsFramebuffer);
  static NAN_METHOD(IsProgram);
  static NAN_METHOD(IsRenderbuffer);
  static NAN_METHOD(IsShader);
  static NAN_METHOD(IsTexture);

  static NAN_METHOD(RenderbufferStorage);
  static NAN_METHOD(GetShaderSource);
  static NAN_METHOD(ValidateProgram);

  static NAN_METHOD(TexSubImage2D);
  static NAN_METHOD(ReadPixels);
  static NAN_METHOD(GetTexParameter);
  static NAN_METHOD(GetActiveAttrib);
  static NAN_METHOD(GetActiveUniform);
  static NAN_METHOD(GetAttachedShaders);
  static NAN_METHOD(GetParameter);
  static NAN_METHOD(GetBufferParameter);
  static NAN_METHOD(GetFramebufferAttachmentParameter);
  static NAN_METHOD(GetProgramInfoLog);
  static NAN_METHOD(GetRenderbufferParameter);
  static NAN_METHOD(GetVertexAttrib);
  static NAN_METHOD(GetSupportedExtensions);
  static NAN_METHOD(GetExtension);
  static NAN_METHOD(CheckFramebufferStatus);

  static NAN_METHOD(FrontFace);

private:
  static Persistent<Function> constructor_template;
};

}

#endif /* WEBGL_H_ */
