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

#ifndef __RTREMOTE_RTREMOTEWEBSOCKETUTILS_H
#define __RTREMOTE_RTREMOTEWEBSOCKETUTILS_H


#include <rtError.h>
#include <arpa/inet.h>

#define WEB_SOCKET_HEADER  10001
#define WEB_SOCKET_BODY 10002
#define SOCKET_BODY 10003

/**
 * The rt remote web socket utils class
 */
class rtRemoteWebSocketUtils
{
public:


  /**
   * get message/packet type from buffer
   * @param buff the buffer
   * @param length  the buffer length
   * @return the message type
   */
  static int getMessageType(char* buff, int length);

  /**
   * do hand shake with client
   * @param fd the fd
   * @return status code
   */
  static rtError doHandShake(int fd);

  /**
   * parse websocket message and return the message payload with len
   * @param fd the fd
   * @param headerBuff the message first header, length is 3
   * @param dataBuff the out data buffer, start from 0
   * @param dataLength the final payload data length
   * @param capacity the data buffer capacity
   * @return the status code
   */
  static rtError readWebsocketMessage(int fd, char* headerBuff, char* dataBuff, int& dataLength, int capacity);

  /**
   * get websocket response header
   * @param bodyLen the total payload body length
   * @param headerBuffer the out header buff
   * @param headerLen the out header len
   */
  static void getWebsocketResponseHeader(int bodyLen, char* headerBuffer, int& headerLen);
private:
  /**
   * read line(end of /r/n) from buffer
   * @param allBuf the total header buff
   * @param level the position in total buffer
   * @param lineBuf the out line buffer array
   * @return the next start position
   */
  static int readLine(char* allBuf, int level, char* lineBuf);

  /**
   * use mask key unmask the data
   * @param data the source data
   * @param len the source data length
   * @param mask the mask key
   */
  static void umask(char* data, int len, char* mask);

  /**
   * encode string to base64 format
   * @param source the source char array
   * @param sourceLen the source data length
   * @param dest the dest output buff
   */
  static void base64Encode(const char* source, int sourceLen, char* dest);
};


#endif
