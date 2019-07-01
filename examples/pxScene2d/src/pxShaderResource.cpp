//
//  pxShaderResource.cpp
//  Spark
//
//  Created by Fitzpatrick, Hugh on 5/10/19.
//  Copyright © 2019 Comcast. All rights reserved.
//

#include "rtLog.h"

#include "rtThreadQueue.h"
#include "rtFileDownloader.h"
#include "rtString.h"
#include "rtRef.h"

#include "rtPathUtils.h"
#include "pxTimer.h"

#include "pxContext.h"

#include "pxScene2d.h"
#include "pxShaderResource.h"


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

using namespace std;

extern const char* vShaderText;
extern int currentGLProgram;

//=====================================================================================================================================

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

void shaderProgram::initShader(const char* v, const char* f)
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
  }
}

int shaderProgram::getUniformLocation(const char* name)
{
  int l = glGetUniformLocation(mProgram, name);
  
  if (l == -1)
    rtLogWarn("Shader does not define uniform '%s'.\n", name);
  
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
  if(mResolutionLoc == -1)
    return RT_OK;
  
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
    else
    if(p.name == "u_color")
    {
      float color[4] = {1,0,0,1};
      glUniform4fv(p.loc, 1, color);
      p.needsUpdate = false;
    }
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
  
  // Apply UNIFORM values to GPU...
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

rtShaderResource::rtShaderResource(const char* fragmentUrl, const char* vertexUrl /*= NULL*/,
                                   const rtCORSRef& /*cors = NULL*/, rtObjectRef /*archive = NULL*/)

  : pxResource(), mIsRealTime(false), mNeedsFbo(false), mPasses(1), mSamplerCount(3)
{
  mFragmentUrl = fragmentUrl;
  mVertexUrl   = vertexUrl;
}

rtShaderResource::~rtShaderResource()
{
  rtLogDebug("~rtShaderResource()");
}

void rtShaderResource::onInit()
{
  postlink();
  
  if( mInitialized)
    return;
  
  mInitialized = true;
}

void rtShaderResource::init()
{
  if( mInitialized)
    return;
  
  mInitialized = true;
}

void rtShaderResource::setupResource()
{
  rtShaderResource::init();
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // CREATE SHADER PROGRAMS
  
  if(mFragmentSrc.length() > 0)
  {
    // Setup shader
    double startDecodeTime = pxMilliseconds();
    
    const char* vtxCode = mVertexSrc.length() > 0 ? (const char*) mVertexSrc.data() : vShaderText;
    
    // TODO:  TRY
    shaderProgram::initShader( vtxCode, (const char*) mFragmentSrc.data() );
    // TODO:  CATCH
    
    double stopDecodeTime = pxMilliseconds();
    setLoadStatus("decodeTimeMs", static_cast<int>(stopDecodeTime-startDecodeTime));
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}


unsigned long rtShaderResource::Release()
{
  long l = rtAtomicDec(&mRefCount);
  if (l == 0)
  {
    pxShaderManager::removeShader( mVertexUrl   );
    pxShaderManager::removeShader( mFragmentUrl );

    delete this;
  }
  return l;
}

UniformType_t rtShaderResource::getUniformType(rtString &name)
{
  UniformMapIter_t it = mUniform_map.find(name.cString());
  if(it != mUniform_map.end())
  {
    uniformLoc_t &p = (*it).second;

    return p.type;
  }
  
  return UniformType_Unknown;
}

//
//  Set() >> setUniformVal() ... called when setting Uniforms directly on a [shaderProgram].
//
rtError rtShaderResource::setUniformVal(const rtString& name, const rtValue& v)
{
  use();
  
  UniformMapIter_t it = mUniform_map.find(name);
  if(it != mUniform_map.end())
  {
    uniformLoc_t &p = (*it).second;
    
    p.value = v;        // update the VALUE
    p.needsUpdate = true;
    
    return RT_OK;
  }
  
  return RT_FAIL; // not found
}

rtError rtShaderResource::setUniformVals(const rtValue& v)
{
  rtObjectRef uniforms = v.toObject();
  rtArrayObject   *arr = (rtArrayObject *) uniforms.getPtr();
  
  if(arr == NULL)
  {
    return RT_FAIL;
  }

  rtValue     keysVal;
  uniforms->Get("allKeys", &keysVal);
  
  rtObjectRef keys = keysVal.toObject();
  uint32_t     len = keys.get<uint32_t>("length");
  
  // Iterate UNIFORMS map
  for (uint32_t i = 0; i < len; i++)
  {
    rtValue  val;
    rtString key = keys.get<rtString>(i);

    if (arr->Get(key, &val) == RT_OK && !val.isEmpty())
    {
      setUniformVal(key, val); // update this value in Uniforms map...
    }
  }

  return RT_OK;
}

rtError rtShaderResource::Set(const char* name, const rtValue* value)
{
  // Only have EXPLICIT supported Properties ... otherwise its a UNIFORM
  //

  if(strcmp(name, "passes") == 0)
  {
    mPasses = value->toInt32();
    return RT_OK;
  }
  else
  if(strcmp(name, "uniforms") == 0)
  {
    return rtShaderResource::setUniforms( value->toObject() );
  }
  if(strcmp(name, "t") == 0)
  {
    return RT_OK; // ignore
  }
  else
  {
    return setUniformVal(name, *value);
  }
}


//
// Stores the UNIFORMS parsed from JS code/config; to be located in Shader when compiled
//
rtError rtShaderResource::uniforms(rtObjectRef& /* o */) const { return RT_OK; }
rtError rtShaderResource::setUniforms(rtObjectRef o)
{
  rtValue allKeys;
  o->Get("allKeys", &allKeys);
  
  rtArrayObject* keys = (rtArrayObject*) allKeys.toObject().getPtr();
  for (uint32_t i = 0, l = keys->length(); i < l; ++i)
  {
    rtValue keyVal;
    if (keys->Get(i, &keyVal) == RT_OK && !keyVal.isEmpty())
    {
      setFunc_t     setFunc = NULL;
      UniformType_t typeEnum = UniformType_Unknown;

      rtString name = keyVal.toString();
      rtString type = o.get<rtString>(name);

      if(name == "u_time")
      {
        mIsRealTime = true;
      }
      else
      if(type == "int")
      {
        setFunc = &rtShaderResource::setUniform1f;
        typeEnum = UniformType_Int;
      }
      else
      if(type == "float")
      {
        setFunc = rtShaderResource::setUniform1f;
        typeEnum = UniformType_Float;
      }
      else
      if(type == "vec2")
      {
        setFunc = rtShaderResource::setUniform2fv;
        typeEnum = UniformType_Vec2;
      }
      else
      if(type == "vec3")
      {
        setFunc = rtShaderResource::setUniform3fv;
        typeEnum = UniformType_Vec3;
      }
      else
      if(type == "vec4")
      {
        setFunc = rtShaderResource::setUniform4fv;
        typeEnum = UniformType_Vec4;
      }
      else
      if(type == "sampler2D")
      {
        //if(name == "s_texture")
        {
          mNeedsFbo = true;
        }
        setFunc = (mSamplerCount == 3) ? rtShaderResource::bindTexture3 :
                  (mSamplerCount == 4) ? rtShaderResource::bindTexture4 :
                  (mSamplerCount == 5) ? rtShaderResource::bindTexture5 : NULL;
        
        typeEnum = UniformType_Sampler2D;
        
        mSamplerCount++;
      }

      mUniform_map[name] = { name, type, typeEnum, (GLint) -1, rtValue(), false, setFunc };  // ADD TO MAP
    }
  }//FOR

  mUniforms = o;
  return RT_OK;
}

void rtShaderResource::loadResourceFromFile()
{
  init();
  
  rtString status = "resolve";
  
  rtError loadFrgShader = RT_FAIL;

  if(mVertexUrl.beginsWith("data:text/plain,"))
  {
    mVertexSrc.init( (const uint8_t* ) mVertexUrl.substring(16).cString(), mVertexUrl.length() - 16);
  }
  if(mFragmentUrl.beginsWith("data:text/plain,"))
  {
    mFragmentSrc.init( (const uint8_t* ) mFragmentUrl.substring(16).cString(), mFragmentUrl.length() - 16);
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  do
  {
    if (mFragmentSrc.length() != 0)
    {
      // We have FRAGMENT SHADER source already...
      loadFrgShader = RT_OK;
      break;
    }
    
    loadFrgShader = rtLoadFile( mFragmentUrl, mFragmentSrc);
    if (loadFrgShader == RT_OK)
      break;
    
    if (rtIsPathAbsolute(mFragmentUrl))
      break;
    
    rtModuleDirs *dirs = rtModuleDirs::instance();
    
    for (rtModuleDirs::iter it = dirs->iterator(); it.first != it.second; it.first++)
    {
      if (rtLoadFile(rtConcatenatePath(*it.first, mFragmentUrl.cString()).c_str(), mFragmentSrc) == RT_OK)
      {
        loadFrgShader = RT_OK;
        break;
      }
    }
  } while(0);
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  rtError loadVtxShader = RT_FAIL;
  
  do
  {
    if (mVertexSrc.length() != 0)
    {
      // We have VERTEX SHADER source already...
      loadVtxShader = RT_OK;
      break;
    }
    
    if (mVertexSrc.length() == 0)
    {
      //Use the Default VERTEX SHADER source...
      loadVtxShader = RT_OK;
      break;
    }
    
    loadVtxShader = rtLoadFile(mVertexUrl, mVertexSrc);
    if (loadVtxShader == RT_OK)
      break;
    
    if (rtIsPathAbsolute(mVertexUrl))
      break;
    
    rtModuleDirs *dirs = rtModuleDirs::instance();
    
    for (rtModuleDirs::iter it = dirs->iterator(); it.first != it.second; it.first++)
    {
      if (rtLoadFile(rtConcatenatePath(*it.first, mVertexUrl.cString()).c_str(), mVertexSrc) == RT_OK)
      {
        loadVtxShader = RT_OK;
        break;
      }
    }
  } while(0);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // CREATE SHADER PROGRAMS
  
  if (loadFrgShader == RT_OK && loadVtxShader == RT_OK)
  {
    setupResource();
  }
  else
  {
    loadFrgShader = RT_RESOURCE_NOT_FOUND;
    rtLogError("Could not load FRAGMENT shader file %s.", mFragmentUrl.cString());
  }
  if ( loadFrgShader != RT_OK)
  {
    rtLogWarn("shader load failed"); // TODO: why?
    if (loadFrgShader == RT_RESOURCE_NOT_FOUND)
    {
      setLoadStatus("statusCode",PX_RESOURCE_STATUS_FILE_NOT_FOUND);
    }
    else
    {
      setLoadStatus("statusCode", PX_RESOURCE_STATUS_DECODE_FAILURE);
    }
    
    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();
    
    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void*)"reject");
    }
    //mTexture->notifyListeners( mTexture, RT_FAIL, errorCode);
  }
  else
  {
    mFragmentSrc.term(); // Dump the source data...
    mVertexSrc.term();   // Dump the source data...

    setLoadStatus("statusCode",0);
    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();
    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void *) "resolve");
    }
  }
}

