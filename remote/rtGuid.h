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

#ifndef __RT_GUID_H__
#define __RT_GUID_H__

#include <string>

class rtGuid
{
public:
  ~rtGuid();

  static rtGuid newRandom();
  static rtGuid newTime();
  static rtGuid fromString(char const* s);
  static rtGuid const& null();

  rtGuid(rtGuid const& rhs);
  rtGuid const& operator =  (rtGuid const& rhs);
  bool          operator == (rtGuid const& rhs) const;
  bool          operator <  (rtGuid const& rhs) const;
  bool          operator != (rtGuid const& rhs) const;

  std::string toString() const;


private:
  rtGuid();

private:
  std::string m_id;
};

#endif
