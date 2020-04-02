#include <cstring>
#include <vector>
#include <iostream>

#include "webgl.h"
#include <node.h>
#include <node_buffer.h>

#define LOGGING 1

void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

void _CheckGLError(const char* file, int line)
{
    GLenum err ( glGetError() );

    while ( err != GL_NO_ERROR )
    {
        std::string error;
        switch ( err )
        {
            case GL_INVALID_OPERATION:  error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:       error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:      error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:      error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        std::cout << "GL_" << error.c_str() <<" - " << file << ":" << line << std::endl;
        err = glGetError();
    }

    return;
}

#ifdef _WIN32
  #define  strcasestr(s, t) strstr(strupr(s), strupr(t))
#endif

#define CHECK_ARRAY_BUFFER(val) if(!val->IsArrayBufferView()) \
        {Nan::ThrowTypeError("Only support array buffer"); return;}

namespace webgl {

using namespace node;
using namespace v8;
using namespace std;

// forward declarations
enum GLObjectType {
  GLOBJECT_TYPE_BUFFER,
  GLOBJECT_TYPE_FRAMEBUFFER,
  GLOBJECT_TYPE_PROGRAM,
  GLOBJECT_TYPE_RENDERBUFFER,
  GLOBJECT_TYPE_SHADER,
  GLOBJECT_TYPE_TEXTURE,
};

void registerGLObj(GLObjectType type, GLuint obj);
void unregisterGLObj(GLuint obj);

// A 32-bit and 64-bit compatible way of converting a pointer to a GLuint.
static GLuint ToGLuint(const void* ptr) {
  return static_cast<GLuint>(reinterpret_cast<size_t>(ptr));
}

template<typename Type>
inline Type* getArrayData(Local<Value> arg, int* num = NULL) {
  Type *data=NULL;
  if(num) *num=0;

  if(!arg->IsNull()) {
    if(arg->IsArray()) {
      Nan::ThrowError("Not support array type");
    }
    else if(arg->IsObject()) {
      Nan::TypedArrayContents<Type> p(arg);
      data = *p;
      if (num) *num = p.length();
    }
    else
      Nan::ThrowError("Bad array argument");
  }

  return data;
}

inline void *getImageData(Local<Value> arg) {
  void *pixels = NULL;
  if (!arg->IsNull()) {
    Local<Object> obj = Local<Object>::Cast(arg);
    if (!obj->IsObject()){
      Nan::ThrowError("Bad texture argument");
    }else if(obj->IsArrayBufferView()){
        int num;
        pixels = getArrayData<BYTE>(obj, &num);
    }else{
        pixels = node::Buffer::Data(Nan::Get(obj, JS_STR("data")).ToLocalChecked());
    }
  }
  return pixels;
}

Persistent<Function> WebGLRenderingContext::constructor_template;

void WebGLRenderingContext::Initialize (Handle<Object> target) {
  Nan::HandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("WebGLRenderingContext"));

  // prototype
  Nan::SetPrototypeMethod(ctor, "uniform1f", Uniform1f);
  Nan::SetPrototypeMethod(ctor, "uniform2f", Uniform2f);
  Nan::SetPrototypeMethod(ctor, "uniform3f", Uniform3f);
  Nan::SetPrototypeMethod(ctor, "uniform4f", Uniform4f);
  Nan::SetPrototypeMethod(ctor, "uniform1i", Uniform1i);
  Nan::SetPrototypeMethod(ctor, "uniform2i", Uniform2i);
  Nan::SetPrototypeMethod(ctor, "uniform3i", Uniform3i);
  Nan::SetPrototypeMethod(ctor, "uniform4i", Uniform4i);
  Nan::SetPrototypeMethod(ctor, "uniform1fv", Uniform1fv);
  Nan::SetPrototypeMethod(ctor, "uniform2fv", Uniform2fv);
  Nan::SetPrototypeMethod(ctor, "uniform3fv", Uniform3fv);
  Nan::SetPrototypeMethod(ctor, "uniform4fv", Uniform4fv);
  Nan::SetPrototypeMethod(ctor, "uniform1iv", Uniform1iv);
  Nan::SetPrototypeMethod(ctor, "uniform2iv", Uniform2iv);
  Nan::SetPrototypeMethod(ctor, "uniform3iv", Uniform3iv);
  Nan::SetPrototypeMethod(ctor, "uniform4iv", Uniform4iv);
  Nan::SetPrototypeMethod(ctor, "pixelStorei", PixelStorei);
  Nan::SetPrototypeMethod(ctor, "bindAttribLocation", BindAttribLocation);
  Nan::SetPrototypeMethod(ctor, "getError", GetError);
  Nan::SetPrototypeMethod(ctor, "drawArrays", DrawArrays);
  Nan::SetPrototypeMethod(ctor, "uniformMatrix2fv", UniformMatrix2fv);
  Nan::SetPrototypeMethod(ctor, "uniformMatrix3fv", UniformMatrix3fv);
  Nan::SetPrototypeMethod(ctor, "uniformMatrix4fv", UniformMatrix4fv);

  Nan::SetPrototypeMethod(ctor, "generateMipmap", GenerateMipmap);

  Nan::SetPrototypeMethod(ctor, "getAttribLocation", GetAttribLocation);
  Nan::SetPrototypeMethod(ctor, "depthFunc", DepthFunc);
  Nan::SetPrototypeMethod(ctor, "viewport", Viewport);
  Nan::SetPrototypeMethod(ctor, "createShader", CreateShader);
  Nan::SetPrototypeMethod(ctor, "shaderSource", ShaderSource);
  Nan::SetPrototypeMethod(ctor, "compileShader", CompileShader);
  Nan::SetPrototypeMethod(ctor, "getShaderParameter", GetShaderParameter);
  Nan::SetPrototypeMethod(ctor, "getShaderInfoLog", GetShaderInfoLog);
  Nan::SetPrototypeMethod(ctor, "createProgram", CreateProgram);
  Nan::SetPrototypeMethod(ctor, "attachShader", AttachShader);
  Nan::SetPrototypeMethod(ctor, "linkProgram", LinkProgram);
  Nan::SetPrototypeMethod(ctor, "getProgramParameter", GetProgramParameter);
  Nan::SetPrototypeMethod(ctor, "getUniformLocation", GetUniformLocation);
  Nan::SetPrototypeMethod(ctor, "clearColor", ClearColor);
  Nan::SetPrototypeMethod(ctor, "clearDepth", ClearDepth);

  Nan::SetPrototypeMethod(ctor, "disable", Disable);
  Nan::SetPrototypeMethod(ctor, "createTexture", CreateTexture);
  Nan::SetPrototypeMethod(ctor, "bindTexture", BindTexture);
  Nan::SetPrototypeMethod(ctor, "texImage2D", TexImage2D);
  Nan::SetPrototypeMethod(ctor, "texParameteri", TexParameteri);
  Nan::SetPrototypeMethod(ctor, "texParameterf", TexParameterf);
  Nan::SetPrototypeMethod(ctor, "clear", Clear);
  Nan::SetPrototypeMethod(ctor, "useProgram", UseProgram);
  Nan::SetPrototypeMethod(ctor, "createFramebuffer", CreateFramebuffer);
  Nan::SetPrototypeMethod(ctor, "bindFramebuffer", BindFramebuffer);
  Nan::SetPrototypeMethod(ctor, "framebufferTexture2D", FramebufferTexture2D);
  Nan::SetPrototypeMethod(ctor, "createBuffer", CreateBuffer);
  Nan::SetPrototypeMethod(ctor, "bindBuffer", BindBuffer);
  Nan::SetPrototypeMethod(ctor, "bufferData", BufferData);
  Nan::SetPrototypeMethod(ctor, "bufferSubData", BufferSubData);
  Nan::SetPrototypeMethod(ctor, "enable", Enable);
  Nan::SetPrototypeMethod(ctor, "blendEquation", BlendEquation);
  Nan::SetPrototypeMethod(ctor, "blendFunc", BlendFunc);
  Nan::SetPrototypeMethod(ctor, "enableVertexAttribArray", EnableVertexAttribArray);
  Nan::SetPrototypeMethod(ctor, "vertexAttribPointer", VertexAttribPointer);
  Nan::SetPrototypeMethod(ctor, "activeTexture", ActiveTexture);
  Nan::SetPrototypeMethod(ctor, "drawElements", DrawElements);
  Nan::SetPrototypeMethod(ctor, "flush", Flush);
  Nan::SetPrototypeMethod(ctor, "finish", Finish);

  Nan::SetPrototypeMethod(ctor, "vertexAttrib1f", VertexAttrib1f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib2f", VertexAttrib2f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib3f", VertexAttrib3f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib4f", VertexAttrib4f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib1fv", VertexAttrib1fv);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib2fv", VertexAttrib2fv);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib3fv", VertexAttrib3fv);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib4fv", VertexAttrib4fv);

  Nan::SetPrototypeMethod(ctor, "blendColor", BlendColor);
  Nan::SetPrototypeMethod(ctor, "blendEquationSeparate", BlendEquationSeparate);
  Nan::SetPrototypeMethod(ctor, "blendFuncSeparate", BlendFuncSeparate);
  Nan::SetPrototypeMethod(ctor, "clearStencil", ClearStencil);
  Nan::SetPrototypeMethod(ctor, "colorMask", ColorMask);
  Nan::SetPrototypeMethod(ctor, "copyTexImage2D", CopyTexImage2D);
  Nan::SetPrototypeMethod(ctor, "copyTexSubImage2D", CopyTexSubImage2D);
  Nan::SetPrototypeMethod(ctor, "cullFace", CullFace);
  Nan::SetPrototypeMethod(ctor, "depthMask", DepthMask);
  Nan::SetPrototypeMethod(ctor, "depthRange", DepthRange);
  Nan::SetPrototypeMethod(ctor, "disableVertexAttribArray", DisableVertexAttribArray);
  Nan::SetPrototypeMethod(ctor, "hint", Hint);
  Nan::SetPrototypeMethod(ctor, "isEnabled", IsEnabled);
  Nan::SetPrototypeMethod(ctor, "lineWidth", LineWidth);
  Nan::SetPrototypeMethod(ctor, "polygonOffset", PolygonOffset);

  Nan::SetPrototypeMethod(ctor, "sampleCoverage", SampleCoverage);
  Nan::SetPrototypeMethod(ctor, "scissor", Scissor);
  Nan::SetPrototypeMethod(ctor, "stencilFunc", StencilFunc);
  Nan::SetPrototypeMethod(ctor, "stencilFuncSeparate", StencilFuncSeparate);
  Nan::SetPrototypeMethod(ctor, "stencilMask", StencilMask);
  Nan::SetPrototypeMethod(ctor, "stencilMaskSeparate", StencilMaskSeparate);
  Nan::SetPrototypeMethod(ctor, "stencilOp", StencilOp);
  Nan::SetPrototypeMethod(ctor, "stencilOpSeparate", StencilOpSeparate);
  Nan::SetPrototypeMethod(ctor, "bindRenderbuffer", BindRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "createRenderbuffer", CreateRenderbuffer);

  Nan::SetPrototypeMethod(ctor, "deleteBuffer", DeleteBuffer);
  Nan::SetPrototypeMethod(ctor, "deleteFramebuffer", DeleteFramebuffer);
  Nan::SetPrototypeMethod(ctor, "deleteProgram", DeleteProgram);
  Nan::SetPrototypeMethod(ctor, "deleteRenderbuffer", DeleteRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "deleteShader", DeleteShader);
  Nan::SetPrototypeMethod(ctor, "deleteTexture", DeleteTexture);
  Nan::SetPrototypeMethod(ctor, "detachShader", DetachShader);
  Nan::SetPrototypeMethod(ctor, "framebufferRenderbuffer", FramebufferRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "getVertexAttribOffset", GetVertexAttribOffset);

  Nan::SetPrototypeMethod(ctor, "isBuffer", IsBuffer);
  Nan::SetPrototypeMethod(ctor, "isFramebuffer", IsFramebuffer);
  Nan::SetPrototypeMethod(ctor, "isProgram", IsProgram);
  Nan::SetPrototypeMethod(ctor, "isRenderbuffer", IsRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "isShader", IsShader);
  Nan::SetPrototypeMethod(ctor, "isTexture", IsTexture);

  Nan::SetPrototypeMethod(ctor, "renderbufferStorage", RenderbufferStorage);
  Nan::SetPrototypeMethod(ctor, "getShaderSource", GetShaderSource);
  Nan::SetPrototypeMethod(ctor, "validateProgram", ValidateProgram);

  Nan::SetPrototypeMethod(ctor, "texSubImage2D", TexSubImage2D);
  Nan::SetPrototypeMethod(ctor, "readPixels", ReadPixels);
  Nan::SetPrototypeMethod(ctor, "getTexParameter", GetTexParameter);
  Nan::SetPrototypeMethod(ctor, "getActiveAttrib", GetActiveAttrib);
  Nan::SetPrototypeMethod(ctor, "getActiveUniform", GetActiveUniform);
  Nan::SetPrototypeMethod(ctor, "getAttachedShaders", GetAttachedShaders);
  Nan::SetPrototypeMethod(ctor, "getParameter", GetParameter);
  Nan::SetPrototypeMethod(ctor, "getBufferParameter", GetBufferParameter);
  Nan::SetPrototypeMethod(ctor, "getFramebufferAttachmentParameter", GetFramebufferAttachmentParameter);
  Nan::SetPrototypeMethod(ctor, "getProgramInfoLog", GetProgramInfoLog);
  Nan::SetPrototypeMethod(ctor, "getRenderbufferParameter", GetRenderbufferParameter);
  Nan::SetPrototypeMethod(ctor, "getVertexAttrib", GetVertexAttrib);
  Nan::SetPrototypeMethod(ctor, "getSupportedExtensions", GetSupportedExtensions);
  Nan::SetPrototypeMethod(ctor, "getExtension", GetExtension);
  Nan::SetPrototypeMethod(ctor, "checkFramebufferStatus", CheckFramebufferStatus);

  Nan::SetPrototypeMethod(ctor, "frontFace", FrontFace);

  Nan::Set(target, JS_STR("WebGLRenderingContext"), ctor->GetFunction());

  constructor_template.Reset(Isolate::GetCurrent(), ctor->GetFunction());
}