void rtShaderResource::loadResourceFromArchive(rtObjectRef archiveRef)
{
  pxArchive* archive = (pxArchive*)archiveRef.getPtr();
  rtString status = "resolve";
  
  rtError loadShaderSuccess = RT_FAIL;
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if(mFragmentSrc.length() == 0)
  {
    if ((NULL != archive) && (RT_OK == archive->getFileData(mFragmentUrl, mFragmentSrc)))
    {
      loadShaderSuccess = RT_OK;
    }
    else
    {
      loadShaderSuccess = RT_RESOURCE_NOT_FOUND;
      rtLogError("Could not load shader file from archive %s.", mUrl.cString());
    }
  }
  else
  {
    // We have SHADER source as a string ...
    loadShaderSuccess = RT_OK;
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // CREATE SHADER PROGRAMS

  if (loadShaderSuccess == RT_OK)
  {
    double startDecodeTime = pxMilliseconds();
    loadShaderSuccess = RT_OK;
    
    const char* vtxCode = mVertexSrc.length() > 0 ? (const char*) mVertexSrc.data() : vShaderText;
    
    // TODO:  TRY
    shaderProgram::initShader( vtxCode, (const char*) mFragmentSrc.data() );
    // TODO:  CATCH
    
    double stopDecodeTime = pxMilliseconds();
    setLoadStatus("decodeTimeMs", static_cast<int>(stopDecodeTime-startDecodeTime));
  }
  else
  {
    loadShaderSuccess = RT_RESOURCE_NOT_FOUND;
    rtLogError("Could not load shader file %s.", mFragmentUrl.cString());
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if ( loadShaderSuccess != RT_OK)
  {
    rtLogWarn("shader load failed"); // TODO: why?
    if (loadShaderSuccess == RT_RESOURCE_NOT_FOUND)
    {
      setLoadStatus("statusCode",PX_RESOURCE_STATUS_FILE_NOT_FOUND);
    }
    else
    {
      setLoadStatus("statusCode", PX_RESOURCE_STATUS_DECODE_FAILURE);
    }
    
    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();
    
    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void*)"reject");
    }
    //mTexture->notifyListeners( mTexture, RT_FAIL, errorCode);
  }
  else
  {
    mFragmentSrc.term(); // Dump the source data...
    mVertexSrc.term();   // Dump the source data...
    
    setLoadStatus("statusCode",0);
    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();
    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void *) "resolve");
    }
  }
}

