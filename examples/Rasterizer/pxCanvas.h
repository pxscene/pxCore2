#ifndef _PX_CANVAS_H
#define _PX_CANVAS_H

#ifdef _WINDOWS_
#define RTPLATFORM_WINDOWS
#endif


#define MATRIX_T  pxMatrix4T<float>
#include "pxMatrix4T.h"

#include "px2d.h"
#include "pxRasterizer.h"


#ifdef RTPLATFORM_WINDOWS
#include "rtString.h"
#endif

class pxCanvas
{
public:
  pxCanvas();
  ~pxCanvas();

  pxError init(int width, int height);
  pxError initWithBuffer(pxBuffer* buffer);

  pxError term();

  pxOffscreen* offscreen();

  void newPath();
  void moveTo(double x, double y);
  void lineTo(double x, double y);
  // Quadratic
  void curveTo(double x2, double y2, double x3, double y3);
  // Cubic
  //void curveTo(double x2, double y2, double x3, double y3, double x4, double y4);
  void closePath();

  // The following methods create a path for the associated shape
  // to render call either fill() or stroke()

  //void rectangle(float x1, float y1, float x2, float y2);

#if 0
  void rotate(double a);
  void scale(double sx, double sy);
  void translate(double dx, double dy);
#endif

  void matrix(MATRIX_T& matrix) const;
  void setMatrix(const MATRIX_T& matrix);

  void textureMatrix(MATRIX_T& m) const;
  void setTextureMatrix(const MATRIX_T& m);

  // Fill the current path
  void fill(bool time = false);

  // Stroke for the current path
  void stroke();

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

  pxColor strokeColor();
  void setStrokeColor(int r, int g, int b, int a = 255);
  void setStrokeColor(int gray, int a = 255);
  void setStrokeColor(const pxColor& c);

  double alpha() const;
  void setAlpha(double alpha);

  void setStrokeWidth(double w);

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

  bool textureClamp() const { return mRasterizer.textureClamp(); }
  void setTextureClamp(bool f) { mRasterizer.setTextureClamp(f); }

  bool textureClampColor() const { return mRasterizer.textureClampColor(); }
  void setTextureClampColor(bool f) { mRasterizer.setTextureClampColor(f); }

  bool biLerp() const { return mRasterizer.biLerp(); }
  void setBiLerp(bool f) { mRasterizer.setBiLerp(f); }

#if 0
  bool alphaTexture() const { return mRasterizer.alphaTexture(); }
  void setAlphaTexture(bool f) { mRasterizer.setAlphaTexture(f); }
#else
  bool alphaTexture() const;
  void setAlphaTexture(bool f);
#endif

  bool overdraw() const { return mRasterizer.overdraw(); }
  void setOverdraw(bool f) { mRasterizer.setOverdraw(f); }

  void roundRect(/*pxCanvas& c, */double x, double y, double w, double h, double rad);
  //void roundRectangle(double x, double y, double w, double h, double rad);
	void rectangle(double x1, double y1, double x2, double y2);

  void clear();

//private:
  void addCurve(double x1, double y1, double x2, double y2, double x3, double y3);
  void addCurve(double x1, double y1, double x2, double y2, double x3, double y3, int depth);
  void addCurve2(double x1, double y1, double x2, double y2, double x3, double y3);
  void addCurve2(double x1, double y1, double x2, double y2, double x3, double y3, int depth);



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

  int mVertexCount;
  pxVertex mVertices[200000];
  MATRIX_T mMatrix;
  MATRIX_T mTextureMatrix;
  pxColor mFillColor;
  pxColor mStrokeColor;
  double mStrokeWidth;

public: // BUGBUG
  pxRasterizer mRasterizer;

  pxOffscreen* mOffscreen;
};

#endif
