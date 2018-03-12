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

#include "rtString.h"
#include "rtRef.h"
#include "rtFileDownloader.h"

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"

#include "pxCanvas2d.h"

//#include "pxContext.h"
//extern pxContext context;

#include <stdio.h>
#include "math.h"

#ifdef USE_PERF_TIMERS
#include "pxTimer.h"
#endif


class Vector
{
public:
  float x_, y_;

  Vector(float f = 0.0f)
    : x_(f), y_(f) {}

  Vector(float x, float y)
    : x_(x), y_(y) {}
};

class LineSegment
{
public:
  Vector begin_;
  Vector end_;

  LineSegment(const Vector& begin, const Vector& end)
    : begin_(begin), end_(end) {}

  enum IntersectResult { PARALLEL, COINCIDENT, NOT_INTERESECTING, INTERESECTING };

  IntersectResult Intersect(const LineSegment& other_line, Vector& intersection)
  {
    float denom = ((other_line.end_.y_ - other_line.begin_.y_)*(end_.x_ - begin_.x_)) -
      ((other_line.end_.x_ - other_line.begin_.x_)*(end_.y_ - begin_.y_));

    float nume_a = ((other_line.end_.x_ - other_line.begin_.x_)*(begin_.y_ - other_line.begin_.y_)) -
      ((other_line.end_.y_ - other_line.begin_.y_)*(begin_.x_ - other_line.begin_.x_));

    float nume_b = ((end_.x_ - begin_.x_)*(begin_.y_ - other_line.begin_.y_)) -
      ((end_.y_ - begin_.y_)*(begin_.x_ - other_line.begin_.x_));

    if(denom == 0.0f)
    {
      if(nume_a == 0.0f && nume_b == 0.0f)
      {
        return COINCIDENT;
      }
      return PARALLEL;
    }

    float ua = nume_a / denom;
    //  float ub = nume_b / denom;

//        if(ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f)
    {
      // Get the intersection point.
      intersection.x_ = begin_.x_ + ua*(end_.x_ - begin_.x_);
      intersection.y_ = begin_.y_ + ua*(end_.y_ - begin_.y_);

      return INTERESECTING;
    }

    return NOT_INTERESECTING;
  }
};

#if 0
void DoLineSegmentIntersection(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3)
{
  LineSegment linesegment0(p0, p1);
  LineSegment linesegment1(p2, p3);

  Vector intersection;

  std::cout << "Line Segment 0: (" << p0.x_ << ", " << p0.y_ << ") to (" << p1.x_ << ", " << p1.y_ << ")\n"
            << "Line Segment 1: (" << p2.x_ << ", " << p2.y_ << ") to (" << p3.x_ << ", " << p3.y_ << ")\n";

  switch(linesegment0.Intersect(linesegment1, intersection))
  {
  case LineSegment::PARALLEL:
    std::cout << "The lines are parallel\n\n";
    break;
  case LineSegment::COINCIDENT:
    std::cout << "The lines are coincident\n\n";
    break;
  case LineSegment::NOT_INTERESECTING:
    std::cout << "The lines do not intersect\n\n";
    break;
  case LineSegment::INTERESECTING:
    std::cout << "The lines intersect at (" << intersection.x_ << ", " << intersection.y_ << ")\n\n";
    break;
  }
}
#endif



double mFontSize;

  double textX, textY;
  double lastX, lastY;


  int mVertexCount;

  pxColor mFillColor;

  pxColor mStrokeColor;
  double  mStrokeWidth;

  double extentLeft,  extentTop;
  double extentRight, extentBottom;

  bool    mNeedsRedraw;

  int mw;
  int mh;



pxCanvas2d::pxCanvas2d(): mFontSize(0.0), textX(0.0), textY(0.0), lastX(0.0), lastY(0.0),
                          mVertexCount(0),   mFillColor(pxRed),
                          mStrokeColor(pxRed), mStrokeWidth(0.0), mStrokeType(StrokeType::inside),
                          mOffscreen(/*NULL*/),
                          extentLeft(0.0), extentTop(0.0), extentRight(0.0), extentBottom(0.0),
                          mNeedsRedraw(false), mw(0), mh(0)
{

}

pxCanvas2d::~pxCanvas2d()
{
  rtLogInfo("DESTROY ... ~pxCanvas2d() \n");

  term();
}

pxError pxCanvas2d::term()
{
  mOffscreen.term();

  mVertexCount = 0;
  
  return PX_OK;
}

//====================================================================================================================================
//====================================================================================================================================

pxError pxCanvas2d::init(int width, int height)
{
  pxError e = PX_FAIL;
  term();

  mw = width;
  mh = height;
  
//  mOffscreen.initWithColor(width, height, pxRed.u); // HACK
  mOffscreen.initWithColor(width, height, pxClear.u);
  mRasterizer.init(&mOffscreen);

  e = PX_OK;
  
//  mOffscreen = new pxOffscreen;
//  if (mOffscreen)
//  {
//    mOffscreen->initWithColor(width, height, pxClear.u);  // pxBlue pxClear
//    mRasterizer.init(mOffscreen);
//
//    e = PX_OK;
//  }

  mMatrix.identity();
  mTextureMatrix.identity();
  setFillColor(0, 0, 0, 0);
  setStrokeColor(0, 0, 0);
  setAlpha(1.0);

  return e;
}

