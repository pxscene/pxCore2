// pxCore CopyRight 2007-2015 John Robinson
// pxText.h

#include "pxRectangle.h"
#include "pxContext.h"

extern pxContext context;

void pxRectangle::draw()
{
  context.drawRect(mw, mh, mLineWidth, mFillColor, mLineColor);
}

void pxRectangle::commitClone()
{
  const vector<pxObjectCloneProperty>& properties = mClone->getProperties();
  for(vector<pxObjectCloneProperty>::const_iterator it = properties.begin();
      it != properties.end(); ++it)
  {
    if ((it)->propertyName == "fillColor")
    {
      uint32_t c = (it)->value.toInt32();
      mFillColor[0] = (float)((c>>24)&0xff)/255.0f;
      mFillColor[1] = (float)((c>>16)&0xff)/255.0f;
      mFillColor[2] = (float)((c>>8)&0xff)/255.0f;
      mFillColor[3] = (float)((c>>0)&0xff)/255.0f;
    }
    else if ((it)->propertyName == "lineColor")
    {
      uint32_t c = (it)->value.toInt32();
      mLineColor[0] = (float)((c>>24)&0xff)/255.0f;
      mLineColor[1] = (float)((c>>16)&0xff)/255.0f;
      mLineColor[2] = (float)((c>>8)&0xff)/255.0f;
      mLineColor[3] = (float)((c>>0)&0xff)/255.0f;
    }
    else if ((it)->propertyName == "lineWidth")
    {
      mLineWidth = (it)->value.toFloat();
    }
  }
  pxObject::commitClone();
}

rtDefineObject(pxRectangle, pxObject);
rtDefineProperty(pxRectangle, fillColor);
rtDefineProperty(pxRectangle, lineColor);
rtDefineProperty(pxRectangle, lineWidth);

