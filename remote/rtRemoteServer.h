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

#ifndef __RT_REMOTE_OBJECT_LOCATOR_H__
#define __RT_REMOTE_OBJECT_LOCATOR_H__

#include "rtRemoteClient.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteMessageHandler.h"
#include "Hub.h"

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>

typedef void(*clientDisconnectedCallback)(void *data);

class rtRemoteIResolver;

class rtRemoteServer
{
public:
  rtRemoteServer(rtRemoteEnvironment* env);
  ~rtRemoteServer();

public:
  rtError open();
  rtError registerObject(std::string const& objectId, rtObjectRef const& obj);
  rtError unregisterObject(std::string const& objectId);
  rtError findObject(std::string const& objectId, rtObjectRef& obj, uint32_t timeout, clientDisconnectedCallback cb, void *cbdata);
  rtError unregisterDisconnectedCallback( clientDisconnectedCallback cb, void *cbdata );
  rtError removeStaleObjects();
  rtError processMessage(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& msg);

private:

  struct connected_client
  {
    sockaddr_storage    peer;
    int                 fd;
  };

  void runListener();
  void doAccept(int fd);
  std::shared_ptr<rtRemoteClient> doWebSocketAccept(uWS::WebSocket<uWS::SERVER>* ws);


  static rtError onOpenSession_Dispatch(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onOpenSession(client, doc); }

  static rtError onGet_Dispatch(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onGet(client, doc); }

  static rtError onSet_Dispatch(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onSet(client, doc); }

  static rtError onMethodCall_Dispatch(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onMethodCall(client, doc); }

  static rtError onKeepAlive_Dispatch(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onKeepAlive(client, doc); }

  static rtError onKeepAliveResponse_Dispatch(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onKeepAliveResponse(client, doc); }

  static rtError onIncomingMessage_Dispatch(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onIncomingMessage(client, doc); }

  static rtError onClientStateChanged_Dispatch(std::shared_ptr<rtRemoteClient> const& client,
      rtRemoteClient::State state, void* argp)
    { return reinterpret_cast<rtRemoteServer *>(argp)->onClientStateChanged(client, state); }

  // command handlers
  rtError start();
  rtError onIncomingMessage(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& msg);
  rtError onOpenSession(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc);
  rtError onGet(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc);
  rtError onSet(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc);
  rtError onMethodCall(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc);
  rtError onKeepAlive(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc);
  rtError onKeepAliveResponse(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc);
  rtError openRpcListener();
  rtError openWebSocketListener();
  rtError onClientStateChanged(std::shared_ptr<rtRemoteClient> const& client, rtRemoteClient::State state);

private:
  struct ObjectReference
  {
    rtObjectRef       object;
    std::vector<int>  client_fds;
    time_t            last_used;
    bool              owner_removed;
  };

  struct ClientDisconnectedCB
  {
      clientDisconnectedCallback func;
      void*                      data;
      bool operator == (const ClientDisconnectedCB& other) const {return func == other.func && data == other.data;}
  };

  using ClientMap = std::map< std::string, std::shared_ptr<rtRemoteClient> >;
  using ClientDisconnectedCBMap = std::map< rtRemoteClient*, std::vector<ClientDisconnectedCB> >;
  using ClientList = std::vector< std::shared_ptr<rtRemoteClient > >;
  using CommandHandlerMap = std::map< std::string, rtRemoteCallback<rtRemoteMessageHandler> >;
  using ObjectRefeMap = std::map< std::string, ObjectReference >;

  sockaddr_storage              m_rpc_endpoint;
  int                           m_listen_fd;

  std::unique_ptr<std::thread>  m_thread;
  mutable std::mutex            m_mutex;
  CommandHandlerMap             m_command_handlers;

  rtRemoteIResolver*            m_resolver;
  ClientMap                     m_object_map;
  ClientList                    m_connected_clients;
  ClientDisconnectedCBMap       m_disconnected_callback_map;
  int                           m_shutdown_pipe[2];
  uint32_t                      m_keep_alive_interval;
  rtRemoteEnvironment*          m_env;
  uWS::Hub*                     m_websocket_hub;
};

#endif
