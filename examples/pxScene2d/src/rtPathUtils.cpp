// rtCore CopyRight 2007-2015 John Robinson
// rtPathUtils.h

#include "rtPathUtils.h"
#include <stdlib.h>
#include <unistd.h>

rt_error rtGetCurrentDirectory(rtString& d) {
  char* p = getcwd(NULL, 0);

  if (p) {
    d = p;
    free(p);
  }
  return RT_OK;
}
