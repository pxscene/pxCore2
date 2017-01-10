#ifndef __RT_REMOTE_MESSAGE_HANDLER_H__
#define __RT_REMOTE_MESSAGE_HANDLER_H__

#include "rtRemoteMessage.h"

#include <memory>

class rtRemoteClient;

using rtRemoteMessageHandler = rtError (*)(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& msg, void* argp);

#endif
