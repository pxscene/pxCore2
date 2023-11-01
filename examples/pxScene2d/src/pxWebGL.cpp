/*

  pxCore Copyright 2005-2018 John Robinson

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

 */

#include "pxWebGL.h"
#include "pxSharedContext.h"
#include "pxContext.h"

#include <algorithm>

extern int currentGLProgram;

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
        if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
        {
          rtLogDebug("GL %s - %s : %d", error.c_str(), file, line);
        }
        else
        {
          rtLogError("GL %s - %s : %d", error.c_str(), file, line);
        }
        err = glGetError();
    }

    return;
}

pxWebgl::pxWebgl(pxScene2d* scene):pxObject(scene), pixelStorei_UNPACK_FLIP_Y_WEBGL(0), pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL(0),
          pixelStorei_UNPACK_FLIP_BLUE_RED(0), mInitialFrameBuffer (0), mShaders(), mFramebuffers(),
          mTextures(), mBuffers(), mPrograms(), mWebGlContext()
{
}

pxWebgl::~pxWebgl()
{
  rtLogInfo("%s : %lu Framebuffers, %lu Textures, %lu Buffers, %lu Shaders, %lu Programs",
    __FUNCTION__,
    mFramebuffers.size(),
    mTextures.size(),
    mBuffers.size(),
    mShaders.size(),
    mPrograms.size());

  glDeleteFramebuffers( (GLsizei) mFramebuffers.size(), mFramebuffers.data());
  CheckGLError();
  mFramebuffers.clear();

  glDeleteTextures((GLsizei) mTextures.size(), mTextures.data());
  CheckGLError();
  mTextures.clear();

  glDeleteBuffers((GLsizei) mBuffers.size(), mBuffers.data());
  CheckGLError();
  mBuffers.clear();

  for(auto i = mShaders.begin(); i != mShaders.end();)
  {
    glDeleteShader(*i);
    CheckGLError();
    i = mShaders.erase(i);
  }

  for(auto i = mPrograms.begin(); i != mPrograms.end();)
  {
    glDeleteProgram(*i);
    CheckGLError();
    i = mPrograms.erase(i);
  }
}

void pxWebgl::onInit()
{
  mReady.send("resolve",this);
  pxObject::onInit();

  rtLogDebug("[%s]", __FUNCTION__);

  pixelStorei_UNPACK_FLIP_Y_WEBGL = 0;
  pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0;
  pixelStorei_UNPACK_FLIP_BLUE_RED = 0;

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mInitialFrameBuffer);
  CheckGLError();
}

