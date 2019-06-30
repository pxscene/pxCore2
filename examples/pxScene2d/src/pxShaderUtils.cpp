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

// pxShaderUtils.cpp

#include "rtLog.h"
#include "rtString.h"
#include "pxShaderUtils.h"

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
