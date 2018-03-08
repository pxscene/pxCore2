/*
 
 pxCore Copyright 2005-2017 John Robinson
 
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

#ifndef _PX_CANVAS2D_H
#define _PX_CANVAS2D_H

#ifdef _WINDOWS_
#define RTPLATFORM_WINDOWS
#endif


#include "px2d.h"
#include "pxContext.h"
#include "pxMatrix4T.h"
#include "pxRasterizer.h"


#ifdef RTPLATFORM_WINDOWS
#include "rtString.h"
#endif

class pxCanvas2d
{

public:
  pxCanvas2d();
  ~pxCanvas2d();

//  void needsRedraw();
  
  pxError init(int width, int height);
  pxError initWithBuffer(pxBuffer* buffer);

  pxError term();
  
  pxOffscreen& offscreen() { return mOffscreen; };

  void newPath();
  void moveTo(double x, double y);
  void lineTo(double x, double y);
  
  void curveTo(double x2, double y2, double x3, double y3);                       // Quadratic
  void curveTo(double x2, double y2, double x3, double y3, double x4, double y4); // Cubic
  
  void closePath();

  // The following methods create a path for the associated shape
  // to render call either fill() or stroke()

  //void rectangle(float x1, float y1, float x2, float y2);

#if 0
  void rotate(double a);
  void scale(double sx, double sy);
  void translate(double dx, double dy);
#endif

  void matrix(pxMatrix4T<float>& matrix) const;
  void setMatrix(const pxMatrix4T<float>& matrix);

  void textureMatrix(pxMatrix4T<float>& m) const;
  void setTextureMatrix(const pxMatrix4T<float>& m);

  // Fill the current path
  void fill(bool time = false);

#ifdef RTPLATFORM_WINDOWS
  pxError drawChar(const wchar_t c);
  void drawText(const wchar_t* t, double x, double y);
#endif

#ifdef RTPLATFORM_WINDOWS
  rtString font();
  void setFont(const rtString& font);
#endif
    
  void setFontSize(double s);

  pxFillMode fillMode();
  void setFillMode(const pxFillMode& mode);

  pxColor fillColor();
  void setFillColor(int r, int g, int b, int a = 255);
  void setFillColor(int gray, int a = 255);
  void setFillColor(const pxColor& c);

  // Stroke for the current path
  enum class StrokeType { inside, outside, center };
  void stroke();  // DEFAULT: inside
  
  inline void setStrokeMode(StrokeType s) { mStrokeType = s; };

  pxColor strokeColor();
  void setStrokeColor(int r, int g, int b, int a = 255);
  void setStrokeColor(int gray, int a = 255);
  void setStrokeColor(const pxColor& c);

  void setStrokeWidth(double w);
  void setStrokeType(StrokeType t);
  StrokeType strokeType() { return mStrokeType; };

  double alpha() const;
  void setAlpha(double alpha);


  //void clear();

  // Clipping
  // Pass NULL in to unset the clipping region
  void setClip(const pxRect* r);

  void setYOversample(int i)
  {
    mRasterizer.setYOversample(i);
  }

  pxBuffer* texture() const;
  void setTexture(pxBuffer* texture);

  bool textureClamp() const    { return mRasterizer.textureClamp(); }
  void setTextureClamp(bool f) {        mRasterizer.setTextureClamp(f); }

  bool textureClampColor() const    { return mRasterizer.textureClampColor(); }
  void setTextureClampColor(bool f) {        mRasterizer.setTextureClampColor(f); }

  bool biLerp() const { return mRasterizer.biLerp(); }
  void setBiLerp(bool f) { mRasterizer.setBiLerp(f); }

#if 0
  bool alphaTexture() const    { return mRasterizer.alphaTexture();     }
  void setAlphaTexture(bool f) {        mRasterizer.setAlphaTexture(f); }
#else
  bool alphaTexture() const;
  void setAlphaTexture(bool f);
#endif

  bool overdraw() const    { return mRasterizer.overdraw();     }
  void setOverdraw(bool f) {        mRasterizer.setOverdraw(f); }

  void roundRect(double x, double y, double w, double h, double rx, double ry);
  //void roundRectangle(double x, double y, double w, double h, double rad);
	void rectangle(double x1, double y1, double x2, double y2);

  void clear();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
private:
  
  void stroke_inout(StrokeType t);
  void stroke_inside();
  void stroke_outside();
  void stroke_center();
  
  // Quadratic
  void addCurve (double x1, double y1, double x2, double y2, double x3, double y3);
  void addCurve (double x1, double y1, double x2, double y2, double x3, double y3, int depth);

  // Quadratic
  void addCurve2(double x1, double y1, double x2, double y2, double x3, double y3);
  void addCurve2(double x1, double y1, double x2, double y2, double x3, double y3, int depth);
  
  // Cubic
  void addCurve22(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
  void addCurve22(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, unsigned depth);


#ifdef RTPLATFORM_WINDOWS
  inline double convertFixToFloat(const FIXED& fx);
  void calcTextScale(int a, int ascent);
  
  void TextMoveTo(double x, double y);
  void TextLineTo(double x, double y);
  void TextCurveTo(double x2, double y2, double x3, double y3);

  double mTextScale;
  int mBaseLineAdjust;

  rtString mFont;
#endif

  double mFontSize;

  double textX, textY;
  double lastX, lastY;

  double getPenX();
  double getPenY();

  int mVertexCount;
  pxVertex mVertices[200000];

  int vertexCount() const    { return mVertexCount; }

  pxMatrix4T<float> mMatrix;
  pxMatrix4T<float> mTextureMatrix;

  pxColor mFillColor;

  pxColor    mStrokeColor;
  double     mStrokeWidth;
  StrokeType mStrokeType;

//  virtual void draw();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
public: // BUGBUG
  pxRasterizer mRasterizer;
  pxOffscreen  mOffscreen;
  pxTextureRef mTexture;

  double extentLeft,  extentTop;
  double extentRight, extentBottom;

  bool    mNeedsRedraw;

  int mw;
  int mh;

};//CLASS

#endif // _PX_CANVAS2D_H
