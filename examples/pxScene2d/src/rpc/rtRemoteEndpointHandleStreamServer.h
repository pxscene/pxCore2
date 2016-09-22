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