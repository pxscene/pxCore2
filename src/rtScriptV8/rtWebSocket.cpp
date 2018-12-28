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

#include "rtWebSocket.h"


using std::string;


rtDefineObject(rtWebSocket, rtObject);

rtDefineMethod(rtWebSocket, addListener);
rtDefineMethod(rtWebSocket, close);
rtDefineMethod(rtWebSocket, send);
rtDefineMethod(rtWebSocket, delListener);
rtDefineMethod(rtWebSocket, connect);
rtDefineMethod(rtWebSocket, clearListeners);

rtWebSocket::rtWebSocket(const rtObjectRef& options, uv_loop_t* loop)
    : mEmit(new rtEmit()), mWSHub(new uWS::Hub(0, true)), mWs(nullptr), mLoop(loop),
      mHeaders(new std::map<string, string>())
{

  mUri = string(options.get<rtString>("uri").cString());
  mTimeoutMs = options.get<int>("timeoutMs");
  mHeaders->clear();

  rtObjectRef headers = options.get<rtObjectRef>("headers");
  if (headers)
  {
    rtValue allKeys;
    headers->Get("allKeys", &allKeys);
    rtArrayObject* arr = (rtArrayObject*) allKeys.toObject().getPtr();
    for (uint32_t i = 0, l = arr->length(); i < l; ++i)
    {
      rtValue key;
      if (arr->Get(i, &key) == RT_OK && !key.isEmpty())
      {
        rtString s = key.toString();
        string key(s.cString());
        string val(headers.get<rtString>(s).cString());
        mHeaders->insert(std::pair<string, string>(key, val));
      }
    }
  }
}

rtWebSocket::~rtWebSocket()
{
  // clear all
  delete mHeaders;
  delete mWSHub;
}

rtError rtWebSocket::addListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->addListener(eventName, f);
  return RT_OK;
}

rtError rtWebSocket::delListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->delListener(eventName, f);

  return RT_OK;
}

rtError rtWebSocket::clearListeners()
{
  mEmit->clearListeners();
  return RT_OK;
}

rtError rtWebSocket::connect()
{

  // bind callback to webSocket hub
  uWS::Hub& hub = *mWSHub;

  hub.onConnection(
      [&](uWS::WebSocket <uWS::CLIENT>* ws, uWS::HttpRequest /*httpRequest*/)
      {
        mWs = ws;
        mEmit.send("open");
      }
  );

  hub.onMessage(
      [&](uWS::WebSocket <uWS::CLIENT>* ws, char* data, size_t length, uWS::OpCode opCode)
      {
        rtString message(data, length);
        mEmit.send("message", message);
      });

  hub.onError(
      [&](void*)
      {
        mEmit.send("error", rtValue("connection failed"));
      });

  hub.onDisconnection(
      [&](uWS::WebSocket <uWS::CLIENT>* /*ws*/, int code, char* msg, size_t length)
      {
        rtString errorMsg(msg, length);
        mEmit.send("close", rtValue(code), errorMsg);
      });

  hub.connect(mUri, nullptr, *mHeaders);
  return RT_OK;
}

rtError rtWebSocket::close()
{
  if (mWs != nullptr)
  {
    int code = 0;
    const char* message = "closed";
    mWs->close(code, message, strlen(message));
    mWs = nullptr;
  }
  return RT_OK;
}

rtError rtWebSocket::send(const rtString& chunk)
{
  if (mWs == nullptr)
  {
    rtLogError("webSocket still in connecting, cannot send message now.");
    return RT_ERROR;
  }

  //TODO only support text for now
  mWs->send(chunk.cString(), uWS::OpCode::TEXT);
  return RT_OK;
}