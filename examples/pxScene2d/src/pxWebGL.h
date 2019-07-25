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

#ifndef PX_WEBGL_H
#define PX_WEBGL_H

#include "pxScene2d.h"
#include "pxObject.h"

class pxWebgl: public pxObject
{
public:
   rtDeclareObject(pxWebgl, pxObject);
  
  int pixelStorei_UNPACK_FLIP_Y_WEBGL;
  int pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL;
  int pixelStorei_UNPACK_FLIP_BLUE_RED;
  int32_t mInitialFrameBuffer;

    rtMethod4ArgAndNoReturn("drawElements", DrawElements, uint32_t, uint32_t, uint32_t, uint32_t);
    rtMethodNoArgAndReturn("createTexture", createTexture, uint32_t);
    rtMethodNoArgAndReturn("createBuffer", createBuffer, uint32_t);
    rtMethod2ArgAndNoReturn("bindTexture", bindTexture, uint32_t, uint32_t);
    rtMethod2ArgAndNoReturn("bindBuffer", bindBuffer, uint32_t, uint32_t);
    rtMethod2ArgAndNoReturn("bindFramebuffer", bindFramebuffer, uint32_t, uint32_t);
    rtMethod3ArgAndNoReturn("bufferData", bufferData, uint32_t, rtValue, uint32_t);
    rtMethod2ArgAndNoReturn("pixelStorei", pixelStorei, uint32_t, bool);
    rtMethod3ArgAndNoReturn("texParameteri", texParameteri, uint32_t, uint32_t, uint32_t);
    rtMethod4ArgAndNoReturn("viewport", viewport, uint32_t, uint32_t, uint32_t, uint32_t);
    rtMethod4ArgAndNoReturn("clearColor", clearColor, float, float, float, float);
    rtMethod9ArgAndNoReturn("texImage2D", texImage2D, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, rtValue);
    rtMethod1ArgAndNoReturn("clear", Clear, uint32_t);
    rtMethod2ArgAndNoReturn("blendFunc", BlendFunc, uint32_t, uint32_t);
    rtMethod1ArgAndNoReturn("enable", Enable, uint32_t);
    rtMethod1ArgAndNoReturn("disable", Disable, uint32_t);
    rtMethodNoArgAndReturn("createProgram", CreateProgram, uint32_t);
    rtMethod1ArgAndReturn("createShader", CreateShader, uint32_t, uint32_t);
    rtMethod2ArgAndNoReturn("shaderSource", ShaderSource, uint32_t, rtString);
    rtMethod1ArgAndNoReturn("compileShader", CompileShader, uint32_t);
    rtMethod2ArgAndReturn("getShaderParameter", GetShaderParameter, uint32_t, uint32_t, uint32_t);
    rtMethod1ArgAndReturn("getShaderInfoLog", GetShaderInfoLog, uint32_t, rtString);
    rtMethod2ArgAndNoReturn("attachShader", AttachShader, uint32_t, uint32_t);
    rtMethod1ArgAndNoReturn("linkProgram", LinkProgram, uint32_t);
    rtMethod2ArgAndReturn("getProgramParameter", GetProgramParameter, uint32_t, uint32_t, uint32_t);
    rtMethod1ArgAndNoReturn("deleteShader", DeleteShader, uint32_t);
    rtMethod1ArgAndNoReturn("useProgram", UseProgram, uint32_t);
    rtMethod2ArgAndReturn("getAttribLocation", GetAttribLocation, uint32_t, rtString, uint32_t);
    rtMethod6ArgAndNoReturn("vertexAttribPointer", VertexAttribPointer, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    rtMethod1ArgAndNoReturn("enableVertexAttribArray", EnableVertexAttribArray, uint32_t);
    rtMethod2ArgAndReturn("getUniformLocation", GetUniformLocation, uint32_t, rtString, uint32_t);
    rtMethod3ArgAndNoReturn("uniformMatrix4fv", UniformMatrix4fv, uint32_t, bool, rtValue);
    rtMethod3ArgAndNoReturn("drawArrays", DrawArrays, uint32_t, uint32_t, uint32_t);
    rtMethod2ArgAndNoReturn("uniform2fv", Uniform2fv, uint32_t, rtValue);
    rtMethod4ArgAndNoReturn("scissor", Scissor, uint32_t, uint32_t, uint32_t, uint32_t);
    rtMethod1ArgAndNoReturn("disableVertexAttribArray", DisableVertexAttribArray, uint32_t);
    rtMethodNoArgAndReturn("createFramebuffer", CreateFramebuffer, uint32_t);
    rtMethod5ArgAndNoReturn("framebufferTexture2D", FramebufferTexture2D, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    rtMethod2ArgAndNoReturn("uniform1f", Uniform1f, uint32_t, float);
    rtMethod2ArgAndNoReturn("uniform1i", Uniform1i, uint32_t, uint32_t);
    rtMethod1ArgAndNoReturn("activeTexture", ActiveTexture, uint32_t);

    pxWebgl(pxScene2d* scene);

    virtual void onInit();

    //methods
   rtError DrawElements(uint32_t mode, uint32_t count, uint32_t type, uint32_t offset);
   rtError createTexture(uint32_t& texture);
   rtError createBuffer(uint32_t& buffer);
   rtError bindTexture(uint32_t target, uint32_t texture);
   rtError bindBuffer(uint32_t target, uint32_t buffer);
   rtError bindFramebuffer(uint32_t target, uint32_t framebuffer);
   rtError bufferData(uint32_t target, rtValue data, uint32_t usage);
   rtError pixelStorei(uint32_t param, bool);
   rtError texParameteri(uint32_t target, uint32_t pname, uint32_t param);
   rtError viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
   rtError clearColor(float red, float green, float blue, float alpha);
   rtError Clear(uint32_t mask);
   rtError BlendFunc(uint32_t sfactor, uint32_t dfactor);
   rtError Enable(uint32_t cap);
   rtError Disable(uint32_t cap);
   rtError CreateProgram(uint32_t& program);
   rtError CreateShader(uint32_t type, uint32_t& shader);
   rtError texImage2D(uint32_t target, uint32_t level, uint32_t internalformat, uint32_t width, uint32_t height, uint32_t border, uint32_t format, uint32_t type, rtValue data);
   rtError ShaderSource(uint32_t shader, rtString source);
   rtError CompileShader(uint32_t shader);
   rtError GetShaderParameter(uint32_t shader, uint32_t pname, uint32_t& ret);
   rtError GetShaderInfoLog(uint32_t shader, rtString log);
   rtError AttachShader(uint32_t program, uint32_t shader);
   rtError LinkProgram(uint32_t program);
   rtError GetProgramParameter(uint32_t program, uint32_t param, uint32_t& ret);
   rtError DeleteShader(uint32_t shader);
   rtError UseProgram(uint32_t program);
   rtError GetAttribLocation(uint32_t program, rtString name, uint32_t& ret);
   rtError VertexAttribPointer(uint32_t indx, uint32_t size, uint32_t type, uint32_t normalized, uint32_t stride, uint32_t offset);
   rtError EnableVertexAttribArray(uint32_t index);
   rtError GetUniformLocation(uint32_t program, rtString name, uint32_t& ret);
   rtError UniformMatrix4fv(uint32_t location, bool transpose, rtValue data);
   rtError DrawArrays(uint32_t mode, uint32_t first, uint32_t count);
   rtError Uniform2fv(uint32_t location, rtValue data);
   rtError Scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
   rtError DisableVertexAttribArray(uint32_t index);
   rtError CreateFramebuffer(uint32_t& buffer);
   rtError FramebufferTexture2D(uint32_t target, uint32_t attachment, uint32_t textarget, uint32_t texture, uint32_t level);
   rtError Uniform1f(uint32_t location, float x);
   rtError Uniform1i(uint32_t location, uint32_t x);
   rtError ActiveTexture(uint32_t texture);

private:

     void preprocessTexImageData(void *pixels, int width, int height, int format, int type);
 };

#endif // PX_WEBGL_H