WebGLRenderingContext::WebGLRenderingContext() {
  pixelStorei_UNPACK_FLIP_Y_WEBGL = 0;
  pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0;
  pixelStorei_UNPACK_FLIP_BLUE_RED = 0;
  
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mInitialFrameBuffer);
  CheckGLError();
}

NAN_METHOD(WebGLRenderingContext::New) {
  Nan::HandleScope scope;

  WebGLRenderingContext* obj = new WebGLRenderingContext();
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(WebGLRenderingContext::Uniform1f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();

  glUniform1f(location, x);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();
  float y = (float) info[2]->NumberValue();

  glUniform2f(location, x, y);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();
  float y = (float) info[2]->NumberValue();
  float z = (float) info[3]->NumberValue();

  glUniform3f(location, x, y, z);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();
  float y = (float) info[2]->NumberValue();
  float z = (float) info[3]->NumberValue();
  float w = (float) info[4]->NumberValue();

  glUniform4f(location, x, y, z, w);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform1i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();

  glUniform1i(location, x);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();

  glUniform2i(location, x, y);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();
  int z = info[3]->Int32Value();

  glUniform3i(location, x, y, z);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();
  int z = info[3]->Int32Value();
  int w = info[4]->Int32Value();

  glUniform4i(location, x, y, z, w);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform1fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  glUniform1fv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 2;

  glUniform2fv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 3;

  glUniform3fv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 4;

  glUniform4fv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform1iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);

  glUniform1iv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 2;

  glUniform2iv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 3;
  glUniform3iv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 4;
  glUniform4iv(location, num, ptr);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::PixelStorei) {
  Nan::HandleScope scope;

  int pname = info[0]->Int32Value();
  int param = info[1]->Int32Value();

  if (pname == 0x9240 /* UNPACK_FLIP_Y_WEBGL */) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->pixelStorei_UNPACK_FLIP_Y_WEBGL = param;
  } else if (pname == 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL */) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL = param;
  } else if (pname == 0x9245 /* UNPACK_FLIP_BLUE_RED */) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->pixelStorei_UNPACK_FLIP_BLUE_RED = param;
  }

  glPixelStorei(pname,param);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BindAttribLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  int index = info[1]->Int32Value();
  String::Utf8Value name(info[2]);

  glBindAttribLocation(program, index, *name);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetError) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(Nan::New<Integer>(glGetError()));
}