rtError pxWebgl::DrawElements(uint32_t mode, uint32_t count, uint32_t type, uint32_t offset)
{
  rtLogDebug("[%s] mode: %u, count: %u type: %u offset: %u", __FUNCTION__, mode, count, type, offset);

  GLvoid *gloffset = reinterpret_cast<GLvoid*>(offset);

  glDrawElements(mode, count, type, gloffset);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::createTexture(uint32_t& texture)
{
  rtLogDebug("[%s]",__FUNCTION__);

  glGenTextures(1, &texture);
  CheckGLError();

  if (texture)
  {
    if (std::find(mTextures.begin(), mTextures.end(), texture) == mTextures.end())
      mTextures.push_back(texture);
    else
      rtLogWarn("[%s] %u exists",__FUNCTION__, texture);
  }

  rtLogDebug("[%s] returning texture: %u",__FUNCTION__, texture);

  return RT_OK;
}

rtError pxWebgl::createBuffer(uint32_t& buffer)
{
  rtLogDebug("[%s]", __FUNCTION__);

  glGenBuffers(1, &buffer);
  CheckGLError();

  if (buffer)
  {
    if (std::find(mBuffers.begin(), mBuffers.end(), buffer) == mBuffers.end())
      mBuffers.push_back(buffer);
    else
      rtLogWarn("[%s] %u exists",__FUNCTION__, buffer);
  }

  rtLogDebug("[%s] returning buffer: %u", __FUNCTION__, buffer);

  return RT_OK;
}

rtError pxWebgl::bindTexture(uint32_t target, uint32_t texture)
{
  rtLogDebug("[%s] target: %u, texture: %u", __FUNCTION__, target, texture);

  mWebGlContext.textures[target] = texture;

  glBindTexture(target, texture);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::bindBuffer(uint32_t target, uint32_t buffer)
{
  rtLogDebug("[%s] target: %u buffer: %u", __FUNCTION__, target, buffer);

  mWebGlContext.buffers[target] = buffer;

  glBindBuffer(target,buffer);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::bindFramebuffer(uint32_t target, uint32_t framebuffer)
{
  rtLogDebug("[%s] target: %u framebuffer: %u", __FUNCTION__, target, framebuffer);

  glBindFramebuffer(target, (framebuffer==0) ? mInitialFrameBuffer : framebuffer);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::bufferData(uint32_t target, rtValue data, uint32_t usage)
{
  rtLogDebug("[%s] target: %u usage: %u", __FUNCTION__, target, usage);

  rtArrayObject* dataArray = (rtArrayObject*) data.toObject().getPtr();

  rtValue length;
  dataArray->Get("length", &length);
  rtLogDebug("[%s] length: %d", __FUNCTION__, length.toUInt32());

  uint32_t dataBufSize = length.toUInt32() * ((target == GL_ELEMENT_ARRAY_BUFFER) ? sizeof(GLushort) : sizeof(GLfloat));

  rtValue dataValue;
  dataArray->Get("arrayData", &dataValue);
  void* dataPtr = NULL;
  dataValue.getVoidPtr(dataPtr);

  rtLogDebug("[%s] size: %u", __FUNCTION__, dataBufSize);

  glBufferData(target, dataBufSize, dataPtr, usage);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::pixelStorei(uint32_t pname, bool param)
{
  rtLogDebug("[%s] pname: %u param: %u", __FUNCTION__, pname, param);

  if (pname == 0x9240 /* UNPACK_FLIP_Y_WEBGL */) {
    pixelStorei_UNPACK_FLIP_Y_WEBGL = param;
  } else if (pname == 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL */) {
    pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL = param;
  } else if (pname == 0x9245 /* UNPACK_FLIP_BLUE_RED */) {
    pixelStorei_UNPACK_FLIP_BLUE_RED = param;
  }

  glPixelStorei(pname,param);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::texParameteri(uint32_t target, uint32_t pname, uint32_t param)
{
  rtLogDebug("[%s] target: %u pname: %u param: %u", __FUNCTION__, target, pname, param);

  glTexParameteri(target, pname, param);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
  rtLogDebug("[%s] x: %u, y: %u, width: %u, height: %u", __FUNCTION__, x, y, width, height );

  glViewport(x, y, width, height);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::clearColor(float_t red, float_t green, float_t blue, float_t alpha)
{
  rtLogDebug("[%s] red: %3.2f, green: %3.2f, blue: %3.2f, alpha: %3.2f", __FUNCTION__, red, green, blue, alpha);

  glClearColor(red, green, blue, alpha);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::Clear(uint32_t mask)
{
  rtLogDebug("[%s] mask: %u", __FUNCTION__, mask);

  glClear(mask);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::BlendFunc(uint32_t sfactor, uint32_t dfactor)
{
  rtLogDebug("[%s] sfactor: %d dfactor: %d", __FUNCTION__, sfactor, dfactor);

  glBlendFunc(sfactor,dfactor);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::texImage2D(uint32_t target, uint32_t level, uint32_t internalformat, uint32_t width, uint32_t height, uint32_t border, uint32_t format, uint32_t type, rtValue data)
{
  rtLogDebug("[%s] target: %d level: %d internalformat: %d width: %d height %d format %d", __FUNCTION__, target, level, internalformat, width, height, format);

  rtArrayObject* pixelArray = (rtArrayObject*) data.toObject().getPtr();

  void *pixels = NULL;

  if(pixelArray) {
    rtValue length;
    pixelArray->Get("length", &length);

    rtLogDebug("[%s] length is %u", __FUNCTION__, length.toUInt32());

    rtValue dataValue;
    pixelArray->Get("arrayData", &dataValue);
    dataValue.getVoidPtr(pixels);

    preprocessTexImageData(pixels, width, height, format, type);
  }

  glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
  CheckGLError();

  return RT_OK;
}

void pxWebgl::preprocessTexImageData(void * pixels, int width, int height, int format, int type) {

  if (pixelStorei_UNPACK_FLIP_BLUE_RED) {
    if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
      rtLogWarn("[%s] UNPACK_FLIP_BLUE_RED is only implemented for format RGBA and type UNSIGNED_BYTE", __FUNCTION__);
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
      rtLogWarn("[%s] UNPACK_PREMULTIPLY_ALPHA_WEBGL is only implemented for format RGBA and type UNSIGNED_BYTE", __FUNCTION__);
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
    rtLogWarn("[%s] UNPACK_FLIP_Y_WEBGL is not implemented", __FUNCTION__);
  }
}

rtError pxWebgl::Enable(uint32_t cap)
{
  rtLogDebug("[%s] cap: %u", __FUNCTION__, cap);

  std::vector<uint32_t>::iterator it;
  it = find (mWebGlContext.webGlStates.begin(), mWebGlContext.webGlStates.end(), cap);
  if (it == mWebGlContext.webGlStates.end())
  {
    mWebGlContext.webGlStates.push_back(cap);
  }

  glEnable(cap);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::Disable(uint32_t cap)
{
  rtLogDebug("[%s] cap: %u", __FUNCTION__, cap);

  std::vector<uint32_t>::iterator it;
  it = find (mWebGlContext.webGlStates.begin(), mWebGlContext.webGlStates.end(), cap);
  if (it != mWebGlContext.webGlStates.end())
  {
    mWebGlContext.webGlStates.erase(it);
  }

  glDisable(cap);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::Flush()
{
  rtLogDebug("Flush called");
  glFlush();
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::CreateProgram(uint32_t& glprogram)
{
  rtLogDebug("[%s]",__FUNCTION__);

  glprogram=glCreateProgram();
  CheckGLError();

  if (glprogram)
  {
    if (std::find(mPrograms.begin(), mPrograms.end(), glprogram) == mPrograms.end())
      mPrograms.push_back(glprogram);
    else
      rtLogWarn("[%s] %u exists",__FUNCTION__, glprogram);
  }

  rtLogDebug("[%s] returning program: %u", __FUNCTION__, glprogram);

  return RT_OK;
}
rtError pxWebgl::CreateShader(uint32_t type, uint32_t& glshader)
{
  rtLogDebug("[%s] type: %u", __FUNCTION__, type);

  glshader=glCreateShader(type);
  CheckGLError();

  if (glshader)
  {
    if (std::find(mShaders.begin(), mShaders.end(), glshader) == mShaders.end())
      mShaders.push_back(glshader);
    else
      rtLogWarn("[%s] %u exists",__FUNCTION__, glshader);
  }

  rtLogDebug("[%s] returning shader: %u", __FUNCTION__, glshader);

  return RT_OK;
}
rtError pxWebgl::ShaderSource(uint32_t shader, rtString source)
{
  rtLogDebug("[%s] shader: %u, length: %u, source:\n\n%s\n", __FUNCTION__, shader, source.byteLength(), source.cString());

  const char* codes[1];
  codes[0] = source.cString();
  GLint length = source.byteLength();

  glShaderSource (shader, 1, codes, &length);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::CompileShader(uint32_t shader)
{
  rtLogDebug("[%s] shader: %u", __FUNCTION__, shader);

  glCompileShader(shader);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::GetShaderParameter(uint32_t shader, uint32_t pname, uint32_t& ret)
{
  rtLogDebug("[%s] shader: %u pname: %u", __FUNCTION__, shader, pname);

  int value=100;

  switch (pname) {
    case GL_DELETE_STATUS:
    case GL_COMPILE_STATUS:
      glGetShaderiv(shader, pname, &value);

      ret = static_cast</*unsigned long*/ uint32_t>(value!=0);

      break;
    case GL_SHADER_TYPE:
      glGetShaderiv(shader, pname, &value);
      ret = static_cast</*unsigned long*/ uint32_t>(value);

      break;
    case GL_INFO_LOG_LENGTH:
    case GL_SHADER_SOURCE_LENGTH:
      glGetShaderiv(shader, pname, &value);
      ret = static_cast</*unsigned long*/ uint32_t>(value);

      break;
    default:
      rtLogWarn("[%s] Invalid Enum", __FUNCTION__);
  }

  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::GetShaderInfoLog(uint32_t shader, rtString errorlog)
{
  rtLogDebug("[%s] shader: %u", __FUNCTION__, shader);

  int Len = 1024;
  char Error[1024];

  glGetShaderInfoLog(shader, 1024, &Len, Error);
  CheckGLError();

  errorlog.init(Error, Len);

  rtLogDebug("[%s] error string: %s length: %u", __FUNCTION__, Error, Len);

  return RT_OK;
}

rtError pxWebgl::AttachShader(uint32_t program, uint32_t shader)
{
  rtLogDebug("[%s] program %u, shader: %u", __FUNCTION__, program, shader);

  glAttachShader(program, shader);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::LinkProgram(uint32_t program)
{
  rtLogDebug("[%s] program %u", __FUNCTION__, program);

  glLinkProgram(program);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::GetProgramParameter(uint32_t program, uint32_t pname, uint32_t& ret)
{
  rtLogDebug("[%s] program: %d pname: %u", __FUNCTION__, program, pname);

  int value = 0;
  switch (pname) {
    case GL_DELETE_STATUS:
    case GL_LINK_STATUS:
    case GL_VALIDATE_STATUS:
      glGetProgramiv(program, pname, &value);
      ret = static_cast</*unsigned long*/ uint32_t>(value!=0);
      break;
    case GL_ATTACHED_SHADERS:
    case GL_ACTIVE_ATTRIBUTES:
    case GL_ACTIVE_UNIFORMS:
      glGetProgramiv(program, pname, &value);
      ret = (static_cast</*unsigned long*/ uint32_t>(value));
      break;
    default:
      rtLogWarn("[%s] Invalid Enum", __FUNCTION__);
  }

  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::DeleteShader(uint32_t shader)
{
  rtLogDebug("[%s] shader %u", __FUNCTION__, shader);

  glDeleteShader(shader);
  CheckGLError();

  auto i = std::find(mShaders.begin(), mShaders.end(), shader);
  if (i != mShaders.end())
    mShaders.erase(i);
  else
    rtLogWarn("[%s] %u doesn't exist",__FUNCTION__, shader);

  return RT_OK;
}

rtError pxWebgl::UseProgram(uint32_t program)
{
  rtLogDebug("[%s] program %u", __FUNCTION__, program);

  glUseProgram(program);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::GetAttribLocation(uint32_t program, rtString name, uint32_t& ret)
{
  rtLogDebug("[%s] program %u, name: %s", __FUNCTION__, program, name.cString());

  ret = glGetAttribLocation(program, name.cString());
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::VertexAttribPointer(uint32_t indx, uint32_t size, uint32_t type, uint32_t normalized, uint32_t stride, uint32_t offset)
{
  rtLogDebug("[%s] index: %u, size: %u, type: %u, normalized: %u, stride: %u, offset: %u", __FUNCTION__, indx, size, type, normalized, stride, offset);

  GLvoid *gloffset = reinterpret_cast<GLvoid*>(offset);

  attributeData data{size, type, normalized, stride, gloffset};
  mWebGlContext.attributesData[indx] = data;

  glVertexAttribPointer(indx, size, type, normalized, stride, gloffset);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::EnableVertexAttribArray(uint32_t index)
{
  rtLogDebug("[%s] index %u", __FUNCTION__, index);

  std::vector<uint32_t>::iterator it;
  it = find (mWebGlContext.attributes.begin(), mWebGlContext.attributes.end(), index);
  if (it == mWebGlContext.attributes.end())
  {
    mWebGlContext.attributes.push_back(index);
  }

  glEnableVertexAttribArray(index);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::GetUniformLocation(uint32_t program, rtString name, uint32_t& ret)
{
  rtLogDebug("[%s] program %u, name: %s", __FUNCTION__, program, name.cString());

  ret = glGetUniformLocation(program, name.cString());
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::UniformMatrix4fv(uint32_t location, bool transpose, rtValue data)
{
  rtLogDebug("[%s] location: %u transpose: %u", __FUNCTION__, location, transpose);

  rtArrayObject* dataArray = (rtArrayObject*) data.toObject().getPtr();

  rtValue length;
  dataArray->Get("length", &length);

  uint32_t dataBufSize = length.toUInt32();

  rtValue dataValue;
  dataArray->Get("arrayData", &dataValue);
  void* dataPtr = NULL;
  dataValue.getVoidPtr(dataPtr);

  glUniformMatrix4fv(location, dataBufSize / 16, transpose, (GLfloat*)dataPtr);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::DrawArrays(uint32_t mode, uint32_t first, uint32_t count)
{
  rtLogDebug("[%s] mode %u, first: %u, count: %u", __FUNCTION__, mode, first, count);

  glDrawArrays(mode, first, count);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::Uniform2fv(uint32_t location, rtValue data)
{
  rtLogDebug("[%s] location: %u", __FUNCTION__, location);

  rtArrayObject* dataArray = (rtArrayObject*) data.toObject().getPtr();

  rtValue length;
  dataArray->Get("length", &length);

  uint32_t dataBufSize = length.toUInt32();
  rtValue dataValue;
  dataArray->Get("arrayData", &dataValue);
  void* dataPtr = NULL;
  dataValue.getVoidPtr(dataPtr);

  glUniform2fv(location, dataBufSize/2, (GLfloat *)dataPtr);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::Scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
  rtLogDebug("[%s] x: %u, y: %u, width: %u, height: %u", __FUNCTION__, x, y, width, height);

  glScissor(x, y, width, height);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::DisableVertexAttribArray(uint32_t index)
{
  rtLogDebug("[%s] index: %u", __FUNCTION__, index);

  std::vector<uint32_t>::iterator it;
  it = find (mWebGlContext.attributes.begin(), mWebGlContext.attributes.end(), index);
  if (it != mWebGlContext.attributes.end())
  {
    mWebGlContext.attributes.erase(it);
  }

  glDisableVertexAttribArray(index);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::CreateFramebuffer(uint32_t& buffer)
{
  rtLogDebug("[%s]", __FUNCTION__);

  glGenFramebuffers(1, &buffer);
  CheckGLError();

  if (buffer)
  {
    if (std::find(mFramebuffers.begin(), mFramebuffers.end(), buffer) == mFramebuffers.end())
      mFramebuffers.push_back(buffer);
    else
      rtLogWarn("[%s] %u exists",__FUNCTION__, buffer);
  }

  rtLogDebug("[%s] returning buffer: %u",__FUNCTION__, buffer);

  return RT_OK;
}

rtError pxWebgl::FramebufferTexture2D(uint32_t target, uint32_t attachment, uint32_t textarget, uint32_t texture, uint32_t level)
{
  rtLogDebug("[%s] target: %u, attachment: %u, textarget: %u, texture: %u, level: %u", __FUNCTION__, target, attachment, textarget, texture, level);

  glFramebufferTexture2D(target, attachment, textarget, texture, level);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::Uniform1f(uint32_t location, float x)
{
  rtLogDebug("[%s] location: %u x: %f", __FUNCTION__, location, x);

  glUniform1f(location, x);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::Uniform1i(uint32_t location, uint32_t x)
{
  rtLogDebug("[%s] location: %u x: %u", __FUNCTION__, location, x);

  glUniform1i(location, x);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::ActiveTexture(uint32_t texture)
{
  rtLogDebug("[%s] texture: %u", __FUNCTION__, texture);

  glActiveTexture(texture);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::GenerateMipmap(uint32_t target)
{
  rtLogDebug("[%s] target: %u", __FUNCTION__, target);

  glGenerateMipmap(target);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::UniformMatrix3fv(uint32_t location, bool transpose, rtValue data)
{
  rtLogDebug("[%s] location: %u transpose: %u", __FUNCTION__, location, transpose);

  rtArrayObject* dataArray = (rtArrayObject*) data.toObject().getPtr();

  rtValue length;
  dataArray->Get("length", &length);

  uint32_t dataBufSize = length.toUInt32();

  rtValue dataValue;
  dataArray->Get("arrayData", &dataValue);
  void* dataPtr = NULL;
  dataValue.getVoidPtr(dataPtr);

  glUniformMatrix3fv(location, dataBufSize / 16, transpose, (GLfloat*)dataPtr);
  CheckGLError();

  return RT_OK;
}

rtError pxWebgl::DeleteFramebuffer(uint32_t buffer)
{
  rtLogDebug("[%s] framebuffer %u", __FUNCTION__, buffer);

  glDeleteFramebuffers(1, &buffer);
  CheckGLError();

  auto i = std::find(mFramebuffers.begin(), mFramebuffers.end(), buffer);
  if (i != mFramebuffers.end())
    mFramebuffers.erase(i);
  else
    rtLogWarn("[%s] %u doesn't exist",__FUNCTION__, buffer);

  return RT_OK;
}

rtError pxWebgl::DeleteTexture(uint32_t texture)
{
  rtLogDebug("[%s] texture %u", __FUNCTION__, texture);

  glDeleteTextures(1, &texture);
  CheckGLError();

  auto i = std::find(mTextures.begin(), mTextures.end(), texture);
  if (i != mTextures.end())
    mTextures.erase(i);
  else
    rtLogWarn("[%s] %u doesn't exist",__FUNCTION__, texture);

  return RT_OK;
}

rtError pxWebgl::DeleteBuffer(uint32_t buffer)
{
  rtLogDebug("[%s] buffer %u", __FUNCTION__, buffer);

  glDeleteBuffers(1, &buffer);
  CheckGLError();

  auto i = std::find(mBuffers.begin(), mBuffers.end(), buffer);
  if (i != mBuffers.end())
    mBuffers.erase(i);
  else
    rtLogWarn("[%s] %u doesn't exist",__FUNCTION__, buffer);

  return RT_OK;
}

rtError pxWebgl::DeleteProgram(uint32_t program)
{
  rtLogDebug("[%s] program %u", __FUNCTION__, program);

  glDeleteProgram(program);
  CheckGLError();

  auto i = std::find(mPrograms.begin(), mPrograms.end(), program);
  if (i != mPrograms.end())
    mPrograms.erase(i);
  else
    rtLogWarn("[%s] %u doesn't exist",__FUNCTION__, program);

  return RT_OK;
}

rtError pxWebgl::beginNativeSparkRendering()
{
  for (std::vector<uint32_t>::iterator it = mWebGlContext.webGlStates.begin() ; it != mWebGlContext.webGlStates.end(); ++it)
  {
    glDisable(*it);
  }
  for (std::vector<uint32_t>::iterator it = mWebGlContext.attributes.begin() ; it != mWebGlContext.attributes.end(); ++it)
  {
    glDisableVertexAttribArray(*it);
  }
  for (std::map<uint32_t, uint32_t>::iterator it=mWebGlContext.textures.begin(); it!=mWebGlContext.textures.end(); ++it)
  {
    glBindTexture(it->first, 0);
  }
  for (std::map<uint32_t, uint32_t>::iterator it=mWebGlContext.buffers.begin(); it!=mWebGlContext.buffers.end(); ++it)
  {
    glBindBuffer(it->first, 0);
  }
  glGetIntegerv(GL_CURRENT_PROGRAM,&mWebGlContext.programId);

  glGetIntegerv(GL_SCISSOR_BOX, mWebGlContext.scissorBox);
  glGetFloatv(GL_COLOR_CLEAR_VALUE, mWebGlContext.clearColor);
  glGetIntegerv(GL_VIEWPORT, mWebGlContext.viewport);
  glGetIntegerv(GL_ACTIVE_TEXTURE, &mWebGlContext.activeTexture);

  glGetIntegerv(GL_BLEND_SRC_RGB, &mWebGlContext.blendSrcRgb);
  glGetIntegerv(GL_BLEND_DST_RGB, &mWebGlContext.blendDestRgb);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &mWebGlContext.blendSrcAlpha);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &mWebGlContext.blendDestAlpha);

  // set the states needed for pxContext rendering
  currentGLProgram = mWebGlContext.programId;
  glDisable( GL_SCISSOR_TEST );
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  return RT_OK;
}

rtError pxWebgl::endNativeSparkRendering()
{
  glActiveTexture(mWebGlContext.activeTexture);
  for (std::vector<uint32_t>::iterator it = mWebGlContext.webGlStates.begin() ; it != mWebGlContext.webGlStates.end(); ++it)
  {
    glEnable(*it);
  }
  for (std::map<uint32_t, uint32_t>::iterator it=mWebGlContext.textures.begin(); it!=mWebGlContext.textures.end(); ++it)
  {
    glBindTexture(it->first, it->second);
  }
  for (std::map<uint32_t, uint32_t>::iterator it=mWebGlContext.buffers.begin(); it!=mWebGlContext.buffers.end(); ++it)
  {
    glBindBuffer(it->first, it->second);
  }
  for (std::vector<uint32_t>::iterator it = mWebGlContext.attributes.begin() ; it != mWebGlContext.attributes.end(); ++it)
  {
    glEnableVertexAttribArray(*it);
    auto attributeDataItem = mWebGlContext.attributesData.find(*it);
    if ( attributeDataItem != mWebGlContext.attributesData.end())
    {
      glVertexAttribPointer(*it, attributeDataItem->second.size, attributeDataItem->second.type,
        attributeDataItem->second.normalized, attributeDataItem->second.stride, attributeDataItem->second.offset);
    }
  }
  glUseProgram(mWebGlContext.programId);
  glBlendFuncSeparate(mWebGlContext.blendSrcRgb, mWebGlContext.blendDestRgb, mWebGlContext.blendSrcAlpha, mWebGlContext.blendDestAlpha);
  glViewport (mWebGlContext.viewport[0], mWebGlContext.viewport[1], mWebGlContext.viewport[2], mWebGlContext.viewport[3]);
  glScissor(mWebGlContext.scissorBox[0], mWebGlContext.scissorBox[1], mWebGlContext.scissorBox[2], mWebGlContext.scissorBox[3]);
  glClearColor(mWebGlContext.clearColor[0], mWebGlContext.clearColor[1], mWebGlContext.clearColor[2], mWebGlContext.clearColor[3]);

  return RT_OK;
}

rtDefineObject(pxWebgl, pxObject);
rtDefineMethod(pxWebgl, DrawElements);
rtDefineMethod(pxWebgl, createTexture);
rtDefineMethod(pxWebgl, createBuffer);
rtDefineMethod(pxWebgl, bindTexture);
rtDefineMethod(pxWebgl, bindBuffer);
rtDefineMethod(pxWebgl, bindFramebuffer);
rtDefineMethod(pxWebgl, bufferData);
rtDefineMethod(pxWebgl, pixelStorei);
rtDefineMethod(pxWebgl, texParameteri);
rtDefineMethod(pxWebgl, texImage2D);
rtDefineMethod(pxWebgl, viewport);
rtDefineMethod(pxWebgl, clearColor);
rtDefineMethod(pxWebgl, Clear);
rtDefineMethod(pxWebgl, BlendFunc);
rtDefineMethod(pxWebgl, Enable);
rtDefineMethod(pxWebgl, Disable);
rtDefineMethod(pxWebgl, Flush);
rtDefineMethod(pxWebgl, CreateProgram);
rtDefineMethod(pxWebgl, CreateShader);
rtDefineMethod(pxWebgl, ShaderSource);
rtDefineMethod(pxWebgl, CompileShader);
rtDefineMethod(pxWebgl, GetShaderParameter);
rtDefineMethod(pxWebgl, GetShaderInfoLog);
rtDefineMethod(pxWebgl, AttachShader);
rtDefineMethod(pxWebgl, LinkProgram);
rtDefineMethod(pxWebgl, GetProgramParameter);
rtDefineMethod(pxWebgl, DeleteShader);
rtDefineMethod(pxWebgl, UseProgram);
rtDefineMethod(pxWebgl, GetAttribLocation);
rtDefineMethod(pxWebgl, VertexAttribPointer);
rtDefineMethod(pxWebgl, EnableVertexAttribArray);
rtDefineMethod(pxWebgl, GetUniformLocation);
rtDefineMethod(pxWebgl, UniformMatrix4fv);
rtDefineMethod(pxWebgl, DrawArrays);
rtDefineMethod(pxWebgl, Uniform2fv);
rtDefineMethod(pxWebgl, Scissor);
rtDefineMethod(pxWebgl, DisableVertexAttribArray);
rtDefineMethod(pxWebgl, CreateFramebuffer);
rtDefineMethod(pxWebgl, FramebufferTexture2D);
rtDefineMethod(pxWebgl, Uniform1f);
rtDefineMethod(pxWebgl, Uniform1i);
rtDefineMethod(pxWebgl, ActiveTexture);
rtDefineMethod(pxWebgl, GenerateMipmap);
rtDefineMethod(pxWebgl, UniformMatrix3fv);
rtDefineMethod(pxWebgl, DeleteFramebuffer);
rtDefineMethod(pxWebgl, DeleteTexture);
rtDefineMethod(pxWebgl, DeleteBuffer);
rtDefineMethod(pxWebgl, DeleteProgram);
rtDefineMethod(pxWebgl, beginNativeSparkRendering);
rtDefineMethod(pxWebgl, endNativeSparkRendering);