/**
 * rtShaderResource::loadResource()
 *
 * This method will actually start the download of the image for mUrl.
 *
 * ImageManager calls this method when the url is not already present
 * in the cache map.
 *
 * */
void rtShaderResource::loadResource(rtObjectRef archive, bool reloading)
{
  if(!reloading && ((rtPromise*)mReady.getPtr())->status())
  {
    //create a new promise if the old one is complete
    mReady = new rtPromise();
  }
  setLoadStatus("statusCode", -1);
  pxArchive* arc = (pxArchive*)archive.getPtr();
  if (mArchive != arc)
  {
    mArchive = arc;
  }
  
  bool isFrgURL = (mFragmentUrl.beginsWith("http:") || mFragmentUrl.beginsWith("https:") );
  bool isVtxURL = (  mVertexUrl.beginsWith("http:") ||   mVertexUrl.beginsWith("https:") );

  bool isFrgDataURL = mFragmentUrl.beginsWith("data:text/plain,");
  //bool isVtxDataURL =   mVertexUrl.beginsWith("data:text/plain,");

  // FRAGMENT SHADER
  if( isFrgURL && mFragmentSrc.length() == 0)
  {
    rtFileDownloadRequest* fragRequest;
    
    setLoadStatus("sourceType", "http");
    fragRequest = new rtFileDownloadRequest(mFragmentUrl, this, pxResource::onDownloadComplete);
    fragRequest->setProxy(mProxy);
    fragRequest->setTag("frg");
    
    fragRequest->setCallbackFunctionThreadSafe(pxResource::onDownloadComplete);
#ifdef ENABLE_CORS_FOR_RESOURCES
    fragRequest->setCORS(mCORS);
#endif
    mDownloadInProgressMutex.lock();
    mDownloadInProgress = true;
    mDownloadInProgressMutex.unlock();
    AddRef(); //ensure this object is not deleted while downloading
    rtFileDownloader::instance()->addToDownloadQueue(fragRequest);
    
    return;
  }

  // VERTEX SHADER
  if (isVtxURL && mVertexSrc.length() == 0)
  {
    rtFileDownloadRequest* vtxRequest;

    setLoadStatus("sourceType", "http");
    vtxRequest = new rtFileDownloadRequest(mVertexUrl, this, pxResource::onDownloadComplete);
    vtxRequest->setProxy(mProxy);
    vtxRequest->setTag("vtx");

    vtxRequest->setCallbackFunctionThreadSafe(pxResource::onDownloadComplete);
#ifdef ENABLE_CORS_FOR_RESOURCES
    fragRequest->setCORS(mCORS);
#endif
    mDownloadInProgressMutex.lock();
    mDownloadInProgress = true;
    mDownloadInProgressMutex.unlock();
    AddRef(); //ensure this object is not deleted while downloading
    rtFileDownloader::instance()->addToDownloadQueue(vtxRequest);
    
    return;
  }
  else if (isFrgDataURL)
  {
    setLoadStatus("sourceType", "dataurl");
    double startResourceSetupTime = pxMilliseconds();
    loadResourceFromArchive(arc);
    double stopResourceSetupTime = pxMilliseconds();
    setLoadStatus("setupTimeMs", static_cast<int>(stopResourceSetupTime-startResourceSetupTime));
  }
  else if ((arc != NULL ) && (arc->isFile() == false))
  {
    setLoadStatus("sourceType", "archive");
    double startResourceSetupTime = pxMilliseconds();
    loadResourceFromArchive(arc);
    double stopResourceSetupTime = pxMilliseconds();
    setLoadStatus("setupTimeMs", static_cast<int>(stopResourceSetupTime-startResourceSetupTime));
  }
  else
  {
    setLoadStatus("sourceType", "file");
    double startResourceSetupTime = pxMilliseconds();
    loadResourceFromFile();
    double stopResourceSetupTime = pxMilliseconds();
    setLoadStatus("setupTimeMs", static_cast<int>(stopResourceSetupTime-startResourceSetupTime));
  }
}

