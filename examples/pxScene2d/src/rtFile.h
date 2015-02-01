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
  
  rt_error init(uint32_t length);
  rt_error init(uint8_t* data, uint32_t length);

  rt_error term();

  uint8_t* data();
  uint32_t length();

 private:
  uint8_t* m_data;
  uint32_t m_length;
};

rtError rtLoadFile(const char* f, rtData& data);
