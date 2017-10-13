// Copyright 2007 John Robinson

#ifndef  _H_PXRASTERIZER
#define _H_PXRASTERIZER

#include "px2d.h"


#include "pxMatrix4T.h"
#include "pxOffscreen.h"

#define USELONGCOVERAGE 1
#define EDGECLEANUP


//#define FRONT2BACK

class pxRasterizer
{
public:
  pxRasterizer();
  ~pxRasterizer();

  void init(pxBuffer* buffer);
  void term();

  void reset();

  void addEdge(double x1, double y1, double x2, double y2);

  void rasterize();
  //void rasterize2();

  pxFillMode fillMode() const;
  void setFillMode(const pxFillMode& m);

  pxColor color() const;
  void setColor(const pxColor& c);

  double alpha() const;
  void setAlpha(double alpha);

  // 1, 2, 4, 8, 16 supported
  // This will reset the rasterizer
  void setYOversample(int i);

  // ??
  // This will reset the rasterizer
  // Valid values for this are 0, 1, 2, 4, 8, 16
  void setXResolution(int i);

  pxRect clip();
  void setClip(const pxRect* r);

  pxBuffer* texture() const {return mTexture;}
  void setTexture(pxBuffer* texture);

#if 0
  bool alphaTexture() const { return mAlphaTexture; }
  void setAlphaTexture(bool f) 
  { 
    mAlphaTexture = f; 
  }
#else
  bool alphaTexture() const;
  void setAlphaTexture(bool f);
#endif

  void setTextureCoordinates(pxVertex& e1, pxVertex& e2, pxVertex& e3, pxVertex& e4,
                             pxVertex& t1, pxVertex& t2, pxVertex& t3, pxVertex& t4);

  bool textureClamp() const { return mTextureClamp; }
  void setTextureClamp(bool f) { mTextureClamp = f; }

  bool textureClampColor() const { return mTextureClampColor; }
  void setTextureClampColor(bool f) { mTextureClampColor = f; }

	void matrix(pxMatrix4T<float>& m) const;
	void setMatrix(const pxMatrix4T<float>& m);

  void textureMatrix(pxMatrix4T<float>& m) const;
  void setTextureMatrix(const pxMatrix4T<float>& m);

  bool biLerp() const { return mBiLerp; }
  void setBiLerp(bool f) { mBiLerp = f; }

  bool overdraw() const { return mOverdraw; }
  void setOverdraw(bool f) { mOverdraw = f;; }

  void clear();

private:

  void rasterizeComplex();

  void resetTextureEdges();
  void addTextureEdge(double x1, double y1, double x2, double y2,
                      double u1, double v1, double u2, double v2);

  inline void scanCoverage(pxPixel* scanline, int32_t x0, int32_t x1);
  inline pxPixel* getTextureSample(int32_t maxU, int32_t maxV, int32_t& curU, int32_t& curV);

  void calculateEffectiveAlpha();

  int mYOversample;
  int mXResolution;

  int mFirst, mLast;
  int mLeftExtent, mRightExtent;

  pxBuffer* mBuffer;

  //void* mStarts;
  //void* mEnds;

#ifndef EDGECLEANUP
  void* mEdgeArray;
  void * miStarts;
  void * miEnds;
  int mEdgeCount;
#else
  void* mEdgeManager;
#endif

#ifdef USELONGCOVERAGE
  char* mCoverage;
#else
  unsigned char* mCoverage;
#endif

  pxFillMode mFillMode;
  pxPixel mColor;
  double mAlpha;

  bool mAlphaDirty;
  unsigned char mEffectiveAlpha;
  unsigned char mCoverage2Alpha[256];

  pxRect mClip;
  bool mClipValid;
  pxRect mClipInternal;  // mClip interescted with the bounds of the current buffer.
  bool mClipInternalCalculated;

  int mCachedBufferHeight;
  int mCachedBufferWidth;

  // mYOversample derived values
  uint32_t overSampleAdd;
  uint32_t overSampleAddMinusOne;
  uint32_t overSampleAdd4MinusOne;
  uint32_t overSampleAdd4;
  uint32_t overSampleFlush;
  uint32_t overSampleMask;
  //   uint32_t overSampleShift;

  pxBuffer* mTexture;

//    public: // BUGBUG
  bool mTextureClamp;
  bool mTextureClampColor;
  bool mBiLerp;
  bool mAlphaTexture;

  bool mOverdraw;

  unsigned char ltEdgeCover[16];  // static?  can be shared
  unsigned char rtEdgeCover[16];
  
  pxMatrix4T<float> mMatrix22;
  pxMatrix4T<float> mTextureMatrix22;
  
  pxMatrix4T<float> mMatrix;
  pxMatrix4T<float> mTextureMatrix;

  int32_t mTextureOriginX, mTextureOriginY;

public:
//    double mExtentLeft, mExtentTop, mExtentRight, mExtentBottom;

};

#endif // _H_PXRASTERIZER

