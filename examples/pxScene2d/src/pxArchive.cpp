#include "pxArchive.h"

#include "rtThreadQueue.h"

extern rtThreadQueue gUIThreadQueue;

#include "pxFileDownloader.h"

pxArchive::pxArchive(): mIsFile(true) {}

pxArchive::~pxArchive() {}

rtError pxArchive::initFromUrl(const rtString& url)
{
  mReady = new rtPromise;
  mLoadStatus = new rtMapObject;

  mUrl = url;

  if (url.beginsWith("http:") || url.beginsWith("https:"))
  {
    mLoadStatus.set("sourceType", "http");
    mLoadStatus.set("statusCode", -1);
    mDownloadRequest = new pxFileDownloadRequest(url, this);
    mDownloadRequest->setCallbackFunction(pxArchive::onDownloadComplete);
    pxFileDownloader::getInstance()->addToDownloadQueue(mDownloadRequest);
  }
  else
  {
    // Assuming file
    mLoadStatus.set("sourceType", "file");
    // TODO align statusCodes for loadStatus
    if (rtLoadFile(url, mData) == RT_OK)
    {
      mLoadStatus.set("statusCode",0);
      process(mData.data(),mData.length());
    }
    else
    {
      mLoadStatus.set("statusCode",1);
      gUIThreadQueue.addTask(pxArchive::onDownloadCompleteUI,this,NULL);
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

rtError pxArchive::fileNames(rtObjectRef& array) const
{
  rtError e = RT_FAIL;

  // TODO The Horror
  pxArchive* c = const_cast<pxArchive*>(this);

  if (c->mLoadStatus.get<int32_t>("statusCode") == 0)
  {
    rtRefT<rtArrayObject> f = new rtArrayObject;

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

void pxArchive::onDownloadComplete(pxFileDownloadRequest* downloadRequest)
{
  pxArchive* a = (pxArchive*)downloadRequest->getCallbackData();

  a->mLoadStatus.set("statusCode", downloadRequest->getDownloadStatusCode());
  // TODO rtValue doesn't like longs... rtValue and fix downloadRequest
  a->mLoadStatus.set("httpStatusCode", (uint32_t)downloadRequest->getHttpStatusCode());

  if (downloadRequest->getDownloadStatusCode() == 0)
  {
    char* data;
    size_t dataSize;
    downloadRequest->getDownloadedData(data, dataSize);

    // TODO another copy here
    a->mData.init((uint8_t*)data,dataSize);
    a->process(a->mData.data(),a->mData.length());
  }
  else
    gUIThreadQueue.addTask(pxArchive::onDownloadCompleteUI, a, NULL);

  // done with this
  delete downloadRequest;
}

void pxArchive::onDownloadCompleteUI(void* context, void* /*data*/)
{
  pxArchive* a = (pxArchive*)context;
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
}

void pxArchive::process(void* data, size_t dataSize)
{
  if (rtZip::isZip(data,dataSize))
  {
    mIsFile = false;
    if (mZip.initFromBuffer(data,dataSize) == RT_OK)
      gUIThreadQueue.addTask(pxArchive::onDownloadCompleteUI, this, NULL);
  }
  else
  {
    // Single file archive
    mIsFile = true;
    gUIThreadQueue.addTask(pxArchive::onDownloadCompleteUI, this, NULL);
  }
}

rtDefineObject(pxArchive,rtObject);
rtDefineProperty(pxArchive,ready);
rtDefineProperty(pxArchive,loadStatus);
rtDefineMethod(pxArchive,getFileAsString);
rtDefineProperty(pxArchive,fileNames);
