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

#include "rtThreadQueue.h"
#include "rtFileDownloader.h"
#include "rtString.h"
#include "rtRef.h"

#include "rtPathUtils.h"
#include "pxTimer.h"

#include "pxContext.h"

#include "pxScene2d.h"
#include "pxShaderResource.h"


using namespace std;

extern const char* vShaderText;
//extern int currentGLProgram;

//=====================================================================================================================================

rtShaderResource::rtShaderResource()
: pxResource(), mIsRealTime(false), mNeedsFbo(false), mPasses(1), mSamplerCount(3)
{
  mFragmentUrl = NULL;
  mVertexUrl   = NULL;
}

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

    if(shaderProgram::initShader( vtxCode, (const char*) mFragmentSrc.data() ) != RT_OK)
    {
        rtLogError("FATAL: Shader error: %s \n", mCompilation.cString() );
  
        setLoadStatus("glError", mCompilation.cString() );
  
        gUIThreadQueue->addTask(onDownloadCanceledUI, this, (void*)"reject");
      
        rtValue nullValue;
        mReady.send("reject", nullValue );
    }

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
  UniformMapIter_t it = mUniform_map.find(name);
  if(it != mUniform_map.end())
  {
    use();

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

  if(arr == NULL || uniforms == NULL)
  {
    rtLogError(" Empty/Bad Uniforms array.");
    return RT_FAIL;
  }

  rtValue keysVal;
  uniforms->Get("allKeys", &keysVal);

  if( keysVal.isEmpty() )
  {
    rtLogError(" Empty Uniforms array.");
    return RT_FAIL;
  }

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
    else
    {
      rtLogError(" Failed to set Uniform - key: %s", key.cString() );
      return RT_FAIL;
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
        setFunc  = rtShaderResource::setUniform1i;
        typeEnum = UniformType_Int;
      }
      else
      if(type == "float")
      {
        setFunc  = rtShaderResource::setUniform1f;
        typeEnum = UniformType_Float;
      }
      else
      if(type == "ivec2")
      {
        setFunc  = rtShaderResource::setUniform2iv;
        typeEnum = UniformType_iVec2;
      }
      else
      if(type == "ivec3")
      {
        setFunc  = rtShaderResource::setUniform3iv;
        typeEnum = UniformType_iVec3;
      }
      else
      if(type == "ivec4")
      {
        setFunc  = rtShaderResource::setUniform4iv;
        typeEnum = UniformType_iVec4;
      }
      else
      if(type == "vec2")
      {
        setFunc  = rtShaderResource::setUniform2fv;
        typeEnum = UniformType_Vec2;
      }
      else
      if(type == "vec3")
      {
        setFunc  = rtShaderResource::setUniform3fv;
        typeEnum = UniformType_Vec3;
      }
      else
      if(type == "vec4")
      {
        setFunc  = rtShaderResource::setUniform4fv;
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

      mUniform_map[name] = { name, typeEnum, (GLint) -1, rtValue(), false, setFunc };  // ADD TO MAP
    }
  }//FOR

  return RT_OK;
}