NAN_METHOD(WebGLRenderingContext::DrawArrays) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value();
  int first = info[1]->Int32Value();
  int count = info[2]->Int32Value();

  glDrawArrays(mode, first, count);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix2fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value();
  GLboolean transpose = info[1]->BooleanValue();

  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 4) {
    Nan::ThrowError("Not enough data for UniformMatrix2fv");
  }else{
    glUniformMatrix2fv(location, count / 4, transpose, data);
    CheckGLError();
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix3fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value();
  GLboolean transpose = info[1]->BooleanValue();
  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 9) {
    Nan::ThrowError("Not enough data for UniformMatrix3fv");
  }else{
    glUniformMatrix3fv(location, count / 9, transpose, data);
    CheckGLError();
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix4fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value();
  GLboolean transpose = info[1]->BooleanValue();
  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 16) {
    Nan::ThrowError("Not enough data for UniformMatrix4fv");
  }else{
    glUniformMatrix4fv(location, count / 16, transpose, data);
    CheckGLError();
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(WebGLRenderingContext::GenerateMipmap) {
  Nan::HandleScope scope;

  GLint target = info[0]->Int32Value();
  glGenerateMipmap(target);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetAttribLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  String::Utf8Value name(info[1]);

  GLint location = glGetAttribLocation(program, *name);
  CheckGLError();

  info.GetReturnValue().Set(Nan::New<Number>(location));
}


NAN_METHOD(WebGLRenderingContext::DepthFunc) {
  Nan::HandleScope scope;

  glDepthFunc(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::Viewport) {
  Nan::HandleScope scope;

  int x = info[0]->Int32Value();
  int y = info[1]->Int32Value();
  int width = info[2]->Int32Value();
  int height = info[3]->Int32Value();

  glViewport(x, y, width, height);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateShader) {
  Nan::HandleScope scope;

  GLuint shader=glCreateShader(info[0]->Int32Value());
  CheckGLError();
  #ifdef LOGGING
  cout<<"createShader "<<shader<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_SHADER, shader);
  info.GetReturnValue().Set(Nan::New<Number>(shader));
}


NAN_METHOD(WebGLRenderingContext::ShaderSource) {
  Nan::HandleScope scope;

  int id = info[0]->Int32Value();
  String::Utf8Value code(info[1]);

  const char* codes[1];
  codes[0] = *code;
  GLint length=code.length();

  glShaderSource  (id, 1, codes, &length);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CompileShader) {
  Nan::HandleScope scope;

  glCompileShader(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::FrontFace) {
  Nan::HandleScope scope;

  glFrontFace(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetShaderParameter) {
  Nan::HandleScope scope;

  int shader = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int value = 0;
  switch (pname) {
  case GL_DELETE_STATUS:
  case GL_COMPILE_STATUS:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value!=0)));
    break;
  case GL_SHADER_TYPE:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<unsigned long>(value)));
    break;
  case GL_INFO_LOG_LENGTH:
  case GL_SHADER_SOURCE_LENGTH:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
    break;
  default:
    Nan::ThrowTypeError("GetShaderParameter: Invalid Enum");
  }
  CheckGLError();
  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetShaderInfoLog) {
  Nan::HandleScope scope;

  int id = info[0]->Int32Value();
  int Len = 1024;
  char Error[1024];
  glGetShaderInfoLog(id, 1024, &Len, Error);
  CheckGLError();
  info.GetReturnValue().Set(JS_STR(Error));
}