uint32_t rtShaderResource::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
  double startDecodeTime = pxMilliseconds();
  rtError decodeResult = RT_OK; // TODO: create shaderProgram   //pxLoadImage(fileDownloadRequest->downloadedData(),
                                                                //            fileDownloadRequest->downloadedDataSize(),
                                                                //            imageOffscreen, init_w, init_h, init_sx, init_sy)
  if(fileDownloadRequest->tag() == "frg")
  {
    mFragmentSrc.init( (const uint8_t*) fileDownloadRequest->downloadedData(),
                                        fileDownloadRequest->downloadedDataSize());
  }
  else
  if(fileDownloadRequest->tag() == "vtx")
  {
    mVertexSrc.init( (const uint8_t*) fileDownloadRequest->downloadedData(),
                                      fileDownloadRequest->downloadedDataSize());
  }
  
  double stopDecodeTime = pxMilliseconds();
  if (decodeResult == RT_OK)
  {
    setLoadStatus("decodeTimeMs", static_cast<int>(stopDecodeTime-startDecodeTime));

#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
    return PX_RESOURCE_LOAD_WAIT;
#else
    return PX_RESOURCE_LOAD_SUCCESS;
#endif  //ENABLE_BACKGROUND_TEXTURE_CREATION
  }
  
  return PX_RESOURCE_LOAD_FAIL;
}

