//
//  pxShaderEffect.hpp
//  Spark
//
//  Created by Fitzpatrick, Hugh on 5/10/19.
//  Copyright © 2019 Comcast. All rights reserved.
//

#ifndef PXSHADEREFFECT_HPP
#define PXSHADEREFFECT_HPP

class pxScene2d; //fwd

#include "pxContext.h"
#include "pxObject.h"
#include "pxResource.h"

#include "pxShaderResource.h"
//#include "rtObject.h"

#include <map>


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

/*
 
typedef struct
{
  rtString  name;
  rtString  type;
  GLint      loc;
}
uniformLoc_t;

typedef std::map<rtString, uniformLoc_t> UniformMap_t;
typedef std::map<rtString, uniformLoc_t>::iterator UniformMapIter_t;

//====================================================================================================================================================================================

 
enum pxCurrentGLProgram { PROGRAM_UNKNOWN = 0, PROGRAM_SOLID_SHADER,  PROGRAM_A_TEXTURE_SHADER, PROGRAM_TEXTURE_SHADER,
  PROGRAM_TEXTURE_MASKED_SHADER, PROGRAM_TEXTURE_BORDER_SHADER};


class pxShaderEffect
{
public:

  pxShaderEffect();

  virtual ~pxShaderEffect();
  virtual void init(const char* v, const char* f);
  
  int getUniformLocation(const char* name);
  
  void use();
  
  pxError draw(int resW, int resH, float* matrix, float alpha,
               pxTextureRef t, 
               GLenum mode,
               const void* pos,
               const void* uv,
               int count);

protected:
  // Override to do uniform lookups
  virtual void prelink()  {}
  virtual void postlink() {}

  GLuint mProgram, mFragShader, mVertShader;
  
  GLint mTimeLoc;
  
  GLint mResolutionLoc;
  GLint mMatrixLoc;
  
  GLint mPosLoc;
  GLint mUVLoc;
  
  GLint mAlphaLoc;
  GLint mColorLoc;
  
  UniformMap_t mUniform_map;

}; // CLASS

 */

//====================================================================================================================================================================================

class pxShaderObject: public pxObject, pxShaderEffect
{
public:
  
  rtDeclareObject(pxShaderObject, pxObject);
  rtProperty(url,   url,  setUrl, rtString);
  
  rtProperty(frgShader, frgShader, setFrgShader, rtString);
  rtProperty(vtxShader, vtxShader, setVtxShader, rtString);

  rtProperty(uniforms, uniforms, setUniforms, rtObjectRef);

  rtMethod2ArgAndNoReturn("setUniformVal", setUniformVal, rtString, rtValue);
  
  bool isRealTime() { return mIsRealTime; };

  pxShaderObject(pxScene2d* scene): pxObject(scene), mIsRealTime(false) { };
  
  virtual ~pxShaderObject();

  virtual void onInit();
  virtual void sendPromise();

  virtual void resourceReady(rtString readyResolution);
  virtual void resourceDirty();
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);
  
  rtError frgShader(rtString& s) const;
  rtError setFrgShader(const char* s);
  
  rtError vtxShader(rtString& s) const;
  rtError setVtxShader(const char* s);
  
//  rtError attributes(rtObjectRef& o) const;
//  rtError setAttributes(rtObjectRef o);
  
  rtError uniforms(rtObjectRef& o) const;
  rtError setUniforms(rtObjectRef o);

  virtual rtError Set(const char* name, const rtValue* value);
  
  rtError setUniform(rtObjectRef o);
  rtError setUniformVal(const rtString& name, const rtValue& v);

  pxError draw(int resW, int resH, float* matrix, float alpha,
               pxTextureRef t,
               GLenum mode, const void* pos, int count);
  
  
protected:
  // Override to do uniform lookups
  virtual void prelink();
  virtual void postlink();
  
private:
  
  bool        mIsRealTime;
  rtString    mVertexSrc;
  rtString    mFragmentSrc;

  rtObjectRef mUniforms;
  rtObjectRef mAttributes;

}; // CLASS - pxShaderObject

//====================================================================================================================================================================================
#endif /* PXSHADEREFFECT_HPP */