NAN_METHOD(WebGLRenderingContext::CreateProgram) {
  Nan::HandleScope scope;

  GLuint program=glCreateProgram();
  CheckGLError();
  #ifdef LOGGING
  cout<<"createProgram "<<program<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_PROGRAM, program);
  info.GetReturnValue().Set(Nan::New<Number>(program));
}


NAN_METHOD(WebGLRenderingContext::AttachShader) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  int shader = info[1]->Int32Value();

  glAttachShader(program, shader);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::LinkProgram) {
  Nan::HandleScope scope;

  glLinkProgram(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetProgramParameter) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  int pname = info[1]->Int32Value();

  int value = 0;
  switch (pname) {
  case GL_DELETE_STATUS:
  case GL_LINK_STATUS:
  case GL_VALIDATE_STATUS:
    glGetProgramiv(program, pname, &value);
    info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value!=0)));
    break;
  case GL_ATTACHED_SHADERS:
  case GL_ACTIVE_ATTRIBUTES:
  case GL_ACTIVE_UNIFORMS:
    glGetProgramiv(program, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
    break;
  default:
    Nan::ThrowTypeError("GetProgramParameter: Invalid Enum");
  }
  //info.GetReturnValue().Set(Nan::Undefined());
  CheckGLError();
}


NAN_METHOD(WebGLRenderingContext::GetUniformLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value();
  v8::String::Utf8Value name(info[1]);

  GLint location = glGetUniformLocation(program, *name);
  CheckGLError();

  info.GetReturnValue().Set(JS_INT(location));
}


NAN_METHOD(WebGLRenderingContext::ClearColor) {
  Nan::HandleScope scope;

  float red = (float) info[0]->NumberValue();
  float green = (float) info[1]->NumberValue();
  float blue = (float) info[2]->NumberValue();
  float alpha = (float) info[3]->NumberValue();

  glClearColor(red, green, blue, alpha);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::ClearDepth) {
  Nan::HandleScope scope;

  float depth = (float) info[0]->NumberValue();

  glClearDepthf(depth);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Disable) {
  Nan::HandleScope scope;

  glDisable(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Enable) {
  Nan::HandleScope scope;

  glEnable(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CreateTexture) {
  Nan::HandleScope scope;

  GLuint texture;
  glGenTextures(1, &texture);
  CheckGLError();
  #ifdef LOGGING
  cout<<"createTexture "<<texture<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_TEXTURE, texture);
  info.GetReturnValue().Set(Nan::New<Number>(texture));
}


NAN_METHOD(WebGLRenderingContext::BindTexture) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int texture = info[1]->IsNull() ? 0 : info[1]->Int32Value();
#ifdef LOGGING
  cout<<"bindTexture target: " << target << " texture: " <<texture<<endl;
#endif
  glBindTexture(target, texture);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::TexImage2D) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int level = info[1]->Int32Value();
  int internalformat = info[2]->Int32Value();
  int width = info[3]->Int32Value();
  int height = info[4]->Int32Value();
  int border = info[5]->Int32Value();
  int format = info[6]->Int32Value();
  int type = info[7]->Int32Value();
  void *pixels=getImageData(info[8]);

#ifdef LOGGING
  cout  <<  "TexImage2D() target: " << target << " level: " << level << " internalformat: " << internalformat \
        << " width: " << width << " height: " << height << " border: " << border << " format: " << format \
        << " type: " << type << " pixels: " << pixels << endl;
#endif

  if (pixels) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->preprocessTexImageData(pixels, width, height, format, type);
  }

  glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::TexParameteri) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int param = info[2]->Int32Value();

  glTexParameteri(target, pname, param);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::TexParameterf) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  float param = (float) info[2]->NumberValue();

  glTexParameterf(target, pname, param);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::Clear) {
  Nan::HandleScope scope;

  glClear(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::UseProgram) {
  Nan::HandleScope scope;

  glUseProgram(info[0]->Int32Value());
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateBuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  glGenBuffers(1, &buffer);
  CheckGLError();
  #ifdef LOGGING
  cout<<"createBuffer "<<buffer<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_BUFFER, buffer);
  CheckGLError();
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}

NAN_METHOD(WebGLRenderingContext::BindBuffer) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int buffer = info[1]->Uint32Value();
  glBindBuffer(target,buffer);
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CreateFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  glGenFramebuffers(1, &buffer);
  CheckGLError();
  #ifdef LOGGING
  cout<<"createFrameBuffer "<<buffer<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_FRAMEBUFFER, buffer);
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}


