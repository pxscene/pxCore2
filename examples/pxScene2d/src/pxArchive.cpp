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

#include "pxArchive.h"
#include "rtPathUtils.h"
#include "rtThreadQueue.h"

extern rtThreadQueue* gUIThreadQueue;

#include "rtFileDownloader.h"

pxArchive::pxArchive(): mIsFile(true),mDownloadRequest(NULL), mZip(),
                        mDownloadStatusCode(0), mHttpStatusCode(0), mArchiveData(NULL), mArchiveDataSize(0),
                        mUseDownloadedData(false), mArchiveDataMutex()
{
}

pxArchive::~pxArchive()
{
  if (mDownloadRequest != NULL)
  {
    //rtLogInfo("pxArchive::~pxArchive(): mDownloadRequest not null\n");
    rtFileDownloader::setCallbackFunctionThreadSafe(mDownloadRequest, NULL, this);
    mDownloadRequest = NULL;
  }
  if (gUIThreadQueue)
  {
    gUIThreadQueue->removeAllTasksForObject(this);
  }
  clearDownloadedData();
}

void pxArchive::clearDownloadedData()
{
  mArchiveDataMutex.lock();
  if (mArchiveData != NULL)
  {
    delete [] mArchiveData;
    mArchiveData = NULL;
  }
  mArchiveDataSize = 0;
  mArchiveDataMutex.unlock();
}

void pxArchive::setupArchive()
{
  mArchiveDataMutex.lock();
  if (mUseDownloadedData)
  {
    mLoadStatus.set("statusCode", mDownloadStatusCode);
    // TODO rtValue doesn't like longs... rtValue and fix downloadRequest
    mLoadStatus.set("httpStatusCode", mHttpStatusCode);
    mLoadStatus.set("errorString", mErrorString);

    if (mDownloadStatusCode == 0) {
      mData.init((uint8_t *) mArchiveData, mArchiveDataSize);
      process(mData.data(), mData.length());
    }
    if (mArchiveData != NULL) {
      delete[] mArchiveData;
      mArchiveData = NULL;
    }
  }
  mArchiveDataMutex.unlock();
}

void pxArchive::setArchiveData(int downloadStatusCode, uint32_t httpStatusCode, const char* data, const size_t dataSize, const rtString& errorString)
{
  mArchiveDataMutex.lock();
  mDownloadStatusCode = downloadStatusCode;
  mHttpStatusCode = httpStatusCode;
  mErrorString = errorString;
  if (mArchiveData != NULL)
  {
    delete [] mArchiveData;
    mArchiveData = NULL;
  }
  if (data == NULL)
  {
    mArchiveData = NULL;
    mArchiveDataSize = 0;
  }
  else
  {
    mArchiveData = new char[dataSize];
    mArchiveDataSize = dataSize;
    memcpy(mArchiveData, data, mArchiveDataSize);
  }
  mArchiveDataMutex.unlock();
}

rtError pxArchive::initFromUrl(const rtString& url, const rtCORSRef& cors, rtObjectRef archive)
{
  mReady = new rtPromise;
  mLoadStatus = new rtMapObject;

  mUrl = url;

  // Since this object can be released before we get a async completion
  // We need to maintain this object's lifetime
  // TODO review overall flow and organization
  AddRef();

  if (url.beginsWith("http:") || url.beginsWith("https:"))
  {
    mLoadStatus.set("sourceType", "http");
    mLoadStatus.set("statusCode", -1);
    mDownloadRequest = new rtFileDownloadRequest(url, this, pxArchive::onDownloadComplete);
    mDownloadRequest->setCORS(cors);
    mDownloadRequest->setCallbackFunctionThreadSafe(pxArchive::onDownloadComplete);
    mUseDownloadedData = true;
    rtFileDownloader::instance()->addToDownloadQueue(mDownloadRequest);
  }
  else
  {
    // Assuming file
    mUseDownloadedData = false;
    mLoadStatus.set("sourceType", "file");
    // TODO align statusCodes for loadStatus
    rtError loadStatus = RT_ERROR;

    do
    {
      pxArchive* arc = (pxArchive*) archive.getPtr();
      if ((arc != NULL ) && (arc->isFile() == false))
      {
        loadStatus = arc->getFileData(mUrl, mData);
        if (loadStatus == RT_OK)
          break;
      }

      loadStatus = rtLoadFile(url, mData);
      if (loadStatus == RT_OK)
        break;

      if (rtIsPathAbsolute(url))
        break;

      rtModuleDirs *dirs = rtModuleDirs::instance();

      for (rtModuleDirs::iter it = dirs->iterator(); it.first != it.second; it.first++)
      {
        loadStatus = rtLoadFile(rtConcatenatePath(*it.first, url.cString()).c_str(), mData);
        if (loadStatus == RT_OK)
          break;
      }
    } while (0);

    mLoadStatus.set("statusCode", loadStatus == RT_OK ? 0 : 1);

    if (loadStatus == RT_OK)
    {
      process(mData.data(), mData.length());
    }

    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(pxArchive::onDownloadCompleteUI,this,NULL);
    }
  }

  return RT_OK;
}

