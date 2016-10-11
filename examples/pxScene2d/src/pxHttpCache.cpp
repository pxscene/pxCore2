#include <pxHttpCache.h>
#include <string.h>
#include <curl/curl.h>
#include <sstream>
#include "rtLog.h"

/* structure to maintain the response header and contents information for HTTP request */
struct MemoryStruct {
    MemoryStruct()
        : headerSize(0)
        , headerBuffer()
        , contentsSize(0)
        , contentsBuffer()
    {
        headerBuffer = (char*)malloc(1);
        contentsBuffer = (char*)malloc(1);
    }

  size_t headerSize;
  char* headerBuffer;
  size_t contentsSize;
  char* contentsBuffer;
};

static size_t HeaderCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  mem->headerBuffer = (char*)realloc(mem->headerBuffer, mem->headerSize + downloadSize + 1);
  if(mem->headerBuffer == NULL) {
    return 0;
  }
  memcpy(&(mem->headerBuffer[mem->headerSize]), contents, downloadSize);
  mem->headerSize += downloadSize;
  mem->headerBuffer[mem->headerSize] = 0;

  return downloadSize;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->contentsBuffer = (char*)realloc(mem->contentsBuffer, mem->contentsSize + downloadSize + 1);
  if(mem->contentsBuffer == NULL) {
    return 0;
  }

  memcpy(&(mem->contentsBuffer[mem->contentsSize]), contents, downloadSize);
  mem->contentsSize += downloadSize;
  mem->contentsBuffer[mem->contentsSize] = 0;

  return downloadSize;
}

rtHttpCacheData::rtHttpCacheData():mExpirationDate(-1),mUpdated(false)
{
  fp = NULL;
}

rtHttpCacheData::rtHttpCacheData(const char* url):mUrl(url),mExpirationDate(-1),mUpdated(false)
{
  fp = NULL;
}

rtHttpCacheData::rtHttpCacheData(const char* url, const char* headerMetadata, const char* data, int size):mUrl(url),mExpirationDate(-1),mUpdated(false)
{
  if ((NULL != headerMetadata) && (NULL != data))
  {
    mHeaderMetaData.init(headerMetadata,strlen(headerMetadata));
    populateHeaderMap();
    setExpirationDate();
    mData.init(data,size);
  }
  fp = NULL;
}

void rtHttpCacheData::populateHeaderMap()
{
  int pos=0,prevpos = 0;
  string headerString = mHeaderMetaData.data();
  pos = headerString.find_first_of("\n",0);
  string attribute = headerString.substr(prevpos,(pos = headerString.find_first_of("\n",prevpos))-prevpos);
  do
  {
    if (attribute.size() >  0)
    {
      prevpos = pos+1;

      //parsing the header attribute and value pair
      string key(""),value("");
      int name_end_pos = attribute.find_first_of(":");
      if (name_end_pos == string::npos)
      {
        key = attribute; 
      }
      else
      {
        key = attribute.substr(0,name_end_pos);
      }
      int cReturn_nwLnPos  = key.find_first_of("\r");
      if (string::npos != cReturn_nwLnPos)
        key.erase(cReturn_nwLnPos,1);
      cReturn_nwLnPos  = key.find_first_of("\n");
      if (string::npos != cReturn_nwLnPos)
        key.erase(cReturn_nwLnPos,1);
      if (name_end_pos == string::npos)
      {
        mHeaderMap.insert(std::pair<rtString, rtString>(key.c_str(),rtString("")));
      }
      else
      {
       value = attribute.substr(name_end_pos+1,attribute.length());
       cReturn_nwLnPos  = value.find_first_of("\r");
       if (string::npos != cReturn_nwLnPos)
         value.erase(cReturn_nwLnPos,1);
       cReturn_nwLnPos  = value.find_first_of("\n");
       if (string::npos != cReturn_nwLnPos)
         value.erase(cReturn_nwLnPos,1);
       mHeaderMap.insert(std::pair<rtString, rtString>(key.c_str(),value.c_str()));
      }
    }
    attribute = headerString.substr(prevpos,(pos = headerString.find_first_of("\n",prevpos))-prevpos);
  } while(pos != string:: npos);
  for (map<rtString,rtString>::iterator iter = mHeaderMap.begin(); iter != mHeaderMap.end(); iter++)
  {
    printf("key[%s] value[%s] \n",iter->first.cString(),iter->second.cString());
    fflush(stdout);
  }
}

rtString rtHttpCacheData::expirationDate()
{
  char buffer[100];
  memset(buffer,0,100);
  strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&mExpirationDate));
  return rtString(buffer);
}

bool rtHttpCacheData::isExpired()
{
  time_t now = time(NULL);
  if (now >= mExpirationDate)
    return true;
  return false;
}

bool rtHttpCacheData::isValid()
{
  if ((mData.length() != 0)  && !isExpired())
  {
    return true;
  }
  return false;
}

bool rtHttpCacheData::isWritableToCache()
{
  // need to add more  conditions ???
  if (isValid())
  {
    string cacheControl = mHeaderMap["Cache-Control"].cString();
    if (string::npos != cacheControl.find("no-store"))
    {
      return false;
    }
    return true;
  }
  return false;
}

void rtHttpCacheData::setAttributes(const char* rawAttributes)
{
  mHeaderMetaData.init(rawAttributes,strlen(rawAttributes));
  populateHeaderMap();
  setExpirationDate();
}

rtError rtHttpCacheData::attributes(map<rtString, rtString>& cacheAttributes)
{
  cacheAttributes = mHeaderMap;
  return RT_OK;
}

rtData& rtHttpCacheData::getHeaderData()
{
  return mHeaderMetaData;
}

