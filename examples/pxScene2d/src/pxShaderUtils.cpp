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

#include "rtLog.h"
#include "rtString.h"
#include "rtValue.h"

#include "pxTimer.h"

#include "pxShaderUtils.h"

extern const char* vShaderText;
extern int currentGLProgram;

extern rtValue copyUniform(UniformType_t type, rtValue &val);        // helper to copy "variant" like UNIFORM types

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

shaderProgram::shaderProgram() : mProgram(-1), mFragShader(-1), mVertShader(-1), mTimeLoc(-1),
mResolutionLoc(-1), mMatrixLoc(-1), mPosLoc(-1),
mUVLoc(-1), mAlphaLoc(-1), mColorLoc(-1)
{
  mUniform_map.clear();
}

shaderProgram::~shaderProgram()
{
  glDetachShader(mProgram, mFragShader);
  glDetachShader(mProgram, mVertShader);
  glDeleteShader(mFragShader);
  glDeleteShader(mVertShader);
  glDeleteProgram(mProgram);

  // mUniform_map.clear();
}

pxError shaderProgram::initShader(const char* v, const char* f)
{
  if(f)
  {
    rtString vtxStr(v);
    rtString frgStr(f);

    glShaderProgDetails_t details = createShaderProgram(v, f);

    if(details.didError)
    {
      mProgram     = -1; // ERROR
      mCompilation = details.compilation;
      return RT_FAIL;
    }

    mProgram    = details.program;
    mFragShader = details.fragShader;
    mVertShader = details.vertShader;

    prelink();
    linkShaderProgram(mProgram);
    postlink();
  }
  else
  {
    rtLogError("No FRAGMENT Shader defined. \n");
    return RT_FAIL;
  }

  return RT_OK;
}

int shaderProgram::getUniformLocation(const char* name)
{
  if(mProgram == -1)
  {
    rtLogWarn("getUniformLocation() - '%s' ... Shader program undefined.  (-1)", name);
    return -1;
  }

  int l = glGetUniformLocation(mProgram, name);

  if (l == -1)
  {
    rtLogWarn("Shader does not define uniform '%s'.\n", name);
  }

  return l;
}

void shaderProgram::use()
{
  if(currentGLProgram != (int) mProgram)
  {
    currentGLProgram = (int) mProgram;
    glUseProgram(mProgram);
  }
}

void shaderProgram::saveUniforms()
{
  copyUniforms(mUniform_map, mUniform_map2);
}

void shaderProgram::restoreUniforms()
{
  copyUniforms(mUniform_map2, mUniform_map, true); // needs update
}

void shaderProgram::copyUniforms(UniformMap_t &from, UniformMap_t &to, bool needsUpdate /* = false */)
{
  //
  // Copy the UNIFORM: "from" -> "to" ...
  //
  if(from.size() > 0)
  {
    for(UniformMapIter_t it  = from.begin();
                         it != from.end(); ++it)
    {
      const rtString &n = (*it).first;
      uniformLoc_t   &p = (*it).second;

      to[n].value = copyUniform(p.type, p.value);

	  to[n].needsUpdate = needsUpdate;
    }
  }//ENDIF
}


static double last_ms = 0.0;

pxError shaderProgram::draw(int resW, int resH, float* matrix, float alpha,
                            pxTextureRef t,
                            GLenum mode,
                            const void* pos,
                            const void* uv,
                            int count)
{
  if(!resW || !resH || !matrix || !pos)
  {
    return RT_FAIL;
  }

  if(mResolutionLoc == -1)
  {
    return RT_FAIL;
  }

  use(); // this program ! (if different)

  //
  // Always update UNIFORM(S) ...
  //
  glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
  glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));

  //
  // Update specific UNIFORMS ...
  //
  if(mTimeLoc != -1)
  {
    double time_ms = pxMilliseconds();
    if(last_ms == 0)
      last_ms = time_ms;

    glUniform1f(mTimeLoc, (float) ((time_ms - last_ms) / 1000.0) );
  }

  if(mAlphaLoc != -1)
  {
    glUniform1f(mAlphaLoc, alpha);
  }