void rtShaderResource::prelink()
{
  mPosLoc = 0;
  mUVLoc = 1;
  glBindAttribLocation(mProgram, mPosLoc, "pos");
  glBindAttribLocation(mProgram, mUVLoc,  "uv");
}

void rtShaderResource::postlink()
{
  mTimeLoc       = getUniformLocation("u_time");
  mResolutionLoc = getUniformLocation("u_resolution");
  mMatrixLoc     = getUniformLocation("amymatrix");
  
  if(mUniform_map.size() > 0)
  {
    for (UniformMapIter_t it  = mUniform_map.begin();
                          it != mUniform_map.end(); ++it)
    {
      (*it).second.loc = getUniformLocation( (*it).second.name );
    }
  }//ENDIF
}

//=====================================================================================================================================
//
// GL Setters
//
/*static*/ rtError rtShaderResource::fillInt32Vec(int32_t *vec, const rtArrayObject* vals)
{
  if(vals)
  {
    rtValue count;                  // HACK - WORKAROUND
    vals->Get("length", &count);    // HACK - WORKAROUND
    
    int len = count.toUInt32();
    int  ii = 0;
    
    for (uint32_t i = 0, l = len; i < l; ++i)
    {
      rtValue vv;
      if(vals->Get(i, &vv) == RT_OK && !vv.isEmpty())
      {
        vec[ii++] = vv.toInt32();
      }
    }//FOR
    
    return RT_OK;
  }
  
  return RT_FAIL;
}

/*static*/ rtError rtShaderResource::fillFloatVec(float *vec, const rtArrayObject* vals)
{
  if(vals && vec)
  {
    rtValue count;                  // HACK - WORKAROUND
    vals->Get("length", &count);    // HACK - WORKAROUND
    
    int len = count.toUInt32();
    int ii  = 0;
    
    for (uint32_t i = 0, l = len; i < l; ++i)
    {
      rtValue vv;
      if(vals->Get(i, &vv) == RT_OK && !vv.isEmpty())
      {
        vec[ii++] = vv.toFloat();
      }
    }//FOR
    
    return RT_OK;
  }
  
  return RT_FAIL;
}

// sampler
/*static*/ void rtShaderResource::bindTexture3(const uniformLoc_t &p)
{
  rtImageResource *img = (rtImageResource *) p.value.toObject().getPtr();
  
  if(img)
  {
    pxTextureRef tex = img->getTexture();
    GLuint       tid = tex->getNativeId();
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D,     tid);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glUniform1i(p.loc, 3);
  }
}

// sampler
/*static*/ void rtShaderResource::bindTexture4(const uniformLoc_t &p)
{
  rtImageResource *img = (rtImageResource *) p.value.toObject().getPtr();
  
  if(img)
  {
    pxTextureRef tex = img->getTexture();
    GLuint       tid = tex->getNativeId();
    
    tex->bindTexture();
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,     tid);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glUniform1i(p.loc, 4);
  }
}

// sampler
/*static*/ void rtShaderResource::bindTexture5(const uniformLoc_t &p)
{
  rtImageResource *img = (rtImageResource *) p.value.toObject().getPtr();
  
  if(img)
  {
    pxTextureRef tex = img->getTexture();
    GLuint       tid = tex->getNativeId();
    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D,     tid);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glUniform1i(p.loc, 5);
  }
}

/*static*/ void rtShaderResource::setUniformNiv(int N, const uniformLoc_t &p)
{
  const rtArrayObject* vals = (const rtArrayObject*)  p.value.toObject().getPtr();
  
  if(vals)
  {
    static int32_t vec[4] = {0}; // up to vec4

    switch(N)
    {
      case 2: glUniform2iv(p.loc, 1, vec ); break;
      case 3: glUniform3iv(p.loc, 1, vec ); break;
      case 4: glUniform4iv(p.loc, 1, vec ); break;
    }
  }
}

/*static*/ void rtShaderResource::setUniformNfv(int N, const uniformLoc_t &p)
{
  const rtArrayObject* vals = (const rtArrayObject*)  p.value.toObject().getPtr();
  
  if(vals)
  {
    static float vec[4] = {0.0}; // up to vec4

    fillFloatVec(&vec[0], vals);
    
    switch(N)
    {
      case 2: glUniform2fv(p.loc, 1, vec ); break;
      case 3: glUniform3fv(p.loc, 1, vec ); break;
      case 4: glUniform4fv(p.loc, 1, vec ); break;
    }
  }
}

