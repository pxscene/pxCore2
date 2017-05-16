#ifndef PX_SERVICEMANAGER_H
#define PX_SERVICEMANAGER_H

#ifdef USE_RT_REMOTE

#include "rtRemote.h"

#define SERVICE_MANAGER_OBJECT_NAME "rtServiceManager"

class pxServiceManager
{
public:
  static rtError findServiceManager(rtObjectRef &result);

private:
  static rtRemoteEnvironment* mEnv;
};

#endif

#endif