void rtShaderResource::loadResourceFromFile()
{
  init();

  // Since this object can be released before we get a async completion
  // We need to maintain this object's lifetime
  // TODO review overall flow and organization
  AddRef();

  rtError loadFrgShader = RT_FAIL;

  if(mVertexUrl.beginsWith("data:text/plain,"))
  {
    mVertexSrc.init( (const uint8_t* ) mVertexUrl.substring(16).cString(),
                                       mVertexUrl.byteLength() - 16);
  }
  if(mFragmentUrl.beginsWith("data:text/plain,"))
  {
    mFragmentSrc.init( (const uint8_t* ) mFragmentUrl.substring(16).cString(),
                                         mFragmentUrl.byteLength() - 16);
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

//    // Since this object can be released before we get a async completion
//    // We need to maintain this object's lifetime
//    // TODO review overall flow and organization
//    AddRef();

    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCanceledUI, this, (void*)"reject");
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

void rtShaderResource::loadResourceFromArchive(rtObjectRef /*archiveRef*/)
{
#if 0
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

    if(shaderProgram::initShader( vtxCode, (const char*) mFragmentSrc.data() ) != RT_OK)
    {
      rtLogError("FATAL: Shader error: %s \n", mCompilation.cString() );
      
      setLoadStatus("glError", mCompilation.cString() );
      
      gUIThreadQueue->addTask(onDownloadCanceledUI, this, (void*)"reject");
    }

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
      gUIThreadQueue->addTask(onDownloadCanceledUI, this, (void*)"reject");
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
#endif // 0
}

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

    loadResourceFromFile(); // Detect and Load shaders from URL directly

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
  if(vals && vec)
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
/*static*/ rtError rtShaderResource::bindTextureN(GLuint n, const uniformLoc_t &p)
{
  rtImageResource *img = (rtImageResource *) p.value.toObject().getPtr();

  if(img)
  {
    pxTextureRef tex = img->getTexture();
    
    if(tex)
    {
      GLuint tid = tex->getNativeId();

      GLenum texN;
      
      switch(n)
      {
        case 3: texN = GL_TEXTURE3; break;
        case 4: texN = GL_TEXTURE4; break;
        case 5: texN = GL_TEXTURE5; break;
        default:
          rtLogError("Invalid texture ID: %d  (Not 3,4 or 5) ", n);
          return RT_FAIL;
      }

      glActiveTexture( texN );
      
      glBindTexture(GL_TEXTURE_2D,     tid);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glUniform1i(p.loc, n);

      return RT_OK;
    }
  }

  return RT_FAIL;
}

/*static*/ rtError rtShaderResource::setUniformNiv(int N, const uniformLoc_t &p)
{
  const rtArrayObject* vals = (const rtArrayObject*)  p.value.toObject().getPtr();

  if(vals)
  {
    static int32_t vec[4] = {0}; // up to vec4

    fillInt32Vec(&vec[0], vals);

    switch(N)
    {
      case 2: glUniform2iv(p.loc, 1, vec ); break;
      case 3: glUniform3iv(p.loc, 1, vec ); break;
      case 4: glUniform4iv(p.loc, 1, vec ); break;
    }

    return RT_OK;
  }

  return RT_FAIL;
}

/*static*/ rtError rtShaderResource::setUniformNfv(int N, const uniformLoc_t &p)
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

    return RT_OK;
  }

  return RT_FAIL;
}

rtDefineObject(  rtShaderResource, pxResource);
rtDefineProperty(rtShaderResource, uniforms);

static int gShaderId = 0;

/**********************************************************************/
/**                    pxShaderManager                                */
/**********************************************************************/
ShaderMap    pxShaderManager::mShaderMap;     //  ID to SHADER
ShaderUrlMap pxShaderManager::mShaderUrlMap;  // URL to ID