rtDefineObject(  rtShaderResource, pxResource);
rtDefineProperty(rtShaderResource, uniforms);

static int gShaderId = 0;

/**********************************************************************/
/**                    pxShaderManager                                */
/**********************************************************************/
ShaderMap    pxShaderManager::mShaderMap;     //  ID to SHADER
ShaderUrlMap pxShaderManager::mShaderUrlMap;  // URL to ID

rtRef<rtShaderResource> pxShaderManager::getShader(const char* fragmentUrl,
                                                   const char* vertexUrl /* = NULL*/,
                                                   const rtCORSRef& cors, rtObjectRef archive)
{
  rtRef<rtShaderResource> pShader;
  uint32_t shaderId;
  
  rtString key = fragmentUrl;
  if (false == ((key.beginsWith("http:")) || (key.beginsWith("https:"))))
  {
    pxArchive* arc = (pxArchive*)archive.getPtr();
    if (NULL != arc)
    {
      if (false == arc->isFile())
      {
        rtString data = arc->getName();
        data.append("_");
        data.append(fragmentUrl);
        key = data;
      }
    }
  }

  // Assign shader urls an id number if they don't have one
  ShaderUrlMap::iterator itId = mShaderUrlMap.find(key);
  if( itId != mShaderUrlMap.end())
  {
    shaderId = itId->second;
  }
  else
  {
    shaderId = gShaderId++;
    mShaderUrlMap.insert(make_pair(key, shaderId));
    size_t count = mShaderUrlMap.size(); // JUNK
    count += 0;
  }
  
  ShaderMap::iterator it = mShaderMap.find(shaderId);
  if (it != mShaderMap.end())
  {
    rtLogDebug("Found rtShaderResource in map for %s\n",fragmentUrl);
    pShader = it->second;
    return pShader;
  }
  else
  {
    rtLogDebug("Create rtShaderResource in map for %s\n",fragmentUrl);
    pShader = new rtShaderResource(fragmentUrl, vertexUrl);
    pShader->setCORS(cors);
    mShaderMap.insert(make_pair(shaderId, pShader));
    pShader->loadResource(archive);
  }
  
  return pShader;
}

void pxShaderManager::removeShader(rtString shaderUrl)
{
  if(shaderUrl.isEmpty())
  {
    rtLogDebug("removeShader( <empty> ) - NOT found ! \n");
    return;
  }
  
  ShaderUrlMap::iterator it = mShaderUrlMap.find(shaderUrl);
  
  if( it != mShaderUrlMap.end())
  {
    removeShader( (uint32_t) it->second);
    mShaderUrlMap.erase(it);
  }
  else
  {
     rtLogDebug("removeShader(%s) - NOT found ! \n",shaderUrl.cString());
  }
}

void pxShaderManager::removeShader(uint32_t shaderId)
{
  ShaderMap::iterator it = mShaderMap.find(shaderId);
  if (it != mShaderMap.end())
  {
    mShaderMap.erase(it);
  }
  else
  {
    rtLogDebug("removeShader(id: %d) - NOT found ! \n",shaderId);
  }
}

void pxShaderManager::clearAllShaders()
{
  mShaderUrlMap.clear();
}

//=====================================================================================================================================
//=====================================================================================================================================

rtError findKey(rtArrayObject* array, rtString k)
{
  if(array)
  {
    uint32_t len = array->get<uint32_t>("length");
    rtValue element;
    
    for (uint32_t i = 0, l = len; i < l; ++i)
    {
      if (array->Get(i, &element) == RT_OK && !element.isEmpty() )
      {
        if(k == element.toString())
        {
          return RT_OK;
        }
      }
    }//FOR
  }
  
  return RT_FAIL;
}

