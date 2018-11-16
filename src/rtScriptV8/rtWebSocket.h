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

#ifndef RT_WEB_SOCKET_H
#define RT_WEB_SOCKET_H

#include <map>
#include <uWS.h>

#include "headers.h"
#include "rtObject.h"
#include "rtString.h"

/**
 * this rtWebSocket used to support webSocket in v8 engine
 * base on lib uWebSockets-0.14.8 and use uv_default_loop (compatible with v8 uv loop)
 */
class rtWebSocket : public rtObject
{
public:
  rtDeclareObject(rtWebSocket, rtObject);

  /**
   * create new webSocket instance
   * @param options the webSocket options like uri, timeout,headers
   * @param loop the v8 event loop
   */
  rtWebSocket(const rtObjectRef& options, uv_loop_t* loop);

  ~rtWebSocket();

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);

  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);

  rtMethodNoArgAndNoReturn("close", close);

  rtMethodNoArgAndNoReturn("clearListeners", clearListeners);

  rtMethodNoArgAndNoReturn("connect", connect);

  rtMethod1ArgAndNoReturn("send", send, rtString);


  /**
   * add webSocket event listener
   * @param eventName the event name
   * @param f the callback function
   */
  rtError addListener(rtString eventName, const rtFunctionRef& f);

  /**
   * del  webSocket event listener
   * @param eventName the event name
   * @param f the callback function
   */
  rtError delListener(rtString eventName, const rtFunctionRef& f);

  /**
   * close webSocket
   */
  rtError close();

  /**
   * connect webSocket
   */
  rtError connect();

  /**
   * remove all event listeners
   */
  rtError clearListeners();


  /**
   * send string type message to webSocket server
   * @param chunk  the string data
   */
  rtError send(const rtString& chunk);

private:
  rtEmitRef mEmit;
  int mTimeoutMs;
  uWS::Hub* mWSHub;
  uv_loop_t* mLoop;
  uWS::WebSocket <uWS::CLIENT>* mWs;
  std::map<std::string, std::string>* mHeaders;
  std::string mUri;
};

#endif //RT_WEB_SOCKET_H