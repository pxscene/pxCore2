#include "rtRemoteMapperFile.h"
#include "rtRemoteMapper.h"
#include "rtRemoteEndpoint.h"
#include "rtRemoteConfig.h"
#include "rtRemoteFactory.h"
#include "rtRemoteEnvironment.h"
#include "rtError.h"

#include <string>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mutex>
#include <thread>

rtRemoteMapperFile::rtRemoteMapperFile(rtRemoteEnvironment* env)
: rtRemoteIMapper(env)
{
  m_file_path = m_env->Config->resolver_file_db_path();
}

rtRemoteMapperFile::~rtRemoteMapperFile()
{
}

rtError
rtRemoteMapperFile::registerEndpoint(std::string const& objectId, rtRemoteEndpointPtr const& endpoint)
{
  FILE* fp;
  fp = fopen(m_file_path.c_str(), "r");
  if (fp == nullptr)
    return RT_FAIL;

  // tmp file to write to while reading/checking contents of permanent file. renamed later.
  std::string tmpPath = m_file_path + ".tmp";

  char *line = NULL;
  size_t len = 0;
  int read = -1;

  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while ( access( tmpPath.c_str(), F_OK ) != -1 );
    FILE* tmpFp = fopen(tmpPath.c_str(),"w");
    if (tmpFp == nullptr)
      return RT_FAIL;

    // read line by line
    while ( (read = getline(&line, &len, fp)) != -1)
    {
      if (line[read-1] == '\n')
        line[read-1] = '\0';
      std::string lineString(line);
      size_t index = lineString.find("::=");
      if (index != std::string::npos)
      {
        std::string readId = lineString.substr(0, index);
        // if not the one we want to register, just copy record over
        if (objectId.compare(readId) != 0)
          fprintf(tmpFp, "%s\n", line);
        // else overwrite it
        else
          rtLogWarn("overwriting existing registered endpoint: %s", line);
      }
    }
    std::string result = objectId + "::=" + endpoint->toUriString();
    fprintf(tmpFp, "%s\n", result.c_str());
    fclose(tmpFp);
    
    if (rename(tmpPath.c_str(), m_file_path.c_str()))
      return RT_FAIL;
  } // end critical section

  if (fp)
    fclose(fp);
  
  return RT_OK;
}

rtError
rtRemoteMapperFile::deregisterEndpoint(std::string const& objectId)
{
  rtError err = RT_FAIL;

  FILE* fp;
  fp = fopen(m_file_path.c_str(), "r");
  if (fp == nullptr)
    return RT_FAIL;

  // tmp file to write to while reading/checking contents of permanent file. renamed later.
  std::string tmpPath = m_file_path + ".tmp";

  char *line = NULL;
  size_t len = 0;
  int read = -1;

  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while ( access( tmpPath.c_str(), F_OK ) != -1 );
    FILE* tmpFp = fopen(tmpPath.c_str(),"w");
    if (tmpFp == nullptr)
      return RT_FAIL;

    // read line by line
    while ( (read = getline(&line, &len, fp)) != -1)
    {
      if (line[read-1] == '\n')
        line[read-1] = '\0';
      std::string lineString(line);
      size_t index = lineString.find("::=");
      if (index != std::string::npos)
      {
        std::string readId = lineString.substr(0, index);
        // copy over everything except for the one to be deregistered
        if (objectId.compare(readId) != 0)
          fprintf(tmpFp, "%s\n", line);
        else
          err = RT_OK;
      }
    }
    fclose(tmpFp);
    
    if (rename(tmpPath.c_str(), m_file_path.c_str()))
      return RT_FAIL;
  } // end critical section
    
  if (fp)
    fclose(fp);

  return err;
}

rtError
rtRemoteMapperFile::lookupEndpoint(std::string const& objectId, rtRemoteEndpointPtr& /*endpoint*/)
{
  FILE* fp;
  fp = fopen(m_file_path.c_str(), "r");
  if (fp == nullptr)
    return RT_FAIL;

  char *line = NULL;
  size_t len = 0;
  int read = -1;

  std::string objectUri;  
  {
    // check if someone else is updating the file
    std::string tmpPath = m_file_path + ".tmp";
    while ( access( tmpPath.c_str(), F_OK ) != -1 );
  
    // read line by line
    while ( (read = getline(&line, &len, fp)) != -1)
    {
      if (line[read-1] == '\n')
        line[read-1] = '\0';
      std::string lineString(line);
      size_t index = lineString.find("::=");
      if (index != std::string::npos)
      {
        std::string readId = lineString.substr(0, index);
        if (objectId.compare(readId) == 0)
          objectUri = lineString.substr(index+3, std::string::npos);
      }
    }
  }

  if (fp)
    fclose(fp);
  
  // result contains endpoint's URI (if registered)
  if (!objectUri.empty())
    return RT_OK;
    // TODO: need to wait till Factory changes are integrated into odessa
    //return m_env->Factory->createEndpoint(objectUri, endpoint);
  else
    return RT_FAIL;
}

bool
rtRemoteMapperFile::isRegistered(std::string const& objectId)
{
  FILE* fp;
  fp = fopen(m_file_path.c_str(), "r");
  if (fp == nullptr)
    return RT_FAIL;

  char *line = NULL;
  size_t len = 0;
  int read = -1;

  std::string objectUri;  
  {
    // check if someone else is updating the file
    std::string tmpPath = m_file_path + ".tmp";
    while ( access( tmpPath.c_str(), F_OK ) != -1 );
  
    // read line by line
    while ( (read = getline(&line, &len, fp)) != -1)
    {
      if (line[read-1] == '\n')
        line[read-1] = '\0';
      std::string lineString(line);
      size_t index = lineString.find("::=");
      if (index != std::string::npos)
      {
        std::string readId = lineString.substr(0, index);
        if (objectId.compare(readId) == 0)
          objectUri = lineString.substr(index+3, std::string::npos);
      }
    }
  }

  if (fp)
    fclose(fp);
  
  // result contains endpoint's URI (if registered)
  if (!objectUri.empty())
    return true;

  return false;
}