//  if(mColorLoc != -1)
//  {
//    float color[4] = {1,0,0,1};
//    glUniform4fv(mColorLoc, 1, color);
//  }

  //
  // Bind to object FBO if needed
  //
  if(t)
  {
    UniformMapIter_t found = mUniform_map.find( "s_texture" );
    if(found !=  mUniform_map.end() )
    {
      uniformLoc_t &p = (*found).second;

      if(t->bindGLTexture(p.loc) != PX_OK)  // to GL_TEXTURE1
      {
        rtLogError("Texture Bind failed !!");
      }
    }
  }

  //
  // Apply updated UNIFORM values to GPU...
  //
  if(mUniform_map.size() > 0)
  {
    for(UniformMapIter_t it  = mUniform_map.begin();
                         it != mUniform_map.end(); ++it)
    {
      uniformLoc_t &p = (*it).second;

      if(p.setFunc && p.needsUpdate)
      {
        p.setFunc(p); // SET UNIFORM ... set p.value .. calls glUniformXXX() and glBindTexture(3,4,5) calls ... etc.

        p.needsUpdate = (p.type == UniformType_Sampler2D); // always bind Samplers... otherwise once will do.
      }
    }
  }//ENDIF

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

//=====================================================================================================================================

glShaderProgDetails_t  createShaderProgram(const char* vShaderTxt, const char* fShaderTxt)
{
  glShaderProgDetails_t details = { 0,0,0,   false, "" };
  GLint stat;

  details.fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(details.fragShader, 1, (const char **) &fShaderTxt, NULL);
  glCompileShader(details.fragShader);
  glGetShaderiv(details.fragShader, GL_COMPILE_STATUS, &stat);

  if (!stat)
  {
    rtLogError("FRAGMENT SHADER - Error: Shader did not compile: %d", glGetError());

    GLint maxLength = 0;
    glGetShaderiv(details.fragShader, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    std::vector<char> errorLog(maxLength);
    glGetShaderInfoLog(details.fragShader, maxLength, &maxLength, (char *) &errorLog[0]);

    rtLogError("%s", (char *) &errorLog[0]);

    //Exit with failure.
    glDeleteShader(details.fragShader); //Don't leak the shader.

    details.didError    = true; // ERROR
    details.compilation = rtString("FRAGMENT SHADER - Compile Error: ") + &errorLog[0];

    return details;
  }

  details.vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(details.vertShader, 1, (const char **) &vShaderTxt, NULL);
  glCompileShader(details.vertShader);
  glGetShaderiv(details.vertShader, GL_COMPILE_STATUS, &stat);

  if (!stat)
  {
    char log[1000];
    GLsizei len;
    glGetShaderInfoLog(details.vertShader, 1000, &len, log);
    rtLogError("VERTEX SHADER - Failed to compile:  [%s]", log);

    GLenum err = glGetError();
    rtLogError("VERTEX SHADER - glGetError(): %d",err);

    details.didError    = true; // ERROR
    details.compilation = rtString("VERTEX SHADER - Compile Error: ") + rtString(log);

    return details;
  }

  details.program = glCreateProgram();
  glAttachShader(details.program, details.fragShader);
  glAttachShader(details.program, details.vertShader);
  return details;
}

void linkShaderProgram(GLuint program)
{
  GLint stat;

  glLinkProgram(program);  // needed to put attribs into effect
  glGetProgramiv(program, GL_LINK_STATUS, &stat);

  if (!stat)
  {
    char log[1000];
    GLsizei len;
    glGetProgramInfoLog(program, 1000, &len, log);
    rtLogError("VERTEX SHADER - Failed to link:  [%s]", log);

//    details.program     = -1; // ERROR
//    details.compilation = rtString("VERTEX SHADER - Link Error: ") + rtString(log);

    // TODO purge all exit calls
    // exit(1);
  }
}

//=====================================================================================================================================