pxError pxCanvas2d::initWithBuffer(pxBuffer* buffer)
{
  pxError e = PX_FAIL;
  term();

  if (buffer)
  {
    mRasterizer.init(buffer);
    e = PX_OK;
  }

  mMatrix.identity();
  mTextureMatrix.identity();
  setFillColor(0, 0, 0);
  setStrokeColor(0, 0, 0);
  setAlpha(1.0);

  return e;
}

void pxCanvas2d::newPath()
{
  mVertexCount = 0;
}

double pxCanvas2d::getPenX()
{
  return (mVertexCount > 0) ? mVertices[mVertexCount-1].x() : 0;
}

double pxCanvas2d::getPenY()
{
  return (mVertexCount > 0) ? mVertices[mVertexCount-1].y() : 0;
}

void pxCanvas2d::moveTo(double x, double y)
{
  mVertices[mVertexCount].setX(x);
  mVertices[mVertexCount].setY(y);
  
  mVertexCount++;
}

void pxCanvas2d::lineTo(double x, double y)
{
  mVertices[mVertexCount].setX(x);
  mVertices[mVertexCount].setY(y);
  
  mVertexCount++;
}

// Cubic
void pxCanvas2d::curveTo(double x2, double y2, double x3, double y3, double x4, double y4)  // uses preceeding vertex as (x1,y1)
{
  if (mVertexCount > 0)
  {
    addCurve22(mVertices[mVertexCount-1].x(), mVertices[mVertexCount-1].y(),  x2, y2,  x3, y3,  x4, y4);
  }
}

// Quadtatic
void pxCanvas2d::curveTo(double x2, double y2, double x3, double y3)  // uses preceeding vertex as (x1,y1)
{
  if (mVertexCount > 0)
  {
    addCurve2(mVertices[mVertexCount-1].x(), mVertices[mVertexCount-1].y(),  x2, y2,  x3, y3);
  }
}

void pxCanvas2d::closePath()
{
  if(mVertexCount > 0)
  {
    mVertices[mVertexCount].setX( mVertices[0].x() );
    mVertices[mVertexCount].setY( mVertices[0].y() );
    
    mVertexCount++;
  }
}

void pxCanvas2d::setMatrix(const pxMatrix4T<float>& m)
{
  mMatrix = m;
	mRasterizer.setMatrix(m);
}

void pxCanvas2d::matrix(pxMatrix4T<float>& m) const
{
  m = mMatrix;
}

void pxCanvas2d::setTextureMatrix(const pxMatrix4T<float>& m)
{
  mTextureMatrix = m;
  mRasterizer.setTextureMatrix(m);
}

void pxCanvas2d::textureMatrix(pxMatrix4T<float>& m) const
{
  m = mTextureMatrix;
}

#if 0
void pxCanvas2d::fill(bool time)
{
  extentLeft = extentTop = 1000000;
  extentRight = extentBottom = -1000000;

  if (mVertexCount > 1)
  {
    int i;
    for (i = 0; i < mVertexCount-1; i++)
    {
      pxVertex a;
      pxVertex b;

      if (mVertices[i].x < extentLeft) extentLeft = mVertices[i].x;
      if (mVertices[i].x > extentRight) extentRight = mVertices[i].x;
      if (mVertices[i].y < extentTop) extentTop = mVertices[i].y;
      if (mVertices[i].y > extentBottom) extentBottom = mVertices[i].y;

      mMatrix.multiply(mVertices[i], mMatrix, a);
      mMatrix.multiply(mVertices[i+1], mMatrix, b);
      mRasterizer.addEdge(a.x,a.y,b.x, b.y);
    }
    if (mVertices[i].x < extentLeft) extentLeft = mVertices[i].x;
    if (mVertices[i].x > extentRight) extentRight = mVertices[i].x;
    if (mVertices[i].y < extentTop) extentTop = mVertices[i].y;
    if (mVertices[i].y > extentBottom) extentBottom = mVertices[i].y;
  }
#if 1
  mRasterizer.setColor(mFillColor);
  pxVertex p1, p2, p3, p4;

#if 0
  p1.x = mRasterizer.mExtentLeft;
  p1.y = mRasterizer.mExtentTop;
  p2.x = mRasterizer.mExtentRight;
  p2.y = mRasterizer.mExtentTop;
  p3.x = mRasterizer.mExtentRight;
  p3.y = mRasterizer.mExtentBottom;
  p4.x = mRasterizer.mExtentLeft;
  p4.y = mRasterizer.mExtentBottom;
#else
  pxVertex t1;
  pxVertex t2;
  pxVertex t3;
  pxVertex t4;

  p1.x = extentLeft;
  p1.y = extentTop;
  p2.x = extentRight;
  p2.y = extentTop;
  p3.x = extentRight;
  p3.y = extentBottom;
  p4.x = extentLeft;
  p4.y = extentBottom;

  extentRight -= extentLeft;
  extentBottom -= extentTop;
  extentLeft = extentTop = 0;

  t1.x = extentLeft;
  t1.y = extentTop;
  t2.x = extentRight;
  t2.y = extentTop;
  t3.x = extentRight;
  t3.y = extentBottom;
  t4.x = extentLeft;
  t4.y = extentBottom;
#endif

#if 1
  pxVertex o1, o2, o3, o4;
  mMatrix.multiply(p1, mMatrix, o1);
  mMatrix.multiply(p2, mMatrix, o2);
  mMatrix.multiply(p3, mMatrix, o3);
  mMatrix.multiply(p4, mMatrix, o4);

  pxVertex n1, n2, n3, n4;

  mTextureMatrix.multiply(t1, mTextureMatrix, n1);
  mTextureMatrix.multiply(t2, mTextureMatrix, n2);
  mTextureMatrix.multiply(t3, mTextureMatrix, n3);
  mTextureMatrix.multiply(t4, mTextureMatrix, n4);

  mRasterizer.setTextureCoordinates(o1, o2, o3, o4, n1, n2, n3, n4);
#endif

  if (time)
  {
    double start = pxMilliseconds();
    mRasterizer.rasterize();
    double end = pxMilliseconds();
    printf("**Elapsed Time %gms FPS: %g\n", (end-start), 1000/(end-start));
  }
  else
  {
    mRasterizer.rasterize();
  }

  //mRasterizer.reset();
#else
  mRasterizer.reset();
#endif

}

