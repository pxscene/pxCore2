#ifndef __RT_VALUE_READER_H__
#define __RT_VALUE_READER_H__

#include <memory>
#include <rtError.h>
#include <rtValue.h>
#include <rapidjson/document.h>

class rtRpcClient;

class rtValueReader
{
public:
  static rtError read(rtValue& val, rapidjson::Value const& from, std::shared_ptr<rtRpcClient> const& client);
};

#endif