rtData& rtHttpCacheData::getContentsData()
{
  return mData;
}

rtError rtHttpCacheData::data(rtData& data)
{
  if (NULL == fp)
    return RT_ERROR;

  if (mHeaderMap.end() != mHeaderMap.find("ETag"))
  {
    bool isUpdated = isDataUpdatedInServer();
    if (isUpdated)
    {
      populateHeaderMap();
      setExpirationDate();
      data.init(mData.data(),mData.length());
      mUpdated = true;
      fclose(fp);
      return RT_OK;
    }
  }

  char *contentsData = NULL;
  char buffer[100];
  int bytesCount = 0;
  int totalBytes = 0;
  while (!feof(fp))
  {
    bytesCount = fread(buffer,1,100,fp);
    if (NULL == contentsData)
      contentsData = (char *)malloc(bytesCount);
    else
      contentsData = (char *)realloc(contentsData,totalBytes+bytesCount);
    if (NULL == contentsData)
    {
      fclose(fp);
      return RT_ERROR;
    }
    memcpy(contentsData+totalBytes,buffer,bytesCount);
    totalBytes += bytesCount;
    memset(buffer,0,100);
  }
  fclose(fp);
  if (NULL != contentsData)
  {
    mData.init(contentsData,totalBytes);
    free(contentsData);
    contentsData = NULL;
    data.init(mData.data(),mData.length());
  }
  return RT_OK;
}

void rtHttpCacheData::setData(const rtData& cacheData)
{
  mData.init(cacheData.data(),cacheData.length());
}

rtError rtHttpCacheData::url(rtString& url)
{
  url = mUrl;
  return RT_OK;
}

rtError rtHttpCacheData::etag(rtString& tag) //returns the etag (if available)
{
  if (mHeaderMap.end() != mHeaderMap.find("Etag"))
  {
    tag = mHeaderMap["Etag"];   	
    return RT_OK;
  }
  return RT_ERROR;
}

bool rtHttpCacheData::isUpdated()
{
  return mUpdated;
}

void rtHttpCacheData::setFilePointer(FILE* openedDescriptor)
{
  fp = openedDescriptor;
}

void rtHttpCacheData::setProxy(rtString& proxyServer)
{
  mProxyServer = proxyServer;
}

void rtHttpCacheData::setExpirationDate()
{
  string expirationDate = "";
  bool foundMaxAge = false;
  if (mHeaderMap.end() != mHeaderMap.find("Cache-Control"))
  {
    string cacheControl = mHeaderMap["Cache-Control"].cString();
    int pos = cacheControl.find("max-age");
    if (string::npos != pos)
    {
      foundMaxAge = true;
      string maxAge = cacheControl.substr(pos+8);
      long int maxAgeInt = stol(maxAge);
      mExpirationDate = time(NULL) + maxAgeInt;
    }
  }
  if (false == foundMaxAge) 
  {
    if (mHeaderMap.end() != mHeaderMap.find("Expires"))
    {
      struct tm timeinfo;
      memset(&timeinfo,0,sizeof(struct tm));
      strptime(mHeaderMap["Expires"].cString(), " %a, %d %b %Y %H:%M:%S %Z", &timeinfo);
      mExpirationDate = timegm(&timeinfo);
    }
  }
}

bool rtHttpCacheData::isDataUpdatedInServer()
{
  CURL *curl_handle;
  CURLcode res = CURLE_OK;

  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();

  if (NULL == curl_handle)
    return false;

  bool isUpdated = false;
  struct curl_slist *list = NULL;
  bool useProxy = !mProxyServer.isEmpty();
  MemoryStruct chunk;
  long int httpCode = -1;

   /* specify URL to get */
  rtLogInfo("Url for curl request [%s]",mUrl.cString());
  curl_easy_setopt(curl_handle, CURLOPT_URL, mUrl.cString());
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1); //when redirected, follow the redirections
  curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, HeaderCallback);
  curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30);
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, true);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  if (useProxy)
  {
      curl_easy_setopt(curl_handle, CURLOPT_PROXY, mProxyServer.cString());
      curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
  }

  string headerOption = "If-None-Match:";
  headerOption.append(mHeaderMap["ETag"].cString());
  list = curl_slist_append(list, headerOption.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);

  /* get it! */
  res = curl_easy_perform(curl_handle);
 
  /* check for errors */
  if (res != CURLE_OK) 
  {
       stringstream errorStringStream;
       
       errorStringStream << "Download error for: " << mUrl.cString()
               << ".  Error code : " << res << ".  Using proxy: ";
       if (useProxy)
       {
           errorStringStream << "true - " << mProxyServer.cString();
       }
       else
       {
           errorStringStream << "false";
       }
   
       goto exit;    
   }

   if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpCode) == CURLE_OK)
   {
     rtLogInfo("http code from server for image stored in cache(%d)",httpCode);
     if (200 == httpCode)
     {
       if (chunk.headerBuffer != NULL)
       {
         mHeaderMetaData.init(chunk.headerBuffer, chunk.headerSize);
       }
       if (chunk.contentsBuffer != NULL)
       {
         mData.init(chunk.contentsBuffer, chunk.contentsSize);
       }
       isUpdated = true;
     }
   }

exit:
   curl_easy_cleanup(curl_handle);

   if (chunk.contentsBuffer != NULL)
   {
      free(chunk.contentsBuffer);
      chunk.contentsBuffer = NULL;
   }
  
   if (chunk.headerBuffer != NULL)
   {
      free(chunk.headerBuffer);
      chunk.headerBuffer = NULL;
   }
   return isUpdated;
}
