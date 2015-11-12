// pxCore CopyRight 2007-2015 John Robinson
// pxText.h

#ifndef PX_RECTANGLE_H
#define PX_RECTANGLE_H

#include "pxScene2d.h"

class pxRectangle: public pxObject {
public:
  rtDeclareObject(pxRectangle, pxObject);
  rtProperty(fillColor, fillColor, setFillColor, uint32_t);
  rtProperty(lineColor, lineColor, setLineColor, uint32_t);
  rtProperty(lineWidth, lineWidth, setLineWidth, float);

  pxRectangle(pxScene2d* scene):pxObject(scene),mLineWidth(0) 
  {
    float f[4] = {0,0,0,1};
    float l[4] = {1,1,1,1};
    setFillColor(f);
    setLineColor(l);
    //mReady.send("resolve",this);
  }

  virtual void onInit() {mReady.send("resolve",this);}
  
  rtError fillColor(uint32_t& /*c*/) const {
    rtLogWarn("fillColor not implemented");
    return RT_OK;
  }

  rtError setFillColor(uint32_t c) {
    mFillColor[0] = (float)((c>>24)&0xff)/255.0f;
    mFillColor[1] = (float)((c>>16)&0xff)/255.0f;
    mFillColor[2] = (float)((c>>8)&0xff)/255.0f;
    mFillColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError lineColor(uint32_t& /*c*/) const {
    rtLogWarn("lineColor not implemented");
    return RT_OK;
  }

  rtError setLineColor(uint32_t c) {
    mLineColor[0] = (float)((c>>24)&0xff)/255.0f;
    mLineColor[1] = (float)((c>>16)&0xff)/255.0f;
    mLineColor[2] = (float)((c>>8)&0xff)/255.0f;
    mLineColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError lineWidth(float& w) const {
    w = mLineWidth;
    return RT_OK;
  }
  
  rtError setLineWidth(float w) {
    mLineWidth = w;
    return RT_OK;
  }

  // c is assumed to not be premultiplied
  void setFillColor(float* c) {
    mFillColor[0] = c[0];
    mFillColor[1] = c[1];
    mFillColor[2] = c[2];
    mFillColor[3] = c[3];
  }
  
  // c is assumed to not be premultiplied
  void setLineColor(float* c) {
    mLineColor[0] = c[0];
    mLineColor[1] = c[1];
    mLineColor[2] = c[2];
    mLineColor[3] = c[3];
  }
  
  virtual void draw();
  
private:
  float mFillColor[4];
  float mLineColor[4];
  float mLineWidth;
};

#endif