//=====================================================================================================================================
//
// Set a (JS) CONFIG object directly
//
rtError setShaderConfig(rtObjectRef v, pxObject &obj)
{
  if(v)
  {
    //
    // >>> GET >> "name"
    //
    rtValue nameVal;
    v->Get("name", &nameVal);
    
    //
    // >>> GET >> "shader"
    //
    rtValue shaderVal;
    v->Get("shader", &shaderVal);
    
    if( shaderVal.isEmpty() )
    {
      rtLogError("ERROR:  Unable to parse 'shader' for shaderResource.");
      return RT_FAIL;
    }
    
    rtObjectRef      shaderRef = shaderVal.toObject();
    rtShaderResource *shader   = (shaderRef) ? (rtShaderResource *) shaderRef.getPtr() : NULL;
    
    //
    // >>> GET >> "uniforms"
    //
    rtValue uniformsVal;
    v->Get("uniforms", &uniformsVal);
    
    if(uniformsVal.isEmpty() != true)
    {
      rtObjectRef uniforms = uniformsVal.toObject();
      rtValue count;                          // HACK - WORKAROUND
      uniforms->Get("length", &count);        // HACK - WORKAROUND
      
      if(!shader)
      {
        rtLogError("ERROR:  Unable to parse 'uniforms' for shaderResource.");
        return RT_FAIL;
      }
      else
      {
        obj.setEffectConfig(shaderRef);
        shader->setUniformVals(uniformsVal); // WAS uniforms ????? FIXME
      }
    }
    
    return RT_OK;
  }
  
  rtLogError("ERROR:  Unable to set Config for shaderResource.");
  return RT_FAIL;
}

//=====================================================================================================================================
//
// Set an ARRAY of (RT) CONFIG objects of UNIFORMS to Shader
//
rtError applyConfigArray(rtObjectRef v, pxObject &obj)
{
  rtArrayObject *array = dynamic_cast<rtArrayObject *>(v.getPtr());
  
  if(array)
  {
    uint32_t len = array->length();
    
    // Iterate CONFIG objects in Array
    for (uint32_t i = 0; i < len; i++)
    {
      rtValue config = array->get<rtValue>(i);
      
      // Unwrap the Config object
      rtObjectRef configRef = config.toObject();
      
      applyConfig(configRef, obj);
    }
    
    return RT_OK;
  }
  
  return RT_FAIL;
}

