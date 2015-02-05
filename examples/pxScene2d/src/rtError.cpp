#include "rtError.h"

#define RT_ERROR_CASE(ERR) case ERR: s = # ERR; break;

const char* rtStrError(rtError e)
{
  const char* s = "UNKNOWN";
  switch (e)
  {
    RT_ERROR_CASE(RT_OK);
    // RT_ERROR_CASE(RT_ERROR); same as RT_FAIL
    RT_ERROR_CASE(RT_FAIL);
    RT_ERROR_CASE(RT_ERROR_NOT_ENOUGH_ARGS);
    RT_ERROR_CASE(RT_ERROR_INVALID_ARG);
    RT_ERROR_CASE(RT_PROP_NOT_FOUND);
    RT_ERROR_CASE(RT_OBJECT_NOT_INITIALIZED);
    RT_ERROR_CASE(RT_PROPERTY_NOT_FOUND);
    RT_ERROR_CASE(RT_OBJECT_NO_LONGER_AVAILABLE);
    RT_ERROR_CASE(RT_RESOURCE_NOT_FOUND);
    RT_ERROR_CASE(RT_NO_CONNECTION);
    default:
    break;
  }
  return s;
}