pxShaderResourceRef pxShaderManager::getShader(const char* fragmentUrl,
                                               const char* vertexUrl /* = NULL*/,
                                               const rtCORSRef& cors, rtObjectRef archive)
{
  pxShaderResourceRef shaderRef;
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
  }

  ShaderMap::iterator it = mShaderMap.find(shaderId);
  if (it != mShaderMap.end())
  {
    rtLogDebug("Found rtShaderResource in map for %s\n",fragmentUrl);
    shaderRef = it->second;
    return shaderRef;
  }
  else
  {
    rtLogDebug("Create rtShaderResource in map for %s\n",fragmentUrl);
    shaderRef = new rtShaderResource(fragmentUrl, vertexUrl);
    shaderRef->setCORS(cors);
    mShaderMap.insert(make_pair(shaderId, shaderRef));
    shaderRef->loadResource(archive);
  }

  return shaderRef;
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
// Set a (JS) CONFIG object + UNIFORMS to Shader directly
//
rtError applyJSconfig(rtObjectRef v, pxObject &obj)
{
  if(v)
  {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // (JS)  NAME
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    rtValue nameVal;
    v->Get("name", &nameVal);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // (JS)  SHADER
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    rtValue shaderVal;
    v->Get("shader", &shaderVal);

    if( shaderVal.isEmpty() )
    {
      rtLogError("ERROR:  Unable to parse 'shader' for shaderResource.");
      return RT_FAIL;
    }

    rtObjectRef      shaderRef = shaderVal.toObject();
    rtShaderResource *shader   = static_cast<rtShaderResource *>( (shaderRef) ? shaderRef.getPtr() : NULL);

    if( shader == NULL )
    {
      rtLogError("ERROR:  Bad shader object");
      return RT_FAIL;
    }

    obj.setEffectConfig(shaderRef);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // (JS)  UNIFORMS
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    rtValue uniformsVal;
    v->Get("uniforms", &uniformsVal);

    if(uniformsVal.isEmpty())
    {
      rtLogError("ERROR:  Unable to parse 'uniforms' for shaderResource.");
      return RT_FAIL;
    }

    shader->setUniformVals(uniformsVal);

    return RT_OK;
  }

  rtLogError("ERROR:  Unable to set Config for shaderResource.");
  return RT_FAIL;
}

