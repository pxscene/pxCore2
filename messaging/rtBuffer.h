/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __RT_BUFFER_H__
#define __RT_BUFFER_H__

#include <stdint.h>
#include <string>
#include <vector>

class rtBuffer
{
public:
  rtBuffer();
  ~rtBuffer();

public:
  void clear();
  void rewind();
  int position() const;

public:
  void putInt8(int8_t n);
  void putInt16(int16_t n);
  void putInt32(int32_t n);
  // void putInt64(int64_t n);
  void putUInt8(uint8_t n);
  void putUInt16(uint16_t n);
  void putUInt32(uint32_t n);
  // void putUInt64(uint64_t n);
  void putString(char const* s, int n);

  int8_t getInt8() const;
  int16_t getInt16() const;
  int32_t getInt32() const;
  // int64_t getInt64() const;
  uint8_t getUInt8() const;
  uint16_t getUInt16() const;
  uint32_t getUInt32() const;
  // uint64_t getUInt64() const;
  std::string getString() const;

private:
  std::vector<uint8_t> m_vec;
  mutable int m_pos;
};

#endif
