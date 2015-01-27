// pxCore CopyRight 2007-2015 John Robinson
// pxText.h

#ifndef PX_TEXT_H
#define PX_TEXT_H

#include "rtString.h"
#include "rtRefT.h"
#include "pxScene2d.h"

class pxText: public pxObject {
public:
  rtDeclareObject(pxText, pxObject);
  rtProperty(text, text, setText, rtString);

  pxText();
  rtError text(rtString& s) const;
  rtError setText(const char* text);

 protected:
  virtual void draw();
  rtString mText;
};

#endif
