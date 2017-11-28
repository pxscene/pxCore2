#ifndef __RT_VALUE_READER_H__
#define __RT_VALUE_READER_H__

#include <memory>
#include <rtError.h>
#include <rtValue.h>
#include <rapidjson/document.h>

class rtRemoteClient;
class rtRemoteEnvironment;

class rtRemoteValueReader
{
public:
  static rtError read(rtRemoteEnvironment* env, rtValue& val, rapidjson::Value const& from,
    std::shared_ptr<rtRemoteClient> const& client);
};

#endif