rtError pxArchive::ready(rtObjectRef& r) const
{
  r = mReady;
  return RT_OK;
}

rtError pxArchive::loadStatus(rtObjectRef& v) const
{
  v = mLoadStatus;
  return RT_OK;
}

rtError pxArchive::getFileAsString(const char* fileName, rtString& s)
{
  rtError e = RT_FAIL;
  if (mLoadStatus.get<int32_t>("statusCode") == 0)
  {
    if (mIsFile)
    {
      // Ignore fileName
      s = rtString((const char*)mData.data(),mData.length());
      e = RT_OK;
    }
    else
    {
      rtData d;
      if (mZip.getFileData(fileName,d)==RT_OK)
      {
        s = rtString((const char*)d.data(),d.length());
        e = RT_OK;
      }
    }
  }
  return e;
}

rtError pxArchive::getFileData(const char* fileName, rtData& d)
{
  rtError e = RT_FAIL;
  if (mLoadStatus.get<int32_t>("statusCode") == 0)
  {
    if (mIsFile)
    {
      e = d.init(mData.data(), mData.length());
    }
    else
    {
      if (mZip.getFileData(fileName,d)==RT_OK)
      {
        e = RT_OK;
      }
    }
  }
  return e;
}

rtError pxArchive::fileNames(rtObjectRef& array) const
{
  rtError e = RT_FAIL;

  // TODO The Horror
  pxArchive* c = const_cast<pxArchive*>(this);

  if (c->mLoadStatus.get<int32_t>("statusCode") == 0)
  {
    rtRef<rtArrayObject> f = new rtArrayObject;

    if (mIsFile)
    {
      // try to retrieve file path from url
      // look for scheme
      
      size_t schemePos = mUrl.find(0,"://");
      if (schemePos != (size_t)-1)
      {
        size_t pathPos = mUrl.find(schemePos+3,"/");
        if (pathPos != (size_t)-1)
          f->pushBack(mUrl.substring(pathPos));
        else
          f->pushBack("/");  // Make up something
      }
      else
        f->pushBack(mUrl);  // Given no scheme push entire url

    }
    else
    {
      uint32_t fileCount = mZip.fileCount();
      for (uint32_t i = 0; i < fileCount; i++)
      {
        rtString filePath;
        if (mZip.getFilePathAtIndex(i,filePath) == RT_OK)
        {
          f->pushBack(filePath);
        }
      }
    }
    e = RT_OK;
    array = f;
  }
  return e;
}

void pxArchive::onDownloadComplete(rtFileDownloadRequest* downloadRequest)
{
  pxArchive* a = (pxArchive*)downloadRequest->callbackData();

  if (a != NULL)
  {
    a->setArchiveData(downloadRequest->downloadStatusCode(), (uint32_t)downloadRequest->httpStatusCode(),
                      downloadRequest->downloadedData(), downloadRequest->downloadedDataSize(),
                      downloadRequest->errorString());

    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(pxArchive::onDownloadCompleteUI, a, NULL);
    }
  }
}

void pxArchive::onDownloadCompleteUI(void* context, void* /*data*/)
{
  pxArchive* a = (pxArchive*)context;

  a->setupArchive();

  // Todo Real error condition
  if (a->mLoadStatus.get<int32_t>("statusCode") == 0)
  {
    if (a->mIsFile)
      a->mReady.send("resolve", a);
    else
    {
      if (a->mZip.fileCount() > 0)
        a->mReady.send("resolve", a);
      else
        a->mReady.send("reject", a);
    }
  }
  else
    a->mReady.send("reject", a);

  //  We're done with the archive object so release it
  a->Release();
}

void pxArchive::process(void* data, size_t dataSize)
{
  if (rtZip::isZip(data,dataSize))
  {
    mIsFile = false;
    if (mZip.initFromBuffer(data,dataSize) != RT_OK)
    {
      rtLogWarn("error initializing zip data from buffer");
    }
  }
  else
  {
    // Single file archive
    mIsFile = true;
  }
}

bool pxArchive::isFile()
{
  return mIsFile;
}

rtString pxArchive::getName()
{
  return mUrl;
}

rtDefineObject(pxArchive,rtObject);
rtDefineProperty(pxArchive,ready);
rtDefineProperty(pxArchive,loadStatus);
rtDefineMethod(pxArchive,getFileAsString);
rtDefineProperty(pxArchive,fileNames);
