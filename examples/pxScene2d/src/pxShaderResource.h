//
//  pxShaderResource.hpp
//  Spark
//
//  Created by Fitzpatrick, Hugh on 5/10/19.
//  Copyright © 2019 Comcast. All rights reserved.
//

#ifndef PXSHADERRESOURCE_HPP
#define PXSHADERRESOURCE_HPP

#include "pxContext.h"
#include "pxResource.h"

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


struct uniformLoc; //fwd

typedef  void (*setFunc_t)(const uniformLoc &);

typedef enum UniformType
{
  UniformType_Unknown,
  UniformType_Bool,
  UniformType_Int,
  UniformType_UInt,
  UniformType_Float,
  UniformType_Double,
  
// TODO: Possibly not in GL ES 2.0
//
//  // Vectors:  bool
//  UniformType_bVec2,
//  UniformType_bVec3,
//  UniformType_bVec4,
  
  // Vectors:  int
  UniformType_iVec2,
  UniformType_iVec3,
  UniformType_iVec4,

// TODO: Possibly not in GL ES 2.0
//
//  // Vectors:  uint
//  UniformType_uVec2,
//  UniformType_uVec3,
//  UniformType_uVec4,
  
  // Vectors:  float
  UniformType_Vec2,
  UniformType_Vec3,
  UniformType_Vec4,
  
// TODO: Possibly not in GL ES 2.0
//
//  // Vectors:  double
//  UniformType_dVec2,
//  UniformType_dVec3,
//  UniformType_dVec4,
  
  UniformType_Sampler2D,
  UniformType_Struct
  
} UniformType_t;


struct uniformLoc
{  
  rtString       name;         // uniform NAME
  rtString       typeStr;      // GLSL type name
  UniformType_t  type;
  GLint          loc;          // uniform location in Shader
  
  rtValue        value;        // rtValue to be applied per draw.
  bool           needsUpdate;  // flag to set in shader only whence changed.
  
  setFunc_t      setFunc;      // static pointer to *setter* function to unwrap and apply rtValue into Shader
};

typedef uniformLoc uniformLoc_t;

typedef std::map<rtString, uniformLoc_t> UniformMap_t;
typedef std::map<rtString, uniformLoc_t>::iterator UniformMapIter_t;

typedef std::vector<uniformLoc_t> UniformList_t;
typedef std::vector<uniformLoc_t>::iterator UniformListIter_t;

//=====================================================================================================================================

enum pxCurrentGLProgram { PROGRAM_UNKNOWN = 0, PROGRAM_SOLID_SHADER,  PROGRAM_A_TEXTURE_SHADER, PROGRAM_TEXTURE_SHADER,
  PROGRAM_TEXTURE_MASKED_SHADER, PROGRAM_TEXTURE_BORDER_SHADER};

class shaderProgram
{
public:

  shaderProgram();

  virtual ~shaderProgram();
  
  void initShader(const char* v, const char* f);
  
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

  GLint mProgram, mFragShader, mVertShader;
  
  GLint mTimeLoc;
  
  GLint mResolutionLoc;
  GLint mMatrixLoc;
  
  GLint mPosLoc;
  GLint mUVLoc;
  
  GLint mAlphaLoc;
  GLint mColorLoc;
  
  UniformMap_t  mUniform_map;
  UniformList_t mUniform_list;

}; // CLASS

//=====================================================================================================================================

class rtShaderResource: public pxResource, shaderProgram
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
  virtual void     loadResourceFromArchive(rtObjectRef archiveRef);

  //
  // GL Setters
  //
  static rtError fillInt32Vec(int32_t *vec, const rtArrayObject* vals);
  static rtError fillFloatVec(float   *vec, const rtArrayObject* vals);

  // samplers
  static void bindTexture3(const uniformLoc_t &p);
  static void bindTexture4(const uniformLoc_t &p);
  static void bindTexture5(const uniformLoc_t &p);
  
  // Scalars
  static void setUniform1i (const uniformLoc_t &p)  { glUniform1i(p.loc, p.value.toInt32() ); };
  static void setUniform1f (const uniformLoc_t &p)  { glUniform1f(p.loc, p.value.toFloat() ); };

  // INT vectors
  static void setUniform2iv(const uniformLoc_t &p)  { setUniformNiv(2, p); }
  static void setUniform3iv(const uniformLoc_t &p)  { setUniformNiv(3, p); }
  static void setUniform4iv(const uniformLoc_t &p)  { setUniformNiv(4, p); }
  
  static void setUniformNiv(int N, const uniformLoc_t &p);

  // FLOAT vectors
  static void setUniform2fv(const uniformLoc_t &p)  { setUniformNfv(2, p); }
  static void setUniform3fv(const uniformLoc_t &p)  { setUniformNfv(3, p); }
  static void setUniform4fv(const uniformLoc_t &p)  { setUniformNfv(4, p); }

  static void setUniformNfv(int N, const uniformLoc_t &p);

protected:
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

  rtObjectRef mUniforms;
  rtObjectRef mAttributes;

  rtString    mVertexUrl;
  rtData      mVertexSrc;
  
  rtString    mFragmentUrl;
  rtData      mFragmentSrc;

}; // CLASS - pxShaderResource

//=====================================================================================================================================

typedef std::map<uint32_t, rtShaderResource*> ShaderMap;
typedef std::map<rtString, uint32_t> ShaderUrlMap;

class pxShaderManager
{
public:

  static rtRef<rtShaderResource> getShader(const char* fragmentUrl, const char* vertexUrl = NULL, const rtCORSRef& cors = NULL, rtObjectRef archive = NULL);
  static void removeShader(rtString shaderUrl);
  static void removeShader(uint32_t shaderId);
  static void clearAllShaders();
  
protected:
  static ShaderMap    mShaderMap;
  static ShaderUrlMap mShaderUrlMap;
  static bool init;
};

//=====================================================================================================================================

#endif /* PXSHADERRESOURCE_HPP */
