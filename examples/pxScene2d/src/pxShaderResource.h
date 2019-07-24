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

  Created by Fitzpatrick, Hugh on 5/10/19.
*/

#ifndef PXSHADERRESOURCE_H
#define PXSHADERRESOURCE_H

#include "pxContext.h"
#include "pxObject.h"
#include "pxResource.h"

#include "pxShaderUtils.h"

#include <map>
#include <vector>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
#include <GLES2/gl2.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#ifdef WIN32
#include <GL/wglew.h>
#endif // WIN32
#ifdef PX_PLATFORM_GLUT
#include <GL/glut.h>
#endif
#include <GL/gl.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif

class pxObject; // fwd

//=====================================================================================================================================

rtError        applyRTconfig(rtObjectRef v, pxObject &obj);    // Set a (RT) CONFIG object of UNIFORMS to Shader
rtError        applyJSconfig(rtObjectRef v, pxObject &obj);    // Set a (JS) CONFIG object of UNIFORMS to Shader

rtObjectRef    copyConfigJS2RT(rtObjectRef &v);                // Copy  (JS) CONFIG object  to (RT) Config object  ... single
rtObjectRef     copyArrayJS2RT(rtObjectRef &v);                // Copy  (JS) CONFIG object to (RT) Config objects  ... array

rtError findKey(rtArrayObject* array, rtString k);            // helper to find key in array
rtValue copyUniform(UniformType_t type, rtValue &val);        // helper to copy "variant" like UNIFORM types

//=====================================================================================================================================

class rtShaderResource: public pxResource, public shaderProgram
{
public:
  rtShaderResource();
  rtShaderResource(const char* fragmentUrl, const char* vertexUrl = NULL, const rtCORSRef& cors = NULL, rtObjectRef archive = NULL);

  virtual ~rtShaderResource();

  rtDeclareObject(rtShaderResource, pxResource);

  virtual unsigned long Release();

  rtProperty(uniforms, uniforms, setUniforms, rtObjectRef);

  bool isRealTime() { return mIsRealTime; }
  bool needsFbo()   { return mNeedsFbo;   }
  int32_t passes()  { return mPasses;     }

  virtual void setupResource();
  virtual void init();
  virtual void onInit();

  rtError uniforms(rtObjectRef& o) const;
  rtError setUniforms(rtObjectRef o);

  virtual rtError Set(const char* name, const rtValue* value);

  UniformType_t getUniformType(rtString &name);

  rtError setUniformVal(const rtString& name, const rtValue& v);
  rtError setUniformVals(const rtValue& v);

  // Loading
  //
  virtual void     loadResource(rtObjectRef archive = NULL, bool reloading=false);
  virtual uint32_t loadResourceData(rtFileDownloadRequest* fileDownloadRequest);
  virtual void     loadResourceFromFile();
  virtual void     loadResourceFromArchive(rtObjectRef /*archiveRef*/);

protected:
  //
  // GL Setters
  //
  static rtError fillInt32Vec(int32_t *vec, const rtArrayObject* vals); // generic code
  static rtError fillFloatVec(float   *vec, const rtArrayObject* vals); // generic code

  // Samplers
  static rtError bindTexture3(const uniformLoc_t &p)   { return bindTextureN(3, p); };
  static rtError bindTexture4(const uniformLoc_t &p)   { return bindTextureN(4, p); };
  static rtError bindTexture5(const uniformLoc_t &p)   { return bindTextureN(5, p); };

  static rtError bindTextureN(GLenum n, const uniformLoc_t &p); // generic setter

  // Scalars
  static rtError setUniform1i(const uniformLoc_t &p)   { glUniform1i(p.loc, p.value.toInt32() ); return RT_OK; }
  static rtError setUniform1f(const uniformLoc_t &p)   { glUniform1f(p.loc, p.value.toFloat() ); return RT_OK; }

  // INT vectors
  static rtError setUniform2iv(const uniformLoc_t &p)  { return setUniformNiv(2, p); }
  static rtError setUniform3iv(const uniformLoc_t &p)  { return setUniformNiv(3, p); }
  static rtError setUniform4iv(const uniformLoc_t &p)  { return setUniformNiv(4, p); }

  static rtError setUniformNiv(int N, const uniformLoc_t &p); // generic setter

  // FLOAT vectors
  static rtError setUniform2fv(const uniformLoc_t &p)  { return setUniformNfv(2, p); }
  static rtError setUniform3fv(const uniformLoc_t &p)  { return setUniformNfv(3, p); }
  static rtError setUniform4fv(const uniformLoc_t &p)  { return setUniformNfv(4, p); }

  static rtError setUniformNfv(int N, const uniformLoc_t &p); // generic setter

  // Override to do uniform lookups
  virtual void prelink();
  virtual void postlink();

private:
  bool        mIsRealTime;
  bool        mNeedsFbo;

  // Property ivars
  //
  int32_t     mPasses;
  int32_t     mSamplerCount;

//  rtObjectRef mUniforms;
//  rtObjectRef mAttributes;

  rtString    mVertexUrl;
  rtData      mVertexSrc;

  rtString    mFragmentUrl;
  rtData      mFragmentSrc;

}; // CLASS - pxShaderResource

//=====================================================================================================================================

typedef rtRef<rtShaderResource> pxShaderResourceRef;

typedef std::map<uint32_t, rtShaderResource*> ShaderMap;
typedef std::map<rtString, uint32_t> ShaderUrlMap;

class pxShaderManager
{
public:

  static pxShaderResourceRef getShader(const char* fragmentUrl, const char* vertexUrl = NULL, const rtCORSRef& cors = NULL, rtObjectRef archive = NULL);
  static void removeShader(rtString shaderUrl);
  static void removeShader(uint32_t shaderId);
  static void clearAllShaders();

protected:
  static ShaderMap    mShaderMap;
  static ShaderUrlMap mShaderUrlMap;
  static bool init;
};

//=====================================================================================================================================

#endif /* PXSHADERRESOURCE_H */