NAN_METHOD(WebGLRenderingContext::BindFramebuffer) {
  Nan::HandleScope scope;

  WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());

  int target = info[0]->Int32Value();
  int buffer = info[1]->IsNull() ? 0 : info[1]->Int32Value();

  buffer = (buffer==0)?obj->mInitialFrameBuffer:buffer;

  glBindFramebuffer(target, buffer);
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::FramebufferTexture2D) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int attachment = info[1]->Int32Value();
  int textarget = info[2]->Int32Value();
  int texture = info[3]->Int32Value();
  int level = info[4]->Int32Value();

  glFramebufferTexture2D(target, attachment, textarget, texture, level);
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BufferData) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  if(info[1]->IsObject()) {
    Local<Object> obj = Local<Object>::Cast(info[1]);
    GLenum usage = info[2]->Int32Value();

    CHECK_ARRAY_BUFFER(obj);

    int element_size = 1;
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
    int size = arr->ByteLength()* element_size;
    void* data = arr->Buffer()->GetContents().Data();

    glBufferData(target, size, data, usage);
    CheckGLError();
  }
  else if(info[1]->IsNumber()) {
    GLsizeiptr size = info[1]->Uint32Value();
    GLenum usage = info[2]->Int32Value();
    glBufferData(target, size, NULL, usage);
    CheckGLError();
  }
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BufferSubData) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int offset = info[1]->Int32Value();
  Local<Object> obj = Local<Object>::Cast(info[2]);

   int element_size = 1;
   Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
   int size = arr->ByteLength()* element_size;
   void* data = arr->Buffer()->GetContents().Data();

  glBufferSubData(target, offset, size, data);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BlendEquation) {
  Nan::HandleScope scope;

  int mode=info[0]->Int32Value();;

  glBlendEquation(mode);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BlendFunc) {
  Nan::HandleScope scope;

  int sfactor=info[0]->Int32Value();;
  int dfactor=info[1]->Int32Value();;

  glBlendFunc(sfactor,dfactor);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::EnableVertexAttribArray) {
  Nan::HandleScope scope;

  glEnableVertexAttribArray(info[0]->Int32Value());
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::VertexAttribPointer) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  int size = info[1]->Int32Value();
  int type = info[2]->Int32Value();
  int normalized = info[3]->BooleanValue();
  int stride = info[4]->Int32Value();
  long offset = info[5]->Int32Value();

  //    printf("VertexAttribPointer %d %d %d %d %d %d\n", indx, size, type, normalized, stride, offset);
  glVertexAttribPointer(indx, size, type, normalized, stride, (const GLvoid *)offset);
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::ActiveTexture) {
  Nan::HandleScope scope;

  glActiveTexture(info[0]->Int32Value());
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::DrawElements) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value();
  int count = info[1]->Int32Value();
  int type = info[2]->Int32Value();
  GLvoid *offset = reinterpret_cast<GLvoid*>(info[3]->Uint32Value());
#ifdef LOGGING
  cout << "DrawElements() mode: " << mode << " count: " << count << " type: " << type << " offset: " << offset << endl;
#endif
  glDrawElements(mode, count, type, offset);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::Flush) {
  Nan::HandleScope scope;
  glFlush();
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Finish) {
  Nan::HandleScope scope;
  glFinish();
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib1f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();

  glVertexAttrib1f(indx, x);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib2f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();
  float y = (float) info[2]->NumberValue();

  glVertexAttrib2f(indx, x, y);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib3f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();
  float y = (float) info[2]->NumberValue();
  float z = (float) info[3]->NumberValue();

  glVertexAttrib3f(indx, x, y, z);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib4f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value();
  float x = (float) info[1]->NumberValue();
  float y = (float) info[2]->NumberValue();
  float z = (float) info[3]->NumberValue();
  float w = (float) info[4]->NumberValue();

  glVertexAttrib4f(indx, x, y, z, w);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib1fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib1fv(indx, data);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib2fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib2fv(indx, data);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib3fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib3fv(indx, data);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib4fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib4fv(indx, data);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendColor) {
  Nan::HandleScope scope;

  GLclampf r= (float) info[0]->NumberValue();
  GLclampf g= (float) info[1]->NumberValue();
  GLclampf b= (float) info[2]->NumberValue();
  GLclampf a= (float) info[3]->NumberValue();

  glBlendColor(r,g,b,a);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendEquationSeparate) {
  Nan::HandleScope scope;

  GLenum modeRGB= info[0]->Int32Value();
  GLenum modeAlpha= info[1]->Int32Value();

  glBlendEquationSeparate(modeRGB,modeAlpha);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendFuncSeparate) {
  Nan::HandleScope scope;

  GLenum srcRGB= info[0]->Int32Value();
  GLenum dstRGB= info[1]->Int32Value();
  GLenum srcAlpha= info[2]->Int32Value();
  GLenum dstAlpha= info[3]->Int32Value();

  glBlendFuncSeparate(srcRGB,dstRGB,srcAlpha,dstAlpha);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ClearStencil) {
  Nan::HandleScope scope;

  GLint s = info[0]->Int32Value();

  glClearStencil(s);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ColorMask) {
  Nan::HandleScope scope;

  GLboolean r = info[0]->BooleanValue();
  GLboolean g = info[1]->BooleanValue();
  GLboolean b = info[2]->BooleanValue();
  GLboolean a = info[3]->BooleanValue();

  glColorMask(r,g,b,a);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CopyTexImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLint level = info[1]->Int32Value();
  GLenum internalformat = info[2]->Int32Value();
  GLint x = info[3]->Int32Value();
  GLint y = info[4]->Int32Value();
  GLsizei width = info[5]->Int32Value();
  GLsizei height = info[6]->Int32Value();
  GLint border = info[7]->Int32Value();

  glCopyTexImage2D( target, level, internalformat, x, y, width, height, border);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CopyTexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLint level = info[1]->Int32Value();
  GLint xoffset = info[2]->Int32Value();
  GLint yoffset = info[3]->Int32Value();
  GLint x = info[4]->Int32Value();
  GLint y = info[5]->Int32Value();
  GLsizei width = info[6]->Int32Value();
  GLsizei height = info[7]->Int32Value();

  glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CullFace) {
  Nan::HandleScope scope;

  GLenum mode = info[0]->Int32Value();

  glCullFace(mode);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DepthMask) {
  Nan::HandleScope scope;

  GLboolean flag = info[0]->BooleanValue();

  glDepthMask(flag);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DepthRange) {
  Nan::HandleScope scope;

  GLclampf zNear = (float) info[0]->NumberValue();
  GLclampf zFar = (float) info[1]->NumberValue();

  glDepthRangef(zNear, zFar);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DisableVertexAttribArray) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Int32Value();

  glDisableVertexAttribArray(index);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Hint) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum mode = info[1]->Int32Value();

  glHint(target, mode);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::IsEnabled) {
  Nan::HandleScope scope;

  GLenum cap = info[0]->Int32Value();

  bool ret=glIsEnabled(cap)!=0;
  CheckGLError();
  info.GetReturnValue().Set(Nan::New<Boolean>(ret));
}