//=====================================================================================================================================
//
// Set an (RT) CONFIG object + UNIFORMS to Shader
//
rtError applyRTconfig(rtObjectRef v, pxObject &obj)
{
  rtMapObject *mapp = dynamic_cast<rtMapObject *>(v.getPtr());

  if(mapp)
  {
    rtObjectRef keys;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // (RT)  NAME
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    rtValue name;
    mapp->Get("name", &name);

    if(name.isEmpty() == true)
    {
      rtLogDebug("APPLY name:  %s \n", name.toString().cString());
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // (RT)  SHADER
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    rtValue shaderVal;
    mapp->Get("shader", &shaderVal);

    if(shaderVal.isEmpty() == true)
    {
      rtLogError(" - No 'shader' specified. ");
      return RT_FAIL;
    }

    rtShaderResource *shader = static_cast<rtShaderResource *>( shaderVal.toVoidPtr() );
    obj.setEffectPtr( (rtShaderResource *) ( shader ) );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // (RT)  UNIFORMS
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    rtValue uniformsVal;
    mapp->Get("uniforms", &uniformsVal);

    if(uniformsVal.isEmpty() )
    {
     // rtLogDebug(" - \"uniforms\" list is empty/missing. ");
      return RT_OK;
    }

    rtMapObject *uniforms = dynamic_cast<rtMapObject *>(uniformsVal.toObject().getPtr());

    if(uniforms && shader)
    {
      rtObjectRef keys = uniforms->get<rtObjectRef>("allKeys");
      uint32_t     len = keys.get<uint32_t>("length");

      // Iterate UNIFORMS map
      for (uint32_t i = 0; i < len; i++)
      {
        rtString key = keys.get<rtString>(i);
        rtValue  val = uniforms->get<rtValue>(key);

        shader->setUniformVal(key, val);
      }//FOR
    }

    return RT_OK;
  }

  rtLogWarn(" - applyConfig() - bad args ");

  return RT_FAIL; // default fail
}

//=====================================================================================================================================
//
//  Copy  (JS) CONFIG object to (RT) Config object ... single
//
rtObjectRef copyConfigJS2RT(rtObjectRef &v)
{
  rtShaderResource *pShader = NULL;

  rtObjectRef configRef = v;// v.toObject(); // Unwrap the Config object

  if(configRef == NULL)
  {
    rtLogError("ERROR:  Invlaid 'config' specified. ");
    return NULL;
  }

  rtObjectRef thisConfig = new rtMapObject();

  // printf("\n\nGOT >> CONFIG[%d]  >>  %s\n",i, thisConfig.getTypeStr());

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // (JS to RT) NAME
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  rtValue name = configRef.get<rtValue>("name");
  if(name.isEmpty() == false)
  {
    rtLogDebug("GOT name:  %s \n", name.toString().cString());

    thisConfig->Set("name", &name);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // (JS to RT) SHADER
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  rtValue  shader = configRef.get<rtValue>("shader");

  if(shader.isEmpty() == false)
  {
    rtShaderResource *ptr = dynamic_cast<rtShaderResource *>( shader.toObject().getPtr() );

    pShader = ptr; // needed below...

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
  // (JS to RT) UNIFORMS ARRAY
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  rtValue uniformsValue = configRef.get<rtValue>("uniforms");

  if(uniformsValue.isEmpty())
  {
    rtLogWarn("NOTE:  No 'uniforms' specified. ");
  }
  else
  if(pShader == NULL)
  {
    rtLogError("ERROR:  No 'shader' specified. ");
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
  }

  return thisConfig;
}

//=====================================================================================================================================
//
// Copy  (JS) CONFIG objects to (RT) Config objects ... array
//
rtObjectRef copyArrayJS2RT(rtObjectRef &v)
{
  rtObjectRef keys = v.get<rtObjectRef>("allKeys");
  uint32_t numKeys = keys.get<uint32_t>("length");

  if(numKeys < 1)
  {
    rtLogError("ERROR:  Not an array object");
    return NULL;
  }

  rtObjectRef dstConfigs = new rtArrayObject();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // (JS to RT) CONFIG
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // Iterate CONFIG objects in Array
  for (uint32_t i = 0; i < numKeys; i++)
  {
    rtString            key = keys.get<rtString>(i);
    rtValue          config = v.get<rtValue>(key);
    rtObjectRef   configRef = config.toObject(); // Unwrap the Config object

    rtObjectRef thisConfig = copyConfigJS2RT(configRef);

    rtValue val;
    val.setObject(thisConfig);

    ((rtArrayObject*) dstConfigs.getPtr())->pushBack(val);

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
          // JS only has Doubles.  Cast to Int.
          ans.setValue((int) val.toDouble());
          return ans;
      break;

      case UniformType_UInt:
          // JS only has Doubles.  Cast to UInt.
          ans.setValue( (uint32_t) val.toDouble());
          return ans;
      break;

      case UniformType_Float:
      case UniformType_Double:
          // JS only has Doubles.  Cast to Float.
          ans.setValue((float) val.toDouble() );
          return ans;
      break;

      // Vectors:  int
      case UniformType_iVec2:
      case UniformType_iVec3:
      case UniformType_iVec4:
      {
        rtObjectRef    array = new rtArrayObject();
        rtObjectRef   valObj = val.toObject();
        rtObjectRef     keys = valObj.get<rtObjectRef>("allKeys");

        if (keys)
        {
          uint32_t len = keys.get<uint32_t>("length");
          for (uint32_t i = 0; i < len; i++)
          {
            rtValue  val = valObj.get<rtValue>(i);

            // JS only has Doubles.  Cast to Int.
            val.setInt32( (int32_t) val.toDouble() );

            ((rtArrayObject*) array.getPtr())->pushBack(val);
          }
        }

        ans.setObject(array);

        return ans;
      }
      break;
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
        rtObjectRef valObj = val.toObject();

        if(valObj)
        {
          rtObjectRef keys = valObj.get<rtObjectRef>("allKeys");

          if (keys)
          {
            rtObjectRef array = new rtArrayObject();

            uint32_t len = keys.get<uint32_t>("length");
            for (uint32_t i = 0; i < len; i++)
            {
              rtValue val = valObj.get<rtValue>(i);

              // JS only has Doubles.  Cast to Float.
              val.setFloat( (float) val.toDouble() );

              ((rtArrayObject*) array.getPtr())->pushBack(val);

            }//FOR

            ans.setObject(array);
          }
        }

        return ans;
      }
      break;

      case UniformType_Sampler2D:
        ans.setValue(val);
        return ans;
      break;
      case UniformType_Struct:
      break;
  }//SWITCH

  return ans;
}
//=====================================================================================================================================
