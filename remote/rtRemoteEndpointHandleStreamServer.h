/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef __RT_REMOTE_ENDPOINT_HANDLE_STREAM_SERVER_H__
#define __RT_REMOTE_ENDPOINT_HANDLE_STREAM_SERVER_H__

#include "rtRemoteEndpointHandle.h"
#include "rtRemoteEndpoint.h"
#include "rtError.h"
#include <sys/socket.h>

class rtRemoteEndpointHandleStreamServer : public virtual rtRemoteIEndpointHandle
{
public:
  rtRemoteEndpointHandleStreamServer(rtRemoteEndpointPtr endpoint);
  
  virtual rtError open() override;
	virtual rtError close() override;
	rtError doBind();
	rtError doListen();
	rtError doAccept(int& new_fd, rtRemoteEndpointPtr& remote_addr);

	inline sockaddr_storage sockaddr() const
	  { return m_socket; }

	sockaddr_storage m_socket;
};

#endif