NAN_METHOD(WebGLRenderingContext::LineWidth) {
  Nan::HandleScope scope;

  GLfloat width = (float) info[0]->NumberValue();

  glLineWidth(width);
  CheckGLError();

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::PolygonOffset) {
  Nan::HandleScope scope;

  GLfloat factor = (float) info[0]->NumberValue();
  GLfloat units = (float) info[1]->NumberValue();

  glPolygonOffset(factor, units);
  CheckGLError();

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::SampleCoverage) {
  Nan::HandleScope scope;

  GLclampf value = (float) info[0]->NumberValue();
  GLboolean invert = info[1]->BooleanValue();

  glSampleCoverage(value, invert);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Scissor) {
  Nan::HandleScope scope;

  GLint x = info[0]->Int32Value();
  GLint y = info[1]->Int32Value();
  GLsizei width = info[2]->Int32Value();
  GLsizei height = info[3]->Int32Value();

  glScissor(x, y, width, height);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilFunc) {
  Nan::HandleScope scope;

  GLenum func = info[0]->Int32Value();
  GLint ref = info[1]->Int32Value();
  GLuint mask = info[2]->Int32Value();

  glStencilFunc(func, ref, mask);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilFuncSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value();
  GLenum func = info[1]->Int32Value();
  GLint ref = info[2]->Int32Value();
  GLuint mask = info[3]->Int32Value();

  glStencilFuncSeparate(face, func, ref, mask);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilMask) {
  Nan::HandleScope scope;

  GLuint mask = info[0]->Uint32Value();

  glStencilMask(mask);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilMaskSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value();
  GLuint mask = info[1]->Uint32Value();

  glStencilMaskSeparate(face, mask);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilOp) {
  Nan::HandleScope scope;

  GLenum fail = info[0]->Int32Value();
  GLenum zfail = info[1]->Int32Value();
  GLenum zpass = info[2]->Int32Value();

  glStencilOp(fail, zfail, zpass);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilOpSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value();
  GLenum fail = info[1]->Int32Value();
  GLenum zfail = info[2]->Int32Value();
  GLenum zpass = info[3]->Int32Value();

  glStencilOpSeparate(face, fail, zfail, zpass);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BindRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLuint buffer = info[1]->IsNull() ? 0 : info[1]->Int32Value();

  glBindRenderbuffer(target, buffer);
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffers;
  glGenRenderbuffers(1,&renderbuffers);
  CheckGLError();
  #ifdef LOGGING
  cout<<"createRenderBuffer "<<renderbuffers<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_RENDERBUFFER, renderbuffers);
  CheckGLError();
  info.GetReturnValue().Set(Nan::New<Number>(renderbuffers));
}

NAN_METHOD(WebGLRenderingContext::DeleteBuffer) {
  Nan::HandleScope scope;

  GLuint buffer = info[0]->Uint32Value();

  glDeleteBuffers(1,&buffer);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer = info[0]->Uint32Value();

  glDeleteFramebuffers(1,&buffer);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteProgram) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Uint32Value();

  glDeleteProgram(program);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffer = info[0]->Uint32Value();

  glDeleteRenderbuffers(1, &renderbuffer);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteShader) {
  Nan::HandleScope scope;

  GLuint shader = info[0]->Uint32Value();

  glDeleteShader(shader);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteTexture) {
  Nan::HandleScope scope;

  GLuint texture = info[0]->Uint32Value();

  glDeleteTextures(1,&texture);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DetachShader) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Uint32Value();
  GLuint shader = info[1]->Uint32Value();

  glDetachShader(program, shader);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::FramebufferRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum attachment = info[1]->Int32Value();
  GLenum renderbuffertarget = info[2]->Int32Value();
  GLuint renderbuffer = info[3]->Uint32Value();

  glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetVertexAttribOffset) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Uint32Value();
  GLenum pname = info[1]->Int32Value();
  void *ret=NULL;

  glGetVertexAttribPointerv(index, pname, &ret);
  CheckGLError();
  info.GetReturnValue().Set(JS_INT(ToGLuint(ret)));
}

NAN_METHOD(WebGLRenderingContext::IsBuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(Nan::New<Boolean>(glIsBuffer(info[0]->Uint32Value())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsFramebuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsFramebuffer(info[0]->Uint32Value())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsProgram) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsProgram(info[0]->Uint32Value())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsRenderbuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsRenderbuffer( info[0]->Uint32Value())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsShader) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsShader(info[0]->Uint32Value())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsTexture) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsTexture(info[0]->Uint32Value())!=0));
}

NAN_METHOD(WebGLRenderingContext::RenderbufferStorage) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum internalformat = info[1]->Int32Value();
  GLsizei width = info[2]->Uint32Value();
  GLsizei height = info[3]->Uint32Value();

  glRenderbufferStorage(target, internalformat, width, height);
  CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetShaderSource) {
  Nan::HandleScope scope;

  int shader = info[0]->Int32Value();

  GLint len;
  glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &len);
  CheckGLError();
  GLchar *source=new GLchar[len];
  glGetShaderSource(shader, len, NULL, source);
CheckGLError();
  Local<String> str = JS_STR(source);
  delete[] source;

  info.GetReturnValue().Set(str);
}

NAN_METHOD(WebGLRenderingContext::ValidateProgram) {
  Nan::HandleScope scope;

  glValidateProgram(info[0]->Int32Value());
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::TexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLint level = info[1]->Int32Value();
  GLint xoffset = info[2]->Int32Value();
  GLint yoffset = info[3]->Int32Value();
  GLsizei width = info[4]->Int32Value();
  GLsizei height = info[5]->Int32Value();
  GLenum format = info[6]->Int32Value();
  GLenum type = info[7]->Int32Value();
  void *pixels=getImageData(info[8]);

  if (pixels) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->preprocessTexImageData(pixels, width, height, format, type);
  }

  glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ReadPixels) {
  Nan::HandleScope scope;

  GLint x = info[0]->Int32Value();
  GLint y = info[1]->Int32Value();
  GLsizei width = info[2]->Int32Value();
  GLsizei height = info[3]->Int32Value();
  GLenum format = info[4]->Int32Value();
  GLenum type = info[5]->Int32Value();
  void *pixels=getImageData(info[6]);

  glReadPixels(x, y, width, height, format, type, pixels);
CheckGLError();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetTexParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum pname = info[1]->Int32Value();

  GLint param_value=0;
  glGetTexParameteriv(target, pname, &param_value);
CheckGLError();
  info.GetReturnValue().Set(Nan::New<Number>(param_value));
}

NAN_METHOD(WebGLRenderingContext::GetActiveAttrib) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  GLuint index = info[1]->Int32Value();

  char name[1024];
  GLsizei length=0;
  GLenum type;
  GLsizei size;
  glGetActiveAttrib(program, index, 1024, &length, &size, &type, name);
CheckGLError();
  Local<Array> activeInfo = Nan::New<Array>(3);
  activeInfo->Set(JS_STR("size"), JS_INT(size));
  activeInfo->Set(JS_STR("type"), JS_INT((int)type));
  activeInfo->Set(JS_STR("name"), JS_STR(name));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(WebGLRenderingContext::GetActiveUniform) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  GLuint index = info[1]->Int32Value();

  char name[1024];
  GLsizei length=0;
  GLenum type;
  GLsizei size;
  glGetActiveUniform(program, index, 1024, &length, &size, &type, name);
