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
  try
  {
    if(f)
    {
      rtString vtxStr(v);
      rtString frgStr(f);

      glShaderProgDetails_t details = createShaderProgram(v, f);

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
  }
  catch (glException &e)
  {
    rtLogError("FATAL: Shader error: %s \n", e.desc() );
    return RT_FAIL;
  }

  return RT_OK;
}

int shaderProgram::getUniformLocation(const char* name)
{
  if(mProgram == -1)
  {
    rtLogError("Shader program undefined.  (-1)");
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

static double dt = 0;

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

  use();

  glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
  glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);

  //
  // Update UNIFORMS ...
  //
  for (UniformMapIter_t it  = mUniform_map.begin();
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
      p.needsUpdate = false;
    }
    //    else
    //    if(p.needsUpdate == false)
    //    {
    //      continue; // SKIP
    //    }
    //    else
    //    if(p.name == "u_color")
    //    {
    //      float color[4] = {1,0,0,1};
    //      glUniform4fv(p.loc, 1, color);
    //      p.needsUpdate = false;
    //    }
    else
    if(p.name == "u_alpha")
    {
      glUniform1f(p.loc, alpha);
      p.needsUpdate = false;
    }
    else
    if(p.name == "u_resolution")
    {
      glUniform2f(p.loc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
      p.needsUpdate = false;
    }
    else
    // TODO  ... is this "s_texture" needed given BIND setFunc's
    //
    if(p.name == "s_texture")
    {
      if (t && t->bindGLTexture(p.loc) != PX_OK)
      {
        rtLogError("Texture Bind failed !!");
        //  return PX_FAIL;
      }
      p.needsUpdate = false;
    }
  }//FOR

  // Apply updated UNIFORM values to GPU...
  if(mUniform_map.size() > 0)
  {
    for(UniformMapIter_t  it = mUniform_map.begin();
                         it != mUniform_map.end(); ++it)
    {
      uniformLoc_t &p = (*it).second;

      if(p.setFunc && (p.needsUpdate || p.type == UniformType_Sampler2D ) )
      {
        p.setFunc(p); // SET UNIFORM ... set p.value .. calls glUnifornXXX() ... etc.

        p.needsUpdate = false;
      }
    }//FOR
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
  glShaderProgDetails_t details = { 0,0,0 };
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
    glGetShaderInfoLog(details.fragShader, maxLength, &maxLength, &errorLog[0]);

    rtLogError("%s", &errorLog[0]);

    //Exit with failure.
    glDeleteShader(details.fragShader); //Don't leak the shader.

    throw glException( rtString("FRAGMENT SHADER - Compile Error: ") + &errorLog[0] );
    //TODO get rid of exit
    //exit(1);
  }

  details.vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(details.vertShader, 1, (const char **) &vShaderTxt, NULL);
  glCompileShader(details.vertShader);
  glGetShaderiv(details.vertShader, GL_COMPILE_STATUS, &stat);

  if (!stat)
  {
    GLenum err = glGetError();

    rtLogError("vertex shader did not compile: %d",err);
    throw glException( rtString("VERTEX SHADER - Compile Error: ") );
    //exit(1);
  }

  details.program = glCreateProgram();
  glAttachShader(details.program, details.fragShader);
  glAttachShader(details.program, details.vertShader);
  return details;
}

void linkShaderProgram(GLuint program)
{
  GLint stat;

  glLinkProgram(program);  /* needed to put attribs into effect */
  glGetProgramiv(program, GL_LINK_STATUS, &stat);
  if (!stat)
  {
    char log[1000];
    GLsizei len;
    glGetProgramInfoLog(program, 1000, &len, log);
    rtLogError("VERTEX SHADER - Failed to link: %s", log);

    throw glException( rtString("VERTEX SHADER - Link Error: ") + log );
    // TODO purge all exit calls
    // exit(1);
  }
}

//=====================================================================================================================================
