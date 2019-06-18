//
//  pxShaderEffect.cpp
//  Spark
//
//  Created by Fitzpatrick, Hugh on 5/10/19.
//  Copyright © 2019 Comcast. All rights reserved.
//

//#include "rtCore.h"
#include "rtLog.h"
#include "pxScene2d.h"

#include "pxTimer.h"
#include "pxShaderEffect.h"

#include <exception>

////////////////////////////////////////////////////////////////
//
// Debug Statistics
#ifdef USE_RENDER_STATS
extern uint32_t gDrawCalls;
extern uint32_t gTexBindCalls;
extern uint32_t gFboBindCalls;

#define TRACK_DRAW_CALLS()   { gDrawCalls++;    }
#define TRACK_TEX_CALLS()    { gTexBindCalls++; }
#define TRACK_FBO_CALLS()    { gFboBindCalls++; }
#else
#define TRACK_DRAW_CALLS()
#define TRACK_TEX_CALLS()
#define TRACK_FBO_CALLS()
#endif

#define SAFE_DELETE(p)  if(p) { delete p; p = NULL; };

////////////////////////////////////////////////////////////////


extern pxContext context;

extern const char* vShaderText;

struct glShaderProgDetails
{
  GLuint program;
  GLuint fragShader;
  GLuint vertShader;
};

//====================================================================================================================================================================================

using namespace std;

class glException: public exception
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

/*
 pxCurrentGLProgram currentGLProgram = PROGRAM_UNKNOWN;


static glShaderProgDetails  createShaderProgram(const char* vShaderTxt, const char* fShaderTxt);
void linkShaderProgram(GLuint program);

//====================================================================================================================================================================================


//====================================================================================================================================================================================



pxShaderEffect::pxShaderEffect() : mProgram(-1), mFragShader(-1), mVertShader(-1),
                                   mTimeLoc(-1),
                                   mResolutionLoc(-1), mMatrixLoc(-1), mPosLoc(-1),
                                   mUVLoc(-1), mAlphaLoc(-1), mColorLoc(-1)
{
}

pxShaderEffect::~pxShaderEffect()
{
  glDetachShader(mProgram, mFragShader);
  glDetachShader(mProgram, mVertShader);
  glDeleteShader(mFragShader);
  glDeleteShader(mVertShader);
  glDeleteProgram(mProgram);
}

void pxShaderEffect::init(const char* v, const char* f)
{
  glShaderProgDetails details = createShaderProgram(v, f);

  mProgram    = details.program;
  mFragShader = details.fragShader;
  mVertShader = details.vertShader;

  prelink();
  linkShaderProgram(mProgram);
  postlink();
}

int pxShaderEffect::getUniformLocation(const char* name)
{
  int l = glGetUniformLocation(mProgram, name);

  if (l == -1)
    rtLogError("Shader does not define uniform %s.\n", name);

  return l;
}

void pxShaderEffect::use()
{
  currentGLProgram = PROGRAM_UNKNOWN;
  glUseProgram(mProgram);
}

static double dt = 0;

pxError pxShaderEffect::draw(int resW, int resH, float* matrix, float alpha,
                             pxTextureRef t,
                             GLenum mode,
                             const void* pos,
                             const void* uv,
                             int count)
{
 // if (currentGLProgram != PROGRAM_SOLID_SHADER)
  {
    use();
 //   currentGLProgram = PROGRAM_SOLID_SHADER;
  }

  glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
  glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);


  //
  // Update UNIFORMS ...
  //
  for (std::map< rtString, uniformLoc_t>::iterator it  = mUniform_map.begin();
       it != mUniform_map.end(); ++it)
  {
    uniformLoc_t &p = (*it).second;

    // TODO:  String matching ... yuk ... move to 'char' based switch() for types
    if(p.name == "u_time")
    {
      double time = pxMilliseconds();
      if(dt == 0)
        dt = time;

      double tt = (time - dt) / 1000.0;

      glUniform1f(p.loc, (float) tt );
    }
    else
    if(p.name == "u_color")
    {
      float color[4] = {1,0,0,1};
      glUniform4fv(p.loc, 1, color);
    }
    else
    if(p.name == "u_alpha")
    {
     glUniform1f(p.loc, alpha);
    }
    else
    if(p.name == "u_resolution")
    {
      glUniform2f(p.loc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    }
    else
    if(p.name == "s_texture")
    {
        if (t && t->bindGLTexture(p.loc) != PX_OK)
        {
        //  return PX_FAIL;
        }
    }
  }//FOR

  //
  // DRAW
  //

  if(uv)
  {
    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(mUVLoc,  2, GL_FLOAT, GL_FALSE, 0, uv);

    glEnableVertexAttribArray(mPosLoc);
    glEnableVertexAttribArray(mUVLoc);
    glDrawArrays(mode, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);
    glDisableVertexAttribArray(mUVLoc);
  }
  else
  {
    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);

    glEnableVertexAttribArray(mPosLoc);
    glDrawArrays(mode, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);
  }

  return RT_OK;
}

 */

//====================================================================================================================================================================================


pxShaderObject::~pxShaderObject()
{
  rtLogDebug("~pxShaderObject()");
}

void pxShaderObject::onInit()
{
  try
  {
    const char* vtxCode = mVertexSrc.length() > 0 ? mVertexSrc.cString() : vShaderText;

    pxShaderEffect::init( vtxCode, (const char*) mFragmentSrc.cString() );
  }
  catch(glException e)
  {
    rtLogError("Failed to compile Shader - e: %s", e.desc() );
  }

  mInitialized = true;

  //TODO add mechanism for Unifroms from passed params
}