CheckGLError();
  Local<Array> activeInfo = Nan::New<Array>(3);
  activeInfo->Set(JS_STR("size"), JS_INT(size));
  activeInfo->Set(JS_STR("type"), JS_INT((int)type));
  activeInfo->Set(JS_STR("name"), JS_STR(name));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(WebGLRenderingContext::GetAttachedShaders) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();

  GLuint shaders[1024];
  GLsizei count;
  glGetAttachedShaders(program, 1024, &count, shaders);
CheckGLError();
  Local<Array> shadersArr = Nan::New<Array>(count);
  for(int i=0;i<count;i++)
    shadersArr->Set(i, JS_INT((int)shaders[i]));

  info.GetReturnValue().Set(shadersArr);
}

NAN_METHOD(WebGLRenderingContext::GetParameter) {
  Nan::HandleScope scope;

  GLenum name = info[0]->Int32Value();

  switch(name) {
  case GL_BLEND:
  case GL_CULL_FACE:
  case GL_DEPTH_TEST:
  case GL_DEPTH_WRITEMASK:
  case GL_DITHER:
  case GL_POLYGON_OFFSET_FILL:
  case GL_SAMPLE_COVERAGE_INVERT:
  case GL_SCISSOR_TEST:
  case GL_STENCIL_TEST:
  case 0x9240 /* UNPACK_FLIP_Y_WEBGL */:
  {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    info.GetReturnValue().Set(JS_BOOL(obj->pixelStorei_UNPACK_FLIP_Y_WEBGL!=0));
    break;
  }
  case 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL*/:
  {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    info.GetReturnValue().Set(JS_BOOL(obj->pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL!=0));
    break;
  }
  case 0x9245 /* BGRA*/:
  {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    info.GetReturnValue().Set(JS_BOOL(obj->pixelStorei_UNPACK_FLIP_BLUE_RED!=0));
    break;
  }
  case GL_DEPTH_CLEAR_VALUE:
  case GL_LINE_WIDTH:
  case GL_POLYGON_OFFSET_FACTOR:
  case GL_POLYGON_OFFSET_UNITS:
  case GL_SAMPLE_COVERAGE_VALUE:
  {
    // return a float
    GLfloat params;
    ::glGetFloatv(name, &params);
    info.GetReturnValue().Set(JS_FLOAT(params));
    break;
  }
  case GL_RENDERER:
  case GL_SHADING_LANGUAGE_VERSION:
  case GL_VENDOR:
  case GL_VERSION:
  case GL_EXTENSIONS:
  {
    // return a string
    char *params=(char*) ::glGetString(name);

    if(params!=NULL){
      info.GetReturnValue().Set(JS_STR(params));
    }else{
      info.GetReturnValue().Set(Nan::Undefined());
    }

    break;
  }
  case GL_MAX_VIEWPORT_DIMS:
  {
    // return a int32[2]
    GLint params[2];
    ::glGetIntegerv(name, params);

    Local<Array> arr=Nan::New<Array>(2);
    arr->Set(0,JS_INT(params[0]));
    arr->Set(1,JS_INT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_SCISSOR_BOX:
  case GL_VIEWPORT:
  {
    // return a int32[4]
    GLint params[4];
    ::glGetIntegerv(name, params);

    Local<Array> arr=Nan::New<Array>(4);
    arr->Set(0,JS_INT(params[0]));
    arr->Set(1,JS_INT(params[1]));
    arr->Set(2,JS_INT(params[2]));
    arr->Set(3,JS_INT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_ALIASED_LINE_WIDTH_RANGE:
  case GL_ALIASED_POINT_SIZE_RANGE:
  case GL_DEPTH_RANGE:
  {
    // return a float[2]
    GLfloat params[2];
    ::glGetFloatv(name, params);
    Local<Array> arr=Nan::New<Array>(2);
    arr->Set(0,JS_FLOAT(params[0]));
    arr->Set(1,JS_FLOAT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_BLEND_COLOR:
  case GL_COLOR_CLEAR_VALUE:
  {
    // return a float[4]
    GLfloat params[4];
    ::glGetFloatv(name, params);
    Local<Array> arr=Nan::New<Array>(4);
    arr->Set(0,JS_FLOAT(params[0]));
    arr->Set(1,JS_FLOAT(params[1]));
    arr->Set(2,JS_FLOAT(params[2]));
    arr->Set(3,JS_FLOAT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_COLOR_WRITEMASK:
  {
    // return a boolean[4]
    GLboolean params[4];
    ::glGetBooleanv(name, params);
    Local<Array> arr=Nan::New<Array>(4);
    arr->Set(0,JS_BOOL(params[0]==1));
    arr->Set(1,JS_BOOL(params[1]==1));
    arr->Set(2,JS_BOOL(params[2]==1));
    arr->Set(3,JS_BOOL(params[3]==1));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_ARRAY_BUFFER_BINDING:
  case GL_CURRENT_PROGRAM:
  case GL_ELEMENT_ARRAY_BUFFER_BINDING:
  case GL_FRAMEBUFFER_BINDING:
  case GL_RENDERBUFFER_BINDING:
  case GL_TEXTURE_BINDING_2D:
  case GL_TEXTURE_BINDING_CUBE_MAP:
  {
    GLint params;
    ::glGetIntegerv(name, &params);
    info.GetReturnValue().Set(JS_INT(params));
    break;
  }
  default: {
    // return a long
    GLint params;
    ::glGetIntegerv(name, &params);
    info.GetReturnValue().Set(JS_INT(params));
  }
  }

  CheckGLError();
  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetBufferParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum pname = info[1]->Int32Value();

  GLint params;
  glGetBufferParameteriv(target,pname,&params);
  CheckGLError();
  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(WebGLRenderingContext::GetFramebufferAttachmentParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value();
  GLenum attachment = info[1]->Int32Value();
  GLenum pname = info[2]->Int32Value();

  GLint params;
  glGetFramebufferAttachmentParameteriv(target,attachment, pname,&params);
  CheckGLError();
  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(WebGLRenderingContext::GetProgramInfoLog) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value();
  int Len = 1024;
  char Error[1024];
  glGetProgramInfoLog(program, 1024, &Len, Error);
CheckGLError();
  info.GetReturnValue().Set(JS_STR(Error));
}

NAN_METHOD(WebGLRenderingContext::GetRenderbufferParameter) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int value = 0;
  glGetRenderbufferParameteriv(target,pname,&value);
CheckGLError();
  info.GetReturnValue().Set(JS_INT(value));
}

NAN_METHOD(WebGLRenderingContext::GetVertexAttrib) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Int32Value();
  GLuint pname = info[1]->Int32Value();

  GLint value=0;

  switch (pname) {
  case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
  case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_BOOL(value!=0));
    break;
  case GL_VERTEX_ATTRIB_ARRAY_SIZE:
  case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
  case GL_VERTEX_ATTRIB_ARRAY_TYPE:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_INT(value));
    break;
  case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_INT(value));
    break;
  case GL_CURRENT_VERTEX_ATTRIB: {
    float vextex_attribs[4];
    glGetVertexAttribfv(index,pname,vextex_attribs);
    Local<Array> arr=Nan::New<Array>(4);
    arr->Set(0,JS_FLOAT(vextex_attribs[0]));
    arr->Set(1,JS_FLOAT(vextex_attribs[1]));
    arr->Set(2,JS_FLOAT(vextex_attribs[2]));
    arr->Set(3,JS_FLOAT(vextex_attribs[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  default:
    Nan::ThrowError("GetVertexAttrib: Invalid Enum");
  }
CheckGLError();
  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetSupportedExtensions) {
  Nan::HandleScope scope;

  char *extensions=(char*) glGetString(GL_EXTENSIONS);
CheckGLError();
  info.GetReturnValue().Set(JS_STR(extensions));
}

// TODO GetExtension(name) return the extension name if found, should be an object...
NAN_METHOD(WebGLRenderingContext::GetExtension) {
  Nan::HandleScope scope;

  String::Utf8Value name(info[0]);
  char *sname=*name;
  char *extensions=(char*) glGetString(GL_EXTENSIONS);
  CheckGLError();
  char *ext=strcasestr(extensions, sname);

  if(ext==NULL){
      info.GetReturnValue().Set(Nan::Undefined());
  }else{
     info.GetReturnValue().Set(JS_STR(ext, (int)::strlen(sname)));
  }
}

NAN_METHOD(WebGLRenderingContext::CheckFramebufferStatus) {
  Nan::HandleScope scope;

  GLenum target=info[0]->Int32Value();

  int status = (int)glCheckFramebufferStatus(target);
  CheckGLError();

  info.GetReturnValue().Set(JS_INT(status));
}

void WebGLRenderingContext::preprocessTexImageData(void * pixels, int width, int height, int format, int type) {
  if (pixelStorei_UNPACK_FLIP_BLUE_RED) {
    if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
      Nan::ThrowError("UNPACK_FLIP_BLUE_RED is only implemented for format RGBA and type UNSIGNED_BYTE");
    }
    int total = width * height * 4;
    unsigned char * data = (unsigned char *) pixels;
    for (int o = 0; o < total; o += 4) {
      unsigned char red = data[o];
      data[o] = data[o+2];
      data[o+2] = red;
    }
  }

  if (pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL) {
    if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
      Nan::ThrowError("UNPACK_PREMULTIPLY_ALPHA_WEBGL is only implemented for format RGBA and type UNSIGNED_BYTE");
    }
    int total = width * height * 4;
    unsigned char * data = (unsigned char *) pixels;
    for (int o = 0; o < total; o += 4) {
      unsigned char alpha = data[o + 3];
      data[o] = (data[o] * alpha) >> 8;
      data[o+1] = (data[o+1] * alpha) >> 8;
      data[o+2] = (data[o+2] * alpha) >> 8;
    }
  }

  if (pixelStorei_UNPACK_FLIP_Y_WEBGL) {
    Nan::ThrowError("UNPACK_FLIP_Y_WEBGL is not implemented");
  }
}

struct GLObj {
  GLObjectType type;
  GLuint obj;
  GLObj(GLObjectType type, GLuint obj) {
    this->type=type;
    this->obj=obj;
  }
};

vector<GLObj*> globjs;
static bool atExit=false;

void registerGLObj(GLObjectType type, GLuint obj) {
  globjs.push_back(new GLObj(type,obj));
}


void unregisterGLObj(GLuint obj) {
  if(atExit) return;

  vector<GLObj*>::iterator it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    if(globj->obj==obj) {
      delete globj;
      globjs.erase(it);
      break;
    }
    ++it;
  }
}

void WebGLRenderingContext::AtExit() {
  atExit=true;
  //glFinish();

  vector<GLObj*>::iterator it;

  #ifdef LOGGING
  cout<<"WebGL AtExit() called"<<endl;
  cout<<"  # objects allocated: "<<globjs.size()<<endl;
  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *obj=*it;
    cout<<"[";
    switch(obj->type) {
    case GLOBJECT_TYPE_BUFFER: cout<<"buffer"; break;
    case GLOBJECT_TYPE_FRAMEBUFFER: cout<<"framebuffer"; break;
    case GLOBJECT_TYPE_PROGRAM: cout<<"program"; break;
    case GLOBJECT_TYPE_RENDERBUFFER: cout<<"renderbuffer"; break;
    case GLOBJECT_TYPE_SHADER: cout<<"shader"; break;
    case GLOBJECT_TYPE_TEXTURE: cout<<"texture"; break;
    };
    cout<<": "<<obj->obj<<"] ";
    ++it;
  }
  cout<<endl;
  #endif

  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    GLuint obj=globj->obj;

    switch(globj->type) {
    case GLOBJECT_TYPE_PROGRAM:
      #ifdef LOGGING
      cout<<"  Destroying GL program "<<obj<<endl;
      #endif
      glDeleteProgram(obj);
      break;
    case GLOBJECT_TYPE_BUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL buffer "<<obj<<endl;
      #endif
      glDeleteBuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_FRAMEBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL frame buffer "<<obj<<endl;
      #endif
      glDeleteFramebuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_RENDERBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL render buffer "<<obj<<endl;
      #endif
      glDeleteRenderbuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_SHADER:
      #ifdef LOGGING
      cout<<"  Destroying GL shader "<<obj<<endl;
      #endif
      glDeleteShader(obj);
      break;
    case GLOBJECT_TYPE_TEXTURE:
      #ifdef LOGGING
      cout<<"  Destroying GL texture "<<obj<<endl;
      #endif
      glDeleteTextures(1,&obj);
      break;
    default:
      #ifdef LOGGING
      cout<<"  Unknown object "<<obj<<endl;
      #endif
      break;
    }
    delete globj;
    ++it;
  }

  globjs.clear();
}

} // end namespace webgl
