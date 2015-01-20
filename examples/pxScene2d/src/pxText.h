// pxCore CopyRight 2007-2015 John Robinson
// pxText.h

#ifndef PX_TEXT_H
#define PX_TEXT_H

#include "rtString.h"
#include "rtRefT.h"
#include "pxScene2d.h"

class pxText: public pxObject {
public:
  pxText();
  void text(rtString& s);
  void setText(const char* text);
 private:
  virtual void draw();

  rtString mText;
};

#endif