//=====================================================================================================================================
//
// Set an (RT) CONFIG object of UNIFORMS to Shader
//
rtError applyConfig(rtObjectRef v, pxObject &obj)
{
  rtMapObject *mapp = dynamic_cast<rtMapObject *>(v.getPtr());
  
  rtObjectRef keys;
  
  if(mapp)
  {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // NAME
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    rtValue name;
    mapp->Get("name", &name);
    
    if(name.isEmpty() == true)
    {
      rtLogDebug("APPLY name:  %s \n", name.toString().cString());
      return RT_FAIL;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // SHADER
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    rtValue shaderVal;
    mapp->Get("shader", &shaderVal);
    
    if(shaderVal.isEmpty() == true)
    {
      rtLogError("ERROR:  No 'shader' specified. ");
      return RT_FAIL;
    }
    else
    {
      obj.setEffectPtr(  (rtShaderResource *) ( shaderVal.toVoidPtr() ) );
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // UNIFORMS
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    rtValue uniformsVal;
    mapp->Get("uniforms", &uniformsVal);
    
    if(uniformsVal.isEmpty() == false)
    {
      rtMapObject *uniforms = dynamic_cast<rtMapObject *>(uniformsVal.toObject().getPtr());
      
      if(uniforms)
      {
        rtObjectRef keys = uniforms->get<rtObjectRef>("allKeys");
        uint32_t     len = keys.get<uint32_t>("length");
        
        // Iterate UNIFORMS map
        for (uint32_t i = 0; i < len; i++)
        {
          rtString key = keys.get<rtString>(i);
          rtValue  val = uniforms->get<rtValue>(key);
          
          rtShaderResource *shader = obj.effectPtr();
          if(shader)
          {
            shader->setUniformVal(key, val);
          }
        }//FOR
      }
    }//ENDIF
  }
  
  return RT_OK;
}

//=====================================================================================================================================
//
// Copy  (JS) CONFIG objects to (RT) Config objects ... array
//
rtObjectRef copyShaderConfigs(rtObjectRef &v)
{
  rtObjectRef keys = v.get<rtObjectRef>("allKeys");
  uint32_t numKeys = keys.get<uint32_t>("length");
  
  if(numKeys < 1)
  {
    rtLogError("ERROR:  Not an array object");
    return NULL;
  }
  
  rtObjectRef dstConfigs = new rtArrayObject();
  
  rtShaderResource *pShader = NULL;
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // CONFIG
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  // Iterate CONFIG objects in Array
  for (uint32_t i = 0; i < numKeys; i++)
  {
    rtString            key = keys.get<rtString>(i);
    rtValue          config = v.get<rtValue>(key);
    rtObjectRef   configRef = config.toObject(); // Unwrap the Config object
    
    rtObjectRef thisConfig = new rtMapObject();
    
    //    printf("\n\nGOT >> CONFIG[%d]  >>  %s\n",i, thisConfig.getTypeStr());
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // NAME
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    rtValue name = configRef.get<rtValue>("name");
    if(name.isEmpty() == false)
    {
      rtLogDebug("GOT name:  %s \n", name.toString().cString());
      
      //printf("COPY NAME >>> %s\n",  name.toString().cString());
      
      thisConfig->Set("name", &name);
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // SHADER
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    rtValue  shader = configRef.get<rtValue>("shader");
    
    if(shader.isEmpty() == false)
    {
      rtShaderResource *ptr = dynamic_cast<rtShaderResource *>( shader.toObject().getPtr() );
      
      pShader = ptr;
      
      rtValue ptrVal;
      
      ptrVal.setVoidPtr(ptr);
      thisConfig->Set("shader", &ptrVal);
    }
    else
    {
      rtLogError("ERROR:  No 'shader' specified. ");
      return NULL;
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // UNIFORMS ARRAY
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    rtValue uniformsValue = configRef.get<rtValue>("uniforms");
    
    if(uniformsValue.isEmpty())
    {
      rtLogError("ERROR:  No 'uniforms' specified. ");
      return NULL;
    }
    else
    {
      rtObjectRef dstUniforms = new rtMapObject();
      
      rtValue uniforms;
      uniforms.setObject(dstUniforms);
      
      thisConfig->Set("uniforms", &uniforms);
      
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // Iterate UNIFORMS maps in Array
      //
      rtObjectRef  uniformsObject = uniformsValue.toObject();
      rtObjectRef            keys = uniformsObject.get<rtObjectRef>("allKeys");
      uint32_t            numKeys = keys.get<uint32_t>("length");
      
      for (uint32_t i = 0; i < numKeys; i++)
      {
        rtString key = keys.get<rtString>(i);
        rtValue  val = uniformsObject.get<rtValue>(key);
        
        UniformType_t type = pShader->getUniformType(key);
        
        rtValue uniform = copyUniform(type, val);
        
        dstUniforms->Set(key.cString(), &uniform); // add UNIFORM to 'uniforms' map
        
        //printf("\nCOPY UNIFORM >>> key:%15s  val: %15s", key.cString(), uniform.getTypeStr());
      }//FOR - uniforms
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      rtValue val;
      val.setObject(thisConfig);
      
      ((rtArrayObject*) dstConfigs.getPtr())->pushBack(val);
      
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    }
  }//FOR - config objects
  
  return rtObjectRef(dstConfigs);
}

//=====================================================================================================================================
//
// // helper to copy "variant" like UNIFORM types
//
rtValue copyUniform(UniformType_t type, rtValue &val)
{
  rtValue ans;
  
  switch(type)
  {
      case UniformType_Unknown:
      ans.setEmpty();
      break;
      
      case UniformType_Bool:
      case UniformType_Int:
      case UniformType_UInt:
      case UniformType_Float:
      case UniformType_Double:
      //printf("\nCOPY UNIFORM >>> scalar (float/int/bool) ");
      ans.setValue(val);
      return ans;
      break;
      
      // Vectors:  int
      case UniformType_iVec2:
      case UniformType_iVec3:
      case UniformType_iVec4:
      
      // TODO: Possibly not in ES 2.0
      //
      //      // Vectors:  bool
      //    case UniformType_bVec2:
      //    case UniformType_bVec3:
      //    case UniformType_bVec4:
      //
      //      // Vectors:  uint
      //    case UniformType_uVec2:
      //    case UniformType_uVec3:
      //    case UniformType_uVec4:
      //
      //
      //      // Vectors:  double
      //    case UniformType_dVec2:
      //    case UniformType_dVec3:
      //    case UniformType_dVec4:
      
      // Vectors:  float
      case UniformType_Vec2:
      case UniformType_Vec3:
      case UniformType_Vec4:
    {
      //printf(" \nCOPY UNIFORM >>> vector (vec2/vec3/vec4)");
      
      rtObjectRef    array = new rtArrayObject();
      rtObjectRef   valObj = val.toObject();
      rtObjectRef     keys = valObj.get<rtObjectRef>("allKeys");
      
      if (keys)
      {
        uint32_t len = keys.get<uint32_t>("length");
        for (uint32_t i = 0; i < len; i++)
        {
          rtValue  val = valObj.get<rtValue>(i);
          
          // JS only has Doubles.  Convert to Floats.
          if(val.getType() == RT_doubleType)
          {
            //printf("\n  COPY UNIFORM >>> vec%d  i: %d   val: %f", len, i, (float) val.toDouble());
            
            val.setFloat( (float) val.toDouble() );
          }
          
          ((rtArrayObject*) array.getPtr())->pushBack(val);
        }
      }
      
      ans.setObject(array);
      
      return ans;
    }
      break;
      
      case UniformType_Sampler2D:
      case UniformType_Struct:
      break;
  }//SWITCH
  
  return ans;
}
//=====================================================================================================================================
