// rtCore CopyRight 2007-2015 John Robinson
// rtFile.h

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "rtDefs.h"

class rtData {
 public:
  rtData();
  ~rtData();
  
  rtError init(uint32_t length);
  rtError init(uint8_t* data, uint32_t length);

  rtError term();

  uint8_t* data();
  uint32_t length();

 private:
  uint8_t* mData;
  uint32_t mLength;
};

rtError rtLoadFile(const char* f, rtData& data);
