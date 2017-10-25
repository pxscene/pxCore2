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
