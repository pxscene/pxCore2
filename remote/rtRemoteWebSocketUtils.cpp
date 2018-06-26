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

#include "rtRemoteWebSocketUtils.h"

extern "C" {
#include "sha1.h"
}

#include <cstring>
#include <string>
#include <unistd.h>
#include <cerrno>
#include <rtLog.h>


#define BUFFER_SIZE 1024
#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define OP_TEXT 1
#define OP_CLOSE 8
#define OP_PING 9
#define OP_PONG 10


/*-------------------------------------------------------------------
0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
:                     Payload Data continued ...                :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
--------------------------------------------------------------------*/

/**
 * the frame head entity struct
 */
typedef struct _frame_head
{
  bool fin;
  int opcode;
  int mask;
  int payloadLength;
  char maskingKey[4];
} frame_head;

int
rtRemoteWebSocketUtils::getMessageType(char* buff, int length)
{
  if (length < 4)
    return SOCKET_BODY;

  // websocket hand shake request message first string must be start with GET /HTTP
  // but for compatible normal socket message, we read the first 4 chars, so we must make sure the first 4 chars is G E T ' '
  if (buff[0] == 'G' && buff[1] == 'E' && buff[2] == 'T' && buff[3] == ' ')
  {
    return WEB_SOCKET_HEADER;
  }
  else
  {
    int fin = buff[0] & 0x80;
    int opcode = buff[0] & 0x0F; // opcode must be one of this
    if (fin == 128 && (opcode == OP_TEXT || opcode == OP_CLOSE || opcode == OP_PING || opcode == OP_PONG))
    {
      return WEB_SOCKET_BODY;
    }
    return SOCKET_BODY;
  }
}


void
rtRemoteWebSocketUtils::umask(char* data, int len, char* mask)
{
  for (int i = 0; i < len; ++i)
  {
    *(data + i) ^= *(mask + (i % 4));
  }
}

rtError
rtRemoteWebSocketUtils::readWebsocketMessage(int fd, char* headerBuff, char* dataBuff, int& dataLength, int capacity)
{
  frame_head headEntity;
  frame_head* head = &headEntity;
  head->fin = (headerBuff[0] & 0x80) == 0x80;
  head->opcode = headerBuff[0] & 0x0F;
  head->mask = (headerBuff[1] & 0x80) == 0X80;
  head->payloadLength = (headerBuff[1] & 0x7F); // get payload length

  // update pauload length, max length is 65535
  if (head->payloadLength == 126)
  {
    head->payloadLength = (headerBuff[2] & 0xFF) << 8 | (headerBuff[3] & 0xFF);
  }
  else if (head->payloadLength == 127) // payload grater than 65535
  {
    char totalLen[8], temp;
    char externLen[6];
    int i;
    if (read(fd, externLen, 6) <= 0)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogError("failed to read from fd %d. %s", fd, rtStrError(e));
      return e;
    }
    totalLen[0] = headerBuff[2];
    totalLen[1] = headerBuff[3];
    for (int i = 0; i < 6; ++i)
    {
      totalLen[i + 2] = externLen[i];
    }
    for (i = 0; i < 4; i++)
    {
      temp = totalLen[i];
      totalLen[i] = totalLen[7 - i];
      totalLen[7 - i] = temp;
    }
    memcpy(&(head->payloadLength), totalLen, 8);
  }

  // read masking-key
  ssize_t maskKeyNeedReadLen = 4;
  if (head->payloadLength < 126)
  {
    head->maskingKey[0] = headerBuff[2];
    head->maskingKey[1] = headerBuff[3];
    maskKeyNeedReadLen = 2;
  }
  if (read(fd, head->maskingKey + 4 - maskKeyNeedReadLen, maskKeyNeedReadLen) <= 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to read from fd %d. %s", fd, rtStrError(e));
    return e;
  }

  if (head->payloadLength > capacity)
  {
    rtLogError("body payloadLength > capacity, %d %d", head->payloadLength, capacity);
    return RT_FAIL;
  }

  // start read payload data
  ssize_t needReadLength = head->payloadLength;
  int readedSize = 0;
  do
  {
    ssize_t rSize = read(fd, dataBuff + readedSize, needReadLength);
    if (rSize < 0)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogError("failed to read from fd %d. %s", fd, rtStrError(e));
      return e;
    }
    readedSize += rSize;
  } while (readedSize < needReadLength);

  // un mask data
  if (head->mask)
  {
    umask(dataBuff, head->payloadLength, head->maskingKey);
  }
  dataLength = head->payloadLength;
  return RT_OK;
}

