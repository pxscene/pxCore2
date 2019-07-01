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

// pxShaderUtils.h

#ifndef PX_SHADERGL_H
#define PX_SHADERGL_H

#include <map>

#include "rtValue.h"

#include "pxCore.h"
#include "pxTexture.h"

#include <exception>

#ifdef ENABLE_DFB
#include "pxContextDescDFB.h"
#else
#include "pxContextDescGL.h"
#endif //ENABLE_DFB

class shaderProgram; //fwd

#define MAX_TEXTURE_WIDTH  2048
#define MAX_TEXTURE_HEIGHT 2048

#define DEFAULT_EJECT_TEXTURE_AGE 5

#ifndef ENABLE_DFB
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_IN_BYTES (65 * 1024 * 1024)   // GL
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_THRESHOLD_PADDING_IN_BYTES (5 * 1024 * 1024)
#else
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_IN_BYTES (15 * 1024 * 1024)   // DFB .. Should be 40 ?
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_THRESHOLD_PADDING_IN_BYTES (5 * 1024 * 1024)
#endif

//=====================================================================================================================================

class glException: public std::exception
{
public:
  glException(rtString err)
  {
    mErr = err;
  }
  
  virtual const char* desc() const throw()
  {
    return mErr.cString();
  }
  
private:
  rtString mErr;
};

//=====================================================================================================================================

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


//=====================================================================================================================================

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

//=====================================================================================================================================

typedef uniformLoc uniformLoc_t;

typedef std::map<rtString, uniformLoc_t>            UniformMap_t;
typedef std::map<rtString, uniformLoc_t>::iterator  UniformMapIter_t;

typedef std::vector<uniformLoc_t>                   UniformList_t;
typedef std::vector<uniformLoc_t>::iterator         UniformListIter_t;

typedef struct glShaderProgDetails
{
  GLuint program;
  GLuint fragShader;
  GLuint vertShader;
  
} glShaderProgDetails_t;

//=====================================================================================================================================

glShaderProgDetails_t  createShaderProgram(const char* vShaderTxt, const char* fShaderTxt); //fwd
void linkShaderProgram(GLuint program); //fwd

//=====================================================================================================================================

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
  
  GLint mProgram, mFragShader,    mVertShader;
  GLint mTimeLoc, mResolutionLoc, mMatrixLoc;
  GLint mPosLoc,  mUVLoc,         mAlphaLoc,   mColorLoc;
  
  UniformMap_t  mUniform_map;
  UniformList_t mUniform_list;
  
}; // CLASS

//=====================================================================================================================================

#endif // PX_SHADERGL_H