void pxShaderObject::sendPromise()
{
 // if(mInitialized && imageLoaded && !((rtPromise*)mReady.getPtr())->status())
  {
    //rtLogDebug("pxImage SENDPROMISE for %s\n", mUrl.cString());
    mReady.send("resolve",this);
  }
}

void pxShaderObject::resourceReady(rtString /* readyResolution */)
{

}

void pxShaderObject::resourceDirty()
{

}

//
//  Setting a UNIFORM directly on a shader via Set()
//
rtError pxShaderObject::setUniformVal(const rtString& name, const rtValue& v)
{
  use();

  UniformMapIter_t it = mUniform_map.find(name);
  if(it != mUniform_map.end())
  {
    uniformLoc_t &p = (*it).second;

    p.needsUpdate = true;

    // TODO:  String matching ... yuk ... move to 'char' based switch() for types
    if(p.type == UniformType_Int)
    {
      glUniform1i(p.loc, v.toInt32() );
    }
    else
    if(p.type == UniformType_Float)
    {
      glUniform1f(p.loc, v.toFloat() );
    }
    else
    if(p.type == UniformType_Vec2)
    {
      rtArrayObject* vals = (rtArrayObject*) v.toObject().getPtr();

      if(vals)
      {
        rtValue count;                  // HACK - WORKAROUND
        vals->Get("length", &count);    // HACK - WORKAROUND

        static float vec2[2];
        int ii = 0;

        for (uint32_t i = 0, l = count.toUInt32(); i < l; ++i)
        {
          rtValue vv;
          if(vals->Get(i, &vv) == RT_OK && !vv.isEmpty())
          {
            float val = vv.toFloat();

            vec2[ii++] = val;
          }
        }//FOR

        printf("\n DEBUG: setUniformVal() ... vec2:   [%f, %f]", vec2[0], vec2[1]);

        glUniform2fv(p.loc, 1, vec2 );
      }

     // glUniform1f(mAlphaLoc, v.toFloat() );
    }
    //TODO add more types...

    return RT_OK;
  }

  return RT_FAIL; // not found
}

rtError pxShaderObject::Set(const char* name, const rtValue* value)
{
  // Only have EXPLICIT supported Properties ... otherwise its a UNIFORM
  //
  if(strcmp(name, "fragment") == 0)
  {
    return pxShaderObject::setFrgShader( value->toString() );
  }
  else
  if(strcmp(name, "vertex") == 0)
  {
    return pxShaderObject::setVtxShader( value->toString() );
  }
  else
  if(strcmp(name, "uniforms") == 0)
  {
    return pxShaderObject::setUniforms( value->toObject() );
  }
  else
  return setUniformVal(name, *value);
}

pxError pxShaderObject::draw(int resW, int resH, float* matrix, float alpha,
                        pxTextureRef t,
                        GLenum mode,
                        const void* pos,
                        int count)
{
  draw(resW, resH,  matrix, alpha, t, mode, pos, count);

  return RT_OK;
}

rtError pxShaderObject::url(rtString& /* s */) const          { return RT_OK; }
rtError pxShaderObject::setUrl(const char* /* s */)           { return RT_OK; }

rtError pxShaderObject::vtxShader(rtString& /* s */) const    { return RT_OK; }
rtError pxShaderObject::setVtxShader(const char* s)           { mVertexSrc = s;   return RT_OK; }

rtError pxShaderObject::frgShader(rtString& /* s */) const    { return RT_OK; }
rtError pxShaderObject::setFrgShader(const char* s)           { mFragmentSrc = s; return RT_OK; }

rtError pxShaderObject::uniforms(rtObjectRef& /* o */) const  { return RT_OK; }
rtError pxShaderObject::setUniforms(rtObjectRef o)
{
  rtValue allKeys;
  o->Get("allKeys", &allKeys);

  rtArrayObject* keys = (rtArrayObject*) allKeys.toObject().getPtr();
  for (uint32_t i = 0, l = keys->length(); i < l; ++i)
  {
    rtValue keyVal;
    if (keys->Get(i, &keyVal) == RT_OK && !keyVal.isEmpty())
    {
      rtString key  = keyVal.toString();
      rtString type = o.get<rtString>(key);

      printf("\n Key:  %s   Type: %s", key.cString(), type.cString() );

      if(key == "u_time")
      {
        mIsRealTime = true;
      }

      mUniform_map[key] = { key, type, UniformType_Unknown, (GLint) -1, 0, false }; // name, typeStr, type, loc, value, needsUpdate
    }
  }//FOR

  mUniforms = o;
  return RT_OK;
}


void pxShaderObject::prelink()
{
  mPosLoc = 0;
  mUVLoc = 1;
  glBindAttribLocation(mProgram, mPosLoc, "pos");
  glBindAttribLocation(mProgram, mUVLoc,  "uv");
}

void pxShaderObject::postlink()
{
  mTimeLoc       = getUniformLocation("u_time");
  mResolutionLoc = getUniformLocation("u_resolution");
  mMatrixLoc     = getUniformLocation("amymatrix");
//  mColorLoc      = getUniformLocation("a_color");
//  mAlphaLoc      = getUniformLocation("u_alpha");

  for (std::map< rtString, uniformLoc_t>::iterator it  = mUniform_map.begin();
                                                   it != mUniform_map.end(); ++it)
  {
    (*it).second.loc = getUniformLocation( (*it).second.name );
  }
}

rtDefineObject(pxShaderObject,   pxObject);

rtDefineProperty(pxShaderObject, url);
rtDefineProperty(pxShaderObject, frgShader);
rtDefineProperty(pxShaderObject, vtxShader);
rtDefineProperty(pxShaderObject, uniforms);

rtDefineMethod(pxShaderObject, setUniformVal);