#else

void pxCanvas2d::fill(bool time)
{
  mRasterizer.setColor(mFillColor);
//    setAlphaTexture(true);

//  double startEdge;
//  double endEdge;

#ifdef USE_PERF_TIMERS
  if (time)
  {
    startEdge = pxMilliseconds();
  }
#endif // USE_PERF_TIMERS

  extentLeft  = extentTop    =  1000000;
  extentRight = extentBottom = -1000000;

  if (mVertexCount > 1)
  {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // No TRANSFORMATION applied
    //
    if (mMatrix.isIdentity())
    {
      if (mRasterizer.texture())
      {
#if 1
        int i;
        for (i = 0; i < mVertexCount-1; i++)
        {
#if 1
          if (mVertices[i].x() < extentLeft)     extentLeft = mVertices[i].x();
          if (mVertices[i].x() > extentRight)   extentRight = mVertices[i].x();
          if (mVertices[i].y() < extentTop)       extentTop = mVertices[i].y();
          if (mVertices[i].y() > extentBottom) extentBottom = mVertices[i].y();
          
#endif

          mRasterizer.addEdge(mVertices[i].x(), mVertices[i].y(), mVertices[i+1].x(), mVertices[i+1].y() );
        }
#if 1
        if (mVertices[i].x() < extentLeft)     extentLeft = mVertices[i].x();
        if (mVertices[i].x() > extentRight)   extentRight = mVertices[i].x();
        if (mVertices[i].y() < extentTop)       extentTop = mVertices[i].y();
        if (mVertices[i].y() > extentBottom) extentBottom = mVertices[i].y();
        
#endif
#endif
      }
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      else
      {
#ifdef USE_PERF_TIMERS
        if (time)
        {
          startEdge = pxMilliseconds();
        }
#endif // USE_PERF_TIMERS
        
        pxVertex* vp = mVertices;
        pxVertex* vlast = vp + mVertexCount-1;

        while(vp < vlast)
        {
          mRasterizer.addEdge(vp->x(), vp->y(), (vp+1)->x(), (vp+1)->y() );
          
          if( (vp)->x() < extentLeft)     extentLeft = (vp)->x();
          if( (vp)->x() > extentRight)   extentRight = (vp)->x();
          if( (vp)->y() < extentTop)       extentTop = (vp)->y();
          if( (vp)->y() > extentBottom) extentBottom = (vp)->y();
          
          if( (vp + 1)->x() < extentLeft)     extentLeft = (vp + 1)->x();
          if( (vp + 1)->x() > extentRight)   extentRight = (vp + 1)->x();
          if( (vp + 1)->y() < extentTop)       extentTop = (vp + 1)->y();
          if( (vp + 1)->y() > extentBottom) extentBottom = (vp + 1)->y();
          
          vp++;
        }
      }
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Has TRANSFORMATION applied
    else
    {
      int i;
      for (i = 0; i < mVertexCount-1; i++)
      {
        if (mVertices[i].x() < extentLeft)     extentLeft = mVertices[i].x();
        if (mVertices[i].x() > extentRight)   extentRight = mVertices[i].x();
        if (mVertices[i].y() < extentTop)       extentTop = mVertices[i].y();
        if (mVertices[i].y() > extentBottom) extentBottom = mVertices[i].y();

        pxVertex a = mMatrix.multiply(mVertices[i]);
        pxVertex b = mMatrix.multiply(mVertices[i+1]);

        mRasterizer.addEdge(a.x(), a.y(), b.x(), b.y() );
      }
      
      if (mVertices[i].x() < extentLeft)     extentLeft = mVertices[i].x();
      if (mVertices[i].x() > extentRight)   extentRight = mVertices[i].x();
      if (mVertices[i].y() < extentTop)       extentTop = mVertices[i].y();
      if (mVertices[i].y() > extentBottom) extentBottom = mVertices[i].y();
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  }

#ifdef USE_PERF_TIMERS
  if (time)
  {
    endEdge = pxMilliseconds();
  }
#endif // USE_PERF_TIMERS

  if (mRasterizer.texture())
  {
#if 1
    pxVertex p1, p2, p3, p4;

    pxVertex t1;
    pxVertex t2;
    pxVertex t3;
    pxVertex t4;
    
    p1.setX( extentLeft   );
    p1.setY( extentTop    );
    p2.setX( extentRight  );
    p2.setY( extentTop    );
    p3.setX( extentRight  );
    p3.setY( extentBottom );
    p4.setX( extentLeft   );
    p4.setY( extentBottom );

    extentRight  -= extentLeft;
    extentBottom -= extentTop;
    extentLeft = extentTop = 0;
  
    t1.setX( extentLeft   );
    t1.setY( extentTop    );
    t2.setX( extentRight  );
    t2.setY( extentTop    );
    t3.setX( extentRight  );
    t3.setY( extentBottom );
    t4.setX( extentLeft   );
    t4.setY( extentBottom );

#if 1
    pxVertex o1 = mMatrix.multiply(p1);
    pxVertex o2 = mMatrix.multiply(p2);
    pxVertex o3 = mMatrix.multiply(p3);
    pxVertex o4 = mMatrix.multiply(p4);
    
    pxVertex n1 = mTextureMatrix.multiply(t1);
    pxVertex n2 = mTextureMatrix.multiply(t2);
    pxVertex n3 = mTextureMatrix.multiply(t3);
    pxVertex n4 = mTextureMatrix.multiply(t4);
    
    mRasterizer.setTextureCoordinates(o1, o2, o3, o4, n1, n2, n3, n4);
#endif
  }

  if (time)
  {
#ifdef USE_PERF_TIMERS
    double startScan = pxMilliseconds();
#endif
    mRasterizer.rasterize();
    
#ifdef USE_PERF_TIMERS
    
    double endScan = pxMilliseconds();
    double total   = (endEdge - startEdge) + (endScan - startScan);
    printf("Elapsed Total Fill: %gms FPS: %g\n", total, 1000/total);
    printf("\tElapsed Edge Setup: %gms FPS: %g\n", (endEdge-startEdge), 1000/(endEdge - startEdge));
    printf("\tElapsed Scanning: %gms FPS: %g\n",   (endScan-startScan), 1000/(endScan - startScan));
    
#endif // USE_PERF_TIMERS
  }
  else
  {

    mRasterizer.rasterize();
  }

  //mRasterizer.reset();
#else
  mRasterizer.reset();
#endif

}

#endif

#if 0

// Original code... 'Center' stroke only...
//
void pxCanvas2d::stroke()
{
  double halfStrokeWidth = mStrokeWidth / 2;
  pxFillMode oldFillMode = mRasterizer.fillMode();
  mRasterizer.setFillMode(fillWinding);
  
  if (mVertexCount > 1)
  {
    bool closed = (mVertices[0].x() == mVertices[mVertexCount-1].x() ) &&
                  (mVertices[0].y() == mVertices[mVertexCount-1].y() );
    
    pxVertex firstA1, firstA2;
    pxVertex lastB1,   lastB2;
    
    for (int i = 0; i < mVertexCount-1; i++)
    {
      // Get transformed verticies...
      //
      pxVertex a = mMatrix.multiply(mVertices[i]);
      pxVertex b = mMatrix.multiply(mVertices[i+1]);
      
      // Length of segment
      double dx = ( b.x() - a.x() );
      double dy = ( b.y() - a.y() );
      
      double len = pow(dx * dx + dy * dy, 0.5);
      
      if (len == 0)
        len = 1;
      
      // Normalize
      dx /= len;
      dy /= len;
      
      dx *= halfStrokeWidth;
      dy *= halfStrokeWidth;
      
      // Add edges for Stroke... with offsets
      //
      double ax, ay, bx, by;
      
      ax = a.x() + dy;   ay = a.y() - dx;
      bx = b.x() + dy;   by = b.y() - dx;
      
      mRasterizer.addEdge(ax, ay, bx, by);  // A
      
      bx = b.x() - dy;   by = b.y() + dx;
      ax = a.x() - dy;   ay = a.y() + dx;
      
      mRasterizer.addEdge(bx, by, ax, ay);  // B
      
      // Remember the START
      if (i == 0)
      {
        firstA1.setXY( a.x() + dy, a.y() - dx );
        firstA2.setXY( a.x() - dy, a.y() + dx );
      }
      
      // Join this segment to the last segment
      if (i > 0)
      {
        mRasterizer.addEdge(lastB1.x(), lastB1.y(),
                            a.x() + dy , a.y() - dx);
        
        mRasterizer.addEdge(a.x() - dy , a.y() + dx,
                            lastB2.x()  , lastB2.y() );
      }
      
      // Until the END
      lastB1.setXY( b.x() + dy, b.y() - dx );
      lastB2.setXY( b.x() - dy, b.y() + dx );
      
      if (closed == false)
      {
        // buttcaps
        if (i == 0)
        {
          mRasterizer.addEdge(a.x() - dy, a.y() + dx,
                              a.x() + dy, a.y() - dx);
        }
        
        if (i == mVertexCount-2)
        {
          mRasterizer.addEdge(b.x() + dy, b.y() - dx,
                              b.x() - dy, b.y() + dx);
        }
      }
      else
      {
        // close the shape
        if (i == mVertexCount-2)
        {
          mRasterizer.addEdge(lastB1.x(),   lastB1.y(), firstA1.x(), firstA1.y() );
          mRasterizer.addEdge(firstA2.x(), firstA2.y(), lastB2.x(),  lastB2.y() );
        }
      }
    }//FOR
  }
  
  mRasterizer.setColor(mStrokeColor);
  mRasterizer.rasterize();
  mRasterizer.setFillMode(oldFillMode);
}

#else

void pxCanvas2d::stroke()
{
  pxFillMode oldFillMode = mRasterizer.fillMode();
  mRasterizer.setFillMode(fillWinding);
  
 // stroke_center();
  switch(mStrokeType)
  {
    case StrokeType::inside: stroke_inside();   break;
    case StrokeType::outside: stroke_outside(); break;
    case StrokeType::center: stroke_center();   break;
  }
  
  mRasterizer.setColor(mStrokeColor);
  mRasterizer.rasterize();
  mRasterizer.setFillMode(oldFillMode);
}

#endif

void pxCanvas2d::stroke_inout(StrokeType t)
{
  int sign = (t == StrokeType::inside) ? 1 : -1;
  
  if (mVertexCount > 1)
  {
    // Closed Path ?   (First Pt. == Last Pt.)  ?
    //
    bool closed = (mVertices[0].x() == mVertices[mVertexCount-1].x() ) &&
                  (mVertices[0].y() == mVertices[mVertexCount-1].y() );
    
    pxVertex firstA1, firstA2;
    pxVertex  lastB1,  lastB2;
    
    for (int i = 0; i < mVertexCount-1; i++)
    {
      // Get transformed verticies...
      //
      pxVertex a = mMatrix.multiply(mVertices[i]);
      pxVertex b = mMatrix.multiply(mVertices[i+1]);
      
      // Length of segment
      double dx = ( b.x() - a.x() );
      double dy = ( b.y() - a.y() );
      
      double len = pow( (dx * dx) + (dy * dy), 0.5);
      
      if (len == 0)
      {
        len = 1;
      }
      
      // Normalize
      dx /= len;
      dy /= len;
      
      dx *= sign * mStrokeWidth;
      dy *= sign * mStrokeWidth;

      // Add edges for Stroke... with offsets
      //
      double ax, ay, bx, by;
      
      ax = a.x();   ay = a.y();
      bx = b.x();   by = b.y();
      
      mRasterizer.addEdge( ax, ay,  bx, by);  // A
      
      bx = b.x() - dy;   by = b.y() + dx;
      ax = a.x() - dy;   ay = a.y() + dx;
      
      mRasterizer.addEdge(bx, by, ax, ay);    // B

#if 1
      // Remember the START
      if (i == 0)
      {
        firstA1.setXY( a.x()     , a.y() );
        firstA2.setXY( a.x() - dy, a.y() + dx );
      }
      
      
      // Join this segment to the last segment
      if (i > 0)
      {
        mRasterizer.addEdge( lastB1.x(), lastB1.y(),   a.x(), a.y() );
        mRasterizer.addEdge( a.x() - dy, a.y() + dx,   lastB2.x(), lastB2.y() );
      }

      // Until the END...
      lastB1.setXY( b.x()     , b.y() );
      lastB2.setXY( b.x() - dy, b.y() + dx );
      
      if (closed == false)
      {
        // Buttcaps
        if (i == 0) // START
        {
          mRasterizer.addEdge(a.x() - dy, a.y() + dx,
                              a.x(), a.y() );//  a.x() + dy, a.y() - dx);
        }
        if (i == mVertexCount-2) // END
        {
          mRasterizer.addEdge(b.x(), b.y(), // + dy, b.y() - dx,
                              b.x() - dy, b.y() + dx);
        }
      }
      else
      {
        // ... Close the shape
        if (i == mVertexCount-2)
        {
          mRasterizer.addEdge( lastB1.x(),  lastB1.y(),
                              firstA1.x(), firstA1.y() );
          
          mRasterizer.addEdge(firstA2.x(), firstA2.y(),
                              lastB2.x(),  lastB2.y() );
        }
      }
      
#else
      // Join this segment to the last segment
      if (i > 0)
      {
        mRasterizer.addEdge( lastB1.x(), lastB1.y(),   a.x(), a.y() );
        mRasterizer.addEdge( a.x() - dy, a.y() + dx,   lastB2.x(), lastB2.y() );
      }
      
      if (closed == false)
      {
        // Buttcaps
        if (i == 0) // START
        {
          mRasterizer.addEdge(a.x(), a.y(),
                              a.x() + dy, a.y() - dx);
        }
        if (i == mVertexCount-2) // END
        {
          mRasterizer.addEdge(b.x() , b.y(), //+ dy, b.y() - dx,
                              a.x() - dy, a.y() + dx);//b.y() + dx);
        }
      }
      else
      {
        // Remember the START
        if (i == 0)
        {
          firstA1.setXY( a.x()     , a.y() );
          firstA2.setXY( a.x() - dy, a.y() + dx );
        }
        
        // Until the END...
        lastB1.setXY( b.x()     , b.y() );
        lastB2.setXY( b.x() - dy, b.y() + dx );
        
        // ... Close the shape
        if (i == mVertexCount-2)
        {
          mRasterizer.addEdge( lastB1.x(),  lastB1.y(),
                              firstA1.x(), firstA1.y() );
          
          mRasterizer.addEdge(firstA2.x(), firstA2.y(),
                               lastB2.x(),  lastB2.y() );
        }
      }
#endif
    }//FOR
  }
}


void pxCanvas2d::stroke_inside()
{
  stroke_inout(StrokeType::inside);
}

void pxCanvas2d::stroke_outside()
{
  stroke_inout(StrokeType::outside);
}

void pxCanvas2d::stroke_center()
{
  double halfStrokeWidth = mStrokeWidth / 2;
  
  if (mVertexCount > 1)
  {
    // Closed Path ?   (First Pt. == Last Pt.)  ?
    //
    bool closed = (mVertices[0].x() == mVertices[mVertexCount-1].x() ) &&
                  (mVertices[0].y() == mVertices[mVertexCount-1].y() );

    pxVertex firstA1, firstA2;
    pxVertex lastB1, lastB2;

    for (int i = 0; i < mVertexCount-1; i++)
    {
      // Get transformed verticies...
      //
      pxVertex a = mMatrix.multiply(mVertices[i]);
      pxVertex b = mMatrix.multiply(mVertices[i+1]);

      // Length of segment
      double dx = ( b.x() - a.x() );
      double dy = ( b.y() - a.y() );
      
      double len = pow( (dx * dx) + (dy * dy), 0.5);

      if (len == 0)
      {
        len = 1;
      }
      
      // Normalize
      dx /= len;
      dy /= len;

      dx *= halfStrokeWidth;
      dy *= halfStrokeWidth;
      
      //    ----------------- A  (above/outside)
      //
      //    .................
      //
      //    ----------------- B  (below/inside)
      //
      
      // Add edges for Stroke... with offsets
      //
      double ax, ay, bx, by;
      
      ax = a.x() + dy;   ay = a.y() - dx;
      bx = b.x() + dy;   by = b.y() - dx;
      
      mRasterizer.addEdge( ax, ay,  bx, by);  // A
      
      bx = b.x() - dy;   by = b.y() + dx;
      ax = a.x() - dy;   ay = a.y() + dx;
      
      mRasterizer.addEdge(bx, by, ax, ay);    // B
      
      // Remember the START
      if (i == 0)
      {
        firstA1.setXY( (a.x() + dy), (a.y() - dx) );
        firstA2.setXY( (a.x() - dy), (a.y() + dx) );
      }
      
      // Join this segment to the last segment
      if (i > 0)
      {
        mRasterizer.addEdge(  lastB1.x() ,  lastB1.y(),
                             a.x() + dy , a.y() - dx );
        
        mRasterizer.addEdge( a.x() - dy  , a.y() + dx,
                              lastB2.x()  , lastB2.y()  );
      }
      
      // Until the END...
      lastB1.setXY( (b.x() + dy), (b.y() - dx) );
      lastB2.setXY( (b.x() - dy), (b.y() + dx) );
      
      if (closed == false)
      {
        // Buttcaps
        if (i == 0) // START
        {
          mRasterizer.addEdge(a.x() - dy, a.y() + dx,
                              a.x() + dy, a.y() - dx);
        }
        if (i == mVertexCount-2) // END
        {
          mRasterizer.addEdge(b.x() + dy, b.y() - dx,
                              b.x() - dy, b.y() + dx);
        }
      }
      else
      {
        // ... Close the shape
        if (i == mVertexCount-2)
        {
          mRasterizer.addEdge(lastB1.x(),   lastB1.y(), firstA1.x(), firstA1.y() );
          mRasterizer.addEdge(firstA2.x(), firstA2.y(),  lastB2.x(),  lastB2.y() );
        }
      }
    }//FOR
  }
}


#ifdef RTPLATFORM_WINDOWS
rtString pxCanvas2d::font()
{
  return mFont;
}

void pxCanvas2d::setFont(const rtString& font)
{
  mFont = font;
}

#endif

void pxCanvas2d::setFontSize(double s)
{
  mFontSize = s;
}

void pxCanvas2d::setFillMode(const pxFillMode& mode)
{
  mRasterizer.setFillMode(mode);
}

void pxCanvas2d::setFillColor(int gray, int a)
{
  mFillColor.r = mFillColor.g = mFillColor.b = gray;
  mFillColor.a = a;
}

void pxCanvas2d::setFillColor(int r, int g, int b, int a)
{
  mFillColor.r = r;
  mFillColor.g = g;
  mFillColor.b = b;
  mFillColor.a = a;
}

void pxCanvas2d::setFillColor(const pxColor& c)
{
  mFillColor = c;
}

void pxCanvas2d::setStrokeColor(const pxColor& c)
{
  mStrokeColor = c;
}

void pxCanvas2d::setStrokeColor(int gray, int a)
{
  mStrokeColor.r = mStrokeColor.g = mFillColor.b = gray;
  mStrokeColor.a = a;
}

pxPixel pxCanvas2d::fillColor()
{
  return mFillColor;
}

void pxCanvas2d::setStrokeColor(int r, int g, int b, int a)
{
  mStrokeColor.r = r;
  mStrokeColor.g = g;
  mStrokeColor.b = b;
  mStrokeColor.a = a;
}

pxPixel pxCanvas2d::strokeColor()
{
  return mStrokeColor;
}

double pxCanvas2d::alpha() const
{
  return mRasterizer.alpha();
}

void pxCanvas2d::setAlpha(double alpha)
{
  mRasterizer.setAlpha(alpha);
}


void pxCanvas2d::setStrokeWidth(double w)
{
  mStrokeWidth = w;
}

void pxCanvas2d::setStrokeType(StrokeType t)
{
  mStrokeType = t;
}

#define  QUAD_MAX_DEPTH    7 // 3
#define CUBIC_MAX_DEPTH    7 // 3

// Quadratic
void pxCanvas2d::addCurve(double x1, double y1, double x2, double y2, double x3, double y3)
{
  addCurve(x1, y1, x2, y2, x3, y3, 0);
}

void pxCanvas2d::addCurve(double x1, double y1, double x2, double y2, double x3, double y3, int depth)
{
  if (depth > QUAD_MAX_DEPTH)
  {
    mRasterizer.addEdge(x1, y1, x2, y2);
    mRasterizer.addEdge(x2, y2, x3, y3);
    return;
  }

  // subdivide
  double x15 = (x1 + x2) / 2.0;
  double y15 = (y1 + y2) / 2.0;
  double x25 = (x2 + x3) / 2.0;
  double y25 = (y2 + y3) / 2.0;

  double x2p = (x15 + x25) / 2.0;
  double y2p = (y15 + y25) / 2.0;

  addCurve(x1,  y1,  x15, y15, x2p, y2p, depth+1);
  addCurve(x2p, y2p, x25, y25, x3,  y3,  depth+1);
}


// Quadratic
void pxCanvas2d::addCurve2(double x1, double y1, double x2, double y2, double x3, double y3)
{
  addCurve2(x1, y1, x2, y2, x3, y3, 0);
}

void pxCanvas2d::addCurve2(double x1, double y1, double x2, double y2, double x3, double y3, int depth)
{
  if (depth > 3)
  {
#if 0
    mRasterizer.addEdge(x1, y1, x2, y2);
    mRasterizer.addEdge(x2, y2, x3, y3);
#else
    lineTo(x2, y2);
    lineTo(x3, y3);
#endif
    return;
  }

  // Subdivide ... Casteljau algorithm
  double x12 = (x1 + x2) / 2.0;
  double y12 = (y1 + y2) / 2.0;
  
  double x23 = (x2 + x3) / 2.0;
  double y23 = (y2 + y3) / 2.0;

  double x123 = (x12 + x23) / 2.0;
  double y123 = (y12 + y23) / 2.0;

  addCurve2(x1,   y1,   x12, y12, x123, y123, depth+1);
  addCurve2(x123, y123, x23, y23, x3,   y3,   depth+1);
}


// Cubic
void pxCanvas2d::addCurve22(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
  addCurve22(x1, y1, x2, y2, x3, y3, x4, y4, 0);
}


void pxCanvas2d::addCurve22(double x1, double y1,  double x2, double y2, double x3, double y3,  double x4, double y4, unsigned depth)
{
  if (depth > CUBIC_MAX_DEPTH)
  {
#if 0
    mRasterizer.addEdge(x1, y1, x2, y2);
    mRasterizer.addEdge(x2, y2, x3, y3);
#else
    lineTo(x2, y2);
    lineTo(x3, y3);
#endif
    return;
  }
  
  // Subdivide ... Casteljau algorithm
  double x12   = (x1 + x2) / 2;
  double y12   = (y1 + y2) / 2;
  double x23   = (x2 + x3) / 2;
  double y23   = (y2 + y3) / 2;
  double x34   = (x3 + x4) / 2;
  double y34   = (y3 + y4) / 2;
  
  double x123  = (x12 + x23) / 2;
  double y123  = (y12 + y23) / 2;
  double x234  = (x23 + x34) / 2;
  double y234  = (y23 + y34) / 2;
  
  double x1234 = (x123 + x234) / 2;
  double y1234 = (y123 + y234) / 2;
  
  addCurve22(x1,    y1,    x12,  y12,  x123, y123, x1234, y1234, depth + 1);
  addCurve22(x1234, y1234, x234, y234, x34,  y34,  x4,    y4,    depth + 1);
}

void pxCanvas2d::setClip(const pxRect* r)
{
  mRasterizer.setClip(r);
}

#ifdef RTPLATFORM_WINDOWS
inline double pxCanvas2d::convertFixToFloat(const FIXED& fx)
{
  return (double)fx.value + (float)fx.fract/65536;
}



void pxCanvas2d::calcTextScale(int a, int ascent)
{
//    mTextScale = 2048/(float)mFontSize * ((float)ascent / (float)2048);
  //mTextScale = (float)ascent/2048 * mFontSize;
  //mTextScale = 2048/(float)mFontSize;
  //mTextScale = 43;
  // mTextScale = mFontSize * 2048/ascent;
  mTextScale = (float)2048 / ((2048 * mFontSize) / ascent);
  mBaseLineAdjust = (int)(a/mTextScale);
  //mBaseLineAdjust = 0;
  //mTextScale = 2048/mFontSize;
}

void pxCanvas2d::TextMoveTo(double x, double y)
{
  lastX = x/mTextScale;
  lastY = (2048-y)/mTextScale;
}

void pxCanvas2d::TextLineTo(double x, double y)
{
  x = x/mTextScale;
  y = (2048-y)/mTextScale;

  mRasterizer.addEdge(lastX + textX, lastY+textY-mBaseLineAdjust, x+textX, y+textY-mBaseLineAdjust);
  lastX = x;
  lastY = y;
}

void pxCanvas2d::TextCurveTo(double x2, double y2, double x3, double y3)
{
  x2 = x2 / mTextScale;
  y2 = (2048-y2)/mTextScale;
  x3 = x3/mTextScale;
  y3 = (2048-y3)/mTextScale;

  addCurve(lastX+textX, lastY+textY-mBaseLineAdjust, x2+textX, y2+textY-mBaseLineAdjust, x3+textX, y3+textY-mBaseLineAdjust);

  lastX = x3;
  lastY = y3;
}

pxError pxCanvas2d::drawChar(const wchar_t c)
{
  pxError e = RT_ERROR;

  HDC dc = GetDC(NULL);

  GLYPHMETRICS glyphMetrics;
    
  MAT2 mat = {0};

  mat.eM11.value = 1;
  mat.eM12.value = 0;
  mat.eM21.value = 0;
  mat.eM22.value = 1;

  long bufferSize;

	HFONT	font = ::CreateFont(-2048, 0, 0, 0, 0, FALSE, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
                            DEFAULT_QUALITY, DEFAULT_PITCH, mFont);
  if (font)
  {

    HGDIOBJ oldFont = SelectObject(dc, font);

    TEXTMETRIC tm;
    if (GetTextMetrics(dc, &tm))
    {
      calcTextScale(tm.tmAscent+tm.tmInternalLeading, tm.tmAscent-tm.tmInternalLeading-tm.tmExternalLeading);
    }
    

    bufferSize	= ::GetGlyphOutline(dc, c, GGO_NATIVE, &glyphMetrics, 0, NULL, &mat);

    unsigned char* buffer = (unsigned char*)malloc(bufferSize);

    if (::GetGlyphOutline(dc, c, GGO_NATIVE, &glyphMetrics, bufferSize, buffer, &mat) != GDI_ERROR)
    {
      int s = 0;
      while(s < bufferSize)
      {
        TTPOLYGONHEADER *hdr = (TTPOLYGONHEADER*)(buffer+s);

        TextMoveTo(convertFixToFloat(hdr->pfxStart.x), convertFixToFloat(hdr->pfxStart.y));

        int s2 = sizeof(TTPOLYGONHEADER);
        while(s2 < hdr->cb)
        {
          TTPOLYCURVE *c = (TTPOLYCURVE*)(buffer+s+s2);
          switch(c->wType)
          {
          case TT_PRIM_LINE:
          {
            //OutputDebugString(L"Line\n");

            for (int i = 0; i < c->cpfx; i++)
            {
              TextLineTo(convertFixToFloat(c->apfx[i].x), convertFixToFloat(c->apfx[i].y));
            }
          }
          break;
          case TT_PRIM_QSPLINE:
          {
            //OutputDebugString(L"QSPLINE\n");
            for (int i = 0; i < c->cpfx-1; i++)
            {
              double x3, y3;
              if (i < c->cpfx-2)
              {
                x3 = (convertFixToFloat(c->apfx[i].x) + convertFixToFloat(c->apfx[i+1].x))/2;
                y3 = (convertFixToFloat(c->apfx[i].y) + convertFixToFloat(c->apfx[i+1].y))/2;
              }
              else
              {
                x3 = convertFixToFloat(c->apfx[i+1].x);
                y3 = convertFixToFloat(c->apfx[i+1].y);
              }
              TextCurveTo(convertFixToFloat(c->apfx[i].x), convertFixToFloat(c->apfx[i].y), 
                          x3, y3);
            }
          }
          break;
          case TT_PRIM_CSPLINE:
            //OutputDebugString(L"CSPLINE\n");
            break;
          default:
            //OutputDebugString(L"blah\n");
            break;
          }
          s2 += sizeof(TTPOLYCURVE) + (c->cpfx -1) * sizeof(POINTFX);
        }

        TextLineTo(convertFixToFloat(hdr->pfxStart.x), convertFixToFloat(hdr->pfxStart.y));
        s += s2;
      }
    }

    //fill();
    //setFillMode(fillEvenOdd);
    mRasterizer.setFillMode(fillWinding);
    mRasterizer.rasterize();
    //rasterizeSolid();
    e = PX_OK;

    SelectObject(dc, oldFont);
    DeleteObject(font);
    ::ReleaseDC(NULL, dc);
  }
  textX += glyphMetrics.gmCellIncX/mTextScale;
  return e;
}

void pxCanvas2d::drawText(const wchar_t* t, double x, double y)
{
  // Is overall alignment... compare with flash etc... 
#if 0
  textX = x-0.5;
  textY = y-0.5;
#else
  textX = x;
  textY = y;
#endif
  while(*t) drawChar(*t++);
}
#endif

pxBuffer* pxCanvas2d::texture() const
{
  return mRasterizer.texture();
}

void pxCanvas2d::setTexture(pxBuffer* texture)
{
	if (texture && (texture->width() && texture->height()))
		mRasterizer.setTexture(texture);
	else
		mRasterizer.setTexture(NULL);
}

void pxCanvas2d::clear()
{
  mRasterizer.clear();
}

bool pxCanvas2d::alphaTexture() const    { return mRasterizer.alphaTexture(); }
void pxCanvas2d::setAlphaTexture(bool f) { mRasterizer.setAlphaTexture(f); }


void pxCanvas2d::roundRect(double x, double y, double w, double h, double rx, double ry)
{
  newPath();
	moveTo(x+rx, y);
  
	lineTo(x+w-rx, y);
	curveTo(x+w, y, x+w, y+ry);
  
	lineTo(x+w, y+h-ry);
	curveTo(x+w, y+h, x+w-rx, y+h);
  
	lineTo(x+rx, y+h);
	curveTo(x, y+h, x, y+h-rx);
  
	lineTo(x, y+ry);
	curveTo(x, y, x+rx, y);
  
  closePath();
}

void pxCanvas2d::rectangle(double x1, double y1, double x2, double y2)
{
	newPath();
	moveTo(x1, y1);
	lineTo(x2, y1);
	lineTo(x2, y2);
	lineTo(x1, y2);
//	lineTo(x1, y1);
	closePath();
}

/*
pxOffscreen* pxCanvas2d::offscreen()
{
  return mOffscreen;
}


void pxCanvas2d::needsRedraw()
{
  mNeedsRedraw = true;
}

void pxCanvas2d::draw()
{
  if(mOffscreen)
  {
    if(mNeedsRedraw)
    {
      mNeedsRedraw = false;
      mTexture = context.createTexture(*mOffscreen); // UPLOAD TO GPU
    }
    
    static pxTextureRef nullMaskRef;
    
    context.drawImage(0, 0, mw, mh,
                      mTexture, nullMaskRef,
                      false, NULL,
                      pxConstantsStretch::NONE,
                      pxConstantsStretch::NONE);
  }
}
*/