int
rtRemoteWebSocketUtils::readLine(char* allBuf, int level, char* lineBuf)
{
  size_t len = strlen(allBuf);
  for (; level < len; ++level)
  {
    if (allBuf[level] == '\r' && allBuf[level + 1] == '\n')
      return level + 2;
    else
      *(lineBuf++) = allBuf[level];
  }
}

void
rtRemoteWebSocketUtils::getWebsocketResponseHeader(int bodyLen, char* headerBuffer, int& headerLen)
{
  if (bodyLen < 126)
  {
    headerBuffer[0] = 0x81;
    headerBuffer[1] = bodyLen;
    headerLen = 2;
  }
  else // max is 65535
  {
    headerBuffer[0] = 0x81;
    headerBuffer[1] = 126;
    headerBuffer[2] = (bodyLen >> 8 & 0xFF);
    headerBuffer[3] = (bodyLen & 0xFF);
    headerLen = 4;
  }
}
rtError
rtRemoteWebSocketUtils::doHandShake(int fd)
{
  int level = 0;
  char allBuffer[BUFFER_SIZE]; // hand shake request header never greater than BUFFER_SIZE
  char lineBuf[BUFFER_SIZE];
  char responseContent[BUFFER_SIZE] = {0};
  std::string secretKey;
  if (read(fd, allBuffer, sizeof(allBuffer)) <= 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to read from fd %d. %s", fd, rtStrError(e));
    return e;
  }
  rtLogInfo("websocket request content = \nGET %s", allBuffer);

  do
  {
    memset(lineBuf, 0, sizeof(lineBuf));
    level = readLine(allBuffer, level, lineBuf);
    if (strstr(lineBuf, "Sec-WebSocket-Key")) // only
    {
      strcat(lineBuf, GUID); // append guid
      char hash[20];
      std::string sValue(lineBuf + 19);
      SHA1(hash, sValue.c_str(), static_cast<int>(sValue.size())); // sha1 first
      char sKey[64];
      base64Encode(hash, 20, sKey); // base64
      secretKey = std::string(sKey);
    }
  } while ((allBuffer[level] != '\r' || allBuffer[level + 1] != '\n') && level != -1);

  if (!secretKey.empty())
  {
    sprintf(responseContent, "HTTP/1.1 101 Switching Protocols\r\n" \
                          "Upgrade: websocket\r\n" \
                          "Connection: Upgrade\r\n" \
                          "Sec-WebSocket-Accept: %s\r\n" \
                          "\r\n", secretKey.c_str());
    rtLogInfo("websocket response content = \n%s", responseContent);
    if (write(fd, responseContent, strlen(responseContent)) < 0)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogWarn("a websocket client hand shake failed, request buffer is %s", allBuffer);
      return e;
    }
    else
    {
      rtLogInfo("A websocket connection created");
    }
  }
  else
  {
    rtLogWarn("a websocket client hand shake failed, because of no %s", "Sec-WebSocket-Key");
    return RT_FAIL;
  }
  return RT_OK;
}

void
rtRemoteWebSocketUtils::base64Encode(const char* source, int dataLen, char* dest)
{
  const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const char padding_char = '=';
  int i = 0, j = 0;
  unsigned char trans_index = 0;
  for (; i < dataLen; i += 3)
  {
    trans_index = ((source[i] >> 2) & 0x3f);
    dest[j++] = base64char[(int) trans_index];
    trans_index = ((source[i] << 4) & 0x30);
    if (i + 1 < dataLen)
    {
      trans_index |= ((source[i + 1] >> 4) & 0x0f);
      dest[j++] = base64char[(int) trans_index];
    }
    else
    {
      dest[j++] = base64char[(int) trans_index];
      dest[j++] = padding_char;
      dest[j++] = padding_char;
      break;
    }
    trans_index = ((source[i + 1] << 2) & 0x3c);
    if (i + 2 < dataLen)
    {
      trans_index |= ((source[i + 2] >> 6) & 0x03);
      dest[j++] = base64char[(int) trans_index];
      trans_index = source[i + 2] & 0x3f;
      dest[j++] = base64char[(int) trans_index];
    }
    else
    {
      dest[j++] = base64char[(int) trans_index];
      dest[j++] = padding_char;
      break;
    }
  }
  dest[j] = '\0';
}
