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

#include "pxTimer.h"
#include "pxRasterizer.h"

#include "xs_Core.h"
#include "xs_Float.h"
//#include "rtLog.h"

#include <algorithm>

#define MINEDGES 200000

#define UVFIXED 65536
#define UVFIXEDSHIFT 16

#define FIXEDFAC 65536
#define FIXEDSHIFT 16

#define FIXEDPOINTEDGES
#define EDGEBUCKETS

#define FIXEDSCANLINE(y) (y >> fixedScanlineShift)
#define FIXEDX(x) (x >> (12))  // hardcoded to 4 bits


#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#endif

#define SAFE_DELETE(p)  if(p) { delete p; p = NULL; };

//#define OLDTEXTURE

template<typename T>
inline T pxWrap(T v, T min, T max)
{
  return (v>max)?min:((v<min)?max:v);
}

uint32_t overSampleShift;
uint32_t fixedScanlineShift;
uint32_t xShift;

// Design Notes and Assumptions
// * AddEdge, Rasterize/Reset must be called "atomically".  No pxBuffer changes in width or height can be made after the first call
// to AddEdge and until after Reset has been called.  In addition setClip should not be called during this timeframe as well.
//

// BIGBUCKETS
/*
  #if 0
  #define BUCKET_EDGE_COUNT   800
  #define BUCKET_COUNT        1200
  #else if 0 // stress
  #define BUCKET_EDGE_COUNT   1
  #define BUCKET_COUNT        1
  #elif 1
*/
#define BUCKET_EDGE_COUNT   10
#define BUCKET_COUNT        1200
//#endif




class span
{
public:
  int32_t x0, x1;
  span* next;
};

class pxSpanBuffer
{
public:
  pxSpanBuffer(): mNumRows(0), mCurrentRow(0), mSpanRows(NULL), mFreeSpans(NULL), mCurrentClipSpan(NULL),
                  mCurrentClipX0(0), mCurrentClipX1(0), mCurrentClipComplete(true)
  {
    spanCount= 0;
  }

  ~pxSpanBuffer()
  {
    // This will move all spans into the freelist
    term();

    // free freelist
    span* s = mFreeSpans;
    while(s)
    {
      span* lastSpan = s;
      s = s->next;
      delete lastSpan;
    }
    mFreeSpans = NULL;
  }

  pxError init(const pxRect& r)
  {
    //printf("spanCount %ld\n", spanCount);
    term();

    //mWidth = w;
    //mHeight = h;
    mBounds = r;
    mNumRows = mBounds.bottom()-mBounds.top();

    //mFreeSpans = NULL;

    mSpanRows = new span*[mNumRows];

#if 1
    for (int i = 0; i < mNumRows; i++)
    {
      span* s = newSpan();
      s->x0 = mBounds.left();
      s->x1 = mBounds.right()-1;
      s->next = NULL;
      mSpanRows[i] = s;
    }
#endif
    return PX_OK;
  }

  pxError term()
  {
#if 1
    for (int i = 0; i < mNumRows; i++)
    {
      span* s = mSpanRows[i];
      while (s)
      {
        span* lastSpan = s;
        s = s->next;
        freeSpan(lastSpan);
      }
    }
    delete [] mSpanRows;
    mSpanRows = NULL;
    //mWidth = mHeight = 0;
    mNumRows = 0;
#endif
    return PX_OK;
  }

#if 0
  int32_t height()
  {
    return mHeight;
  }
#endif

#if 0
  void clear()
  {
    for (int i = 0; i < mHeight; i++)
    {
      span* s = mSpanRows[i];
      while(s)
      {
        span* lastspan = s;
        s = s->next;
        freeSpan(lastspan);
      }

      s = newSpan();
      s->x0 = 0;
      s->x1 = mWidth;;
      s->next = NULL;
      mSpanRows[i] = s;
    }
  }
#endif

  inline span* newSpan(void)
  {
    span *s;
    if ((s = mFreeSpans) == NULL)
    {
      s = new span;
    }
    else
    {
      mFreeSpans = s->next;
      spanCount++;
    }
    return s;
  }

  inline void freeSpan(span *s)
  {
    if (s)
    {
      spanCount--;
      s->next=mFreeSpans;
      mFreeSpans=s;
    }
  }


  inline int32_t currentRow()
  {
    return mCurrentRow+mBounds.top();
  }

  inline void setCurrentRow(int32_t row)
  {
#if 1
    row = row-mBounds.top();
    mCurrentRow = pxClamp<int32_t>(row, 0, mBounds.bottom()-mBounds.top());
#else
    mCurrentRow = row-mBounds.top();
#endif
  }

  // The current row has no free space left
  inline bool isCurrentRowFull()
  {
    return (mSpanRows[mCurrentRow] == NULL);
  }

  // Marks the supplied rectangle as occluded
  void addSpan(int32_t left, int32_t right)
  {
    if (mCurrentRow >= 0)
    {
      span *currentRow = mSpanRows[mCurrentRow];

      if (currentRow)
      {
        // http://www.gripho.it/sbuffer3.en.html
        span *old,*current,*n;
        for (old=NULL,current=currentRow;
             current!=NULL;
             old=current,current=current->next)
        {
          if (current->x1 <= left) // Case 1
            continue;

          if (current->x0 < left)
          {
            if (current->x1 <= right) // Case 2
            {
              //DrawPart(y,left,current->x1);
              current->x1=left;
            }
            else // Case 3
            {
              //DrawPart(y,xa,xb);
              n = newSpan();
              n->next = current->next;
              current->next = n;
              n->x1 = current->x1;
              current->x1 = left;
              n->x0 = right;
              return;
            }
          }
          else
          {
            if (current->x0 >= right) // Case 6
              return;

            if (current->x1 <= right) // Case 4
            {
              //DrawPart(y,current->x0,current->x1);
              n = current->next;
              freeSpan(current);

              if (old)
                old->next=n;
              else
                mSpanRows[mCurrentRow] = n;

              current=n;

              if (current==NULL)
                return;
            }
            else // Case 5
            {
              //DrawPart(y,current->x0,xb);
              current->x0 = right;
            }
          }
        }
      }
    }
  }

  void startClipRow(int32_t row)
  {
    mCurrentClipSpan = NULL;
    if (row >= mBounds.top() && row < mBounds.bottom())
    {
      mCurrentClipSpan = mSpanRows[row-mBounds.top()];
    }
  }

  void startClipSpan(int32_t x0, int32_t x1)
  {
    mCurrentClipX0 = x0;
    mCurrentClipX1 = x1;
    mCurrentClipComplete = false;
  }

  inline bool getClip(int32_t& x0, int32_t& x1)
  {
    if (!mCurrentClipComplete && mCurrentClipSpan)
    {
      while(mCurrentClipSpan)
      {

        if (mCurrentClipX0 < mCurrentClipSpan->x0 && mCurrentClipX1 < mCurrentClipSpan->x0)
          break;
        else

          if (mCurrentClipX0 <= mCurrentClipSpan->x0)
          {
            x0 = mCurrentClipSpan->x0;
            if (mCurrentClipX1 <= mCurrentClipSpan->x1)
            {
              x1 = mCurrentClipX1;
              mCurrentClipComplete = true;
              return true;
            }
            else
            {
              x1 = mCurrentClipSpan->x1;
              mCurrentClipSpan = mCurrentClipSpan->next;
              return true;
            }
          }
          else if (mCurrentClipX0 <= mCurrentClipSpan->x1)
          {
            x0 = mCurrentClipX0;
            if (mCurrentClipX1 <= mCurrentClipSpan->x1)
            {
              x1 = mCurrentClipX1;
              mCurrentClipComplete = true;
              return true;
            }
            else
            {
              x1 = mCurrentClipSpan->x1;
              mCurrentClipSpan = mCurrentClipSpan->next;
              return true;
            }
          }
          else mCurrentClipSpan = mCurrentClipSpan->next;
      }
    }
    return false;
  }


private:
  //int32_t mWidth, mHeight;
  pxRect mBounds;
  int32_t mNumRows;

  int32_t mCurrentRow;

  span** mSpanRows;
  span* mFreeSpans;

  span* mCurrentClipSpan;
  int32_t mCurrentClipX0, mCurrentClipX1;
  bool mCurrentClipComplete;
  int32_t spanCount;
  // int32_t currentClipX;
};


pxSpanBuffer mSpanBuffer;

struct edge
{
#if 1
  int32_t mX1, mX2;
  int32_t mY1;
#endif
  int32_t mY2;

  int32_t mHeight;
  int32_t mXCurrent;

  int32_t mXDelta;

#ifndef FIXEDPOINTEDGES
  int32_t mError;
  int32_t mErrorDelta;
  char mSide;
#endif

  char mLeft;
};


#if 1

struct edgeBucket
{
  edge edges[BUCKET_EDGE_COUNT];
  edgeBucket* nextBucket;
  edge* edgePos;
  edge* edgeEnd;

  inline void init()
  {
    edgePos = edges;
    edgeEnd = edges+BUCKET_EDGE_COUNT;
    nextBucket = NULL;
  }

  inline edge* getNewEdge()
  {
    if (edgePos < edgeEnd)
      return edgePos++;
    else
      return NULL;
  }
};

class edgePool
{
public:
  inline void init()
  {
    bucketPos = buckets;
    bucketEnd = buckets+BUCKET_COUNT;
  }

  inline edgeBucket* getNewBucket()
  {
    if (bucketPos < bucketEnd)
    {
      bucketPos->init();
      return bucketPos++;
    }
    else
      return NULL;
  }

public:
  edgeBucket buckets[BUCKET_COUNT];

  edgeBucket* bucketPos;
  edgeBucket* bucketEnd;

  edgePool* nextPool;
};

class edgePoolManager
{
public:
  edgePoolManager() : headPool(NULL), tailPool(NULL), freePool(NULL)
  {
  }

  ~edgePoolManager()
  {
    while(headPool)
    {
      edgePool* p = headPool->nextPool;
      SAFE_DELETE(headPool);

      headPool = p;
    }

    while(tailPool)
    {
      edgePool* p = tailPool->nextPool;
      SAFE_DELETE(tailPool);

      tailPool = p;
    }

    while(freePool)
    {
      edgePool* p = freePool->nextPool;
      SAFE_DELETE(freePool);

      freePool = p;
    }
  }

  inline void reset()
  {
    if (tailPool)
    {
      tailPool->nextPool = freePool;
      tailPool = NULL;
      freePool = headPool;
      headPool = NULL;
    }
  }

  inline edgeBucket* getNewBucket()
  {
    edgeBucket* newBucket = NULL;

    if (tailPool != NULL)
    {
      newBucket = tailPool->getNewBucket();
    }

    if (newBucket)
    {
      return newBucket;
    }
    else
    {
      edgePool* newPool = NULL;
      if (freePool != NULL)
      {
        newPool  = freePool;
        freePool = newPool->nextPool;
      }
      else
      {
        newPool = new edgePool();
      }

      newPool->init();

      // Link it in
      if (!headPool)
      {
        headPool = newPool;
      }

      if (tailPool)
      {
        tailPool->nextPool = newPool;
      }

      tailPool = newPool;

      if (tailPool != NULL)
      {
        newBucket = tailPool->getNewBucket();
      }
    }

    return newBucket;
  }

  bool getTwoEdges(edge*& e1, edge*& e2)
  {
    edge* tb = headPool->buckets[0].edges;
    edge* te = headPool->buckets[0].edgePos;

    if (te - tb == 2)
    {
      e1 = &tb[0];
      e2 = &tb[1];
      return true;
    }
    return false;
  }

private:
  edgePool* headPool;
  edgePool* tailPool;
  edgePool* freePool;
};

edgePoolManager mPoolManager;

class edgeLine
{
public:
  inline edgeLine()
  {
    headBucket = NULL;
    tailBucket = NULL;
  }

  inline void init()
  {
    headBucket = NULL;
    tailBucket = NULL;
  }


  inline edge* getNewEdge()
  {
    edge* e = NULL;
    if (tailBucket != NULL)
      e = tailBucket->getNewEdge();

    if (e)
      return e;
    else
    {
      edgeBucket* newBucket = mPoolManager.getNewBucket();

      // Link it in
      if (!headBucket)
        headBucket = newBucket;
      if (tailBucket)
        tailBucket->nextBucket = newBucket;

      tailBucket = newBucket;

      if (tailBucket != NULL)
        e = tailBucket->getNewEdge();
    }

    return e;
  }

public:
  edgeBucket *headBucket;
  edgeBucket* tailBucket;
};
#endif


struct textureedge
{
  int32_t mX1, mX2;
  int32_t mY1, mY2;

//  int32_t mWidth, mHeight;

//  int32_t mXCurrent;

//  int32_t mXDelta;
//  int32_t mError;
//  int32_t mErrorDelta;

//  int32_t mSide;
//  int32_t mLeft;

#if 1
  // int32_t deltaU;
  // int32_t deltaV;

  int32_t mU1, mU2;
  int32_t mV1, mV2;

  int32_t mdu, mdv;
  int32_t mdx, mdy;

  int32_t mCurrentX;
 // int32_t mCurrentY;

  int32_t mCurrentU;
  int32_t mCurrentV;
#endif
};

struct endPoint
{
  int32_t mY;
  textureedge *mEdge;
};

int compareEndPoints(const void* e1, const void* e2)
{
  return (((endPoint*)e1)->mY - ((endPoint*)e2)->mY);
}

int sortStarts(const void* e1, const void* e2)
{
  return ((*(edge**)e1)->mY1 - (*(edge**)e2)->mY1);
}

int sortEnds(const void* e1, const void* e2)
{
  return ((*(edge**)e1)->mY2 - (*(edge**)e2)->mY2);
}

inline bool sortStarts2(const edge* t1, const edge* t2)
{
  return (t1->mY1 < t2->mY1);
}

inline bool sortEnds2(const edge* t1, const edge* t2)
{
  return (t1->mY2 < t2->mY2);
}

struct CompareStarts {
  inline bool operator()(edge* e1, edge* e2) {
    return e1->mY1 < e2->mY1;
  }
};

struct CompareEnds {
  inline bool operator()(edge* e1, edge* e2) {
    return e1->mY2 < e2->mY2;
  }
};

struct CompareX {
  inline bool operator()(edge* e1, edge* e2) {
    return e1->mXCurrent < e2->mXCurrent;
  }
};

#ifdef EDGECLEANUP
#if 0
class edgeManager
{
public:
  edgeManager()
  {
    mEdgeArray = new edge[MINEDGES];
    mStarts = new edge*[MINEDGES];
    mEnds = new edge*[MINEDGES];
    mEdgeCount = 0;
  }

  ~edgeManager()
  {
    delete [] mEdgeArray;
    mEdgeArray = NULL;
    
    SAFE_DELETE(mStarts);
    SAFE_DELETE(mEnds);
  }

  inline void reset()
  {
    mEdgeCount = 0;
    startTail = mStarts;
    endTail = mEnds;
    resetStart();
    resetEnd();
  }

  inline int32_t getEdgeCount()
  {
    return mEdgeCount;
  }

  inline void resetStart()
  {
    mCurrentStart = 0;
  }

  inline edge* getCurrentStart()
  {
    if (mCurrentStart < mEdgeCount) return mStarts[mCurrentStart];
    else return NULL;
  }

  inline void advanceStart()
  {
    mCurrentStart++;
  }

  inline void resetEnd()
  {
    mCurrentEnd = 0;
  }

  inline edge* getCurrentEnd()
  {
    if (mCurrentEnd < mEdgeCount) return mEnds[mCurrentEnd];
    else return NULL;
  }

  inline void advanceEnd()
  {
    mCurrentEnd++;
  }

  inline edge* addEdge(int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool left)
  {
    edge *e = (edge*)mEdgeArray+mEdgeCount;
    mStarts[mEdgeCount] = mEnds[mEdgeCount] = e;

    e->mX1 = x1;
    e->mY1 = y1;
    e->mX2 = x2;
    e->mY2 = y2;
    e->mLeft = left?1:-1;

    mEdgeCount++;

    return e;
  }

  inline sortEdges()
  {
    std::sort(mStarts, mStarts + mEdgeCount, CompareStarts());
    std::sort(mEnds, mEnds + mEdgeCount, CompareEnds());
  }

private:
  edge* mEdgeArray;
  edge** mStarts;
  edge** mEnds;
  edge** startTail;
  edge** endTail;
  int mEdgeCount;
  int32_t mCurrentStart;
  int32_t mCurrentEnd;
};
#elif 0 // edgeArray version

struct scanlineDesc
{
  int32_t generationId;
  int32_t scanlineEdgeCount;
  edge** scanlineEdges;
};

class edgeManager
{
public:
  edgeManager()
  {
    mEdgeArray = new edge[MINEDGES];

    mStartLines = new scanlineDesc[16000];

    for (int i = 0; i < 16000; i++)
    {
      mStartLines[i].generationId = 0;
      mStartLines[i].scanlineEdgeCount = 0;
      mStartLines[i].scanlineEdges = new edge*[1000];
    }

    mEdgeCount = 0;

    mFirstStart = 16000;
    mLastStart = -1;
  }

  ~edgeManager()
  {
#if 0
    delete [] mEdgeArray;
    mEdgeArray = NULL;
    
    SAFE_DELETE(mStarts);
    SAFE_DELETE(mEnds);
#endif
  }

  inline void reset()
  {
    mEdgeCount = 0;

    for (int i = mFirstStart; i <= mLastStart; i++)
    {
      mStartLines[i].scanlineEdgeCount = 0;
    }

    mFirstStart = 16000;
    mLastStart = -1;
  }

  inline int32_t getEdgeCount()
  {
    return mEdgeCount;
  }

  inline void resetStart()
  {
    mCurrentStartLine = mFirstStart;
    mCurrentStartEdge = 0;
  }

  inline edge* getCurrentStart()
  {
    if (mCurrentStartLine <= mLastStart)
      return mStartLines[mCurrentStartLine].scanlineEdges[mCurrentStartEdge];
    else return NULL;
  }

  void advanceStartLine()
  {
    mCurrentStartEdge = 0;
    for (int32_t i = mCurrentStartLine+1; i <= mLastStart; i++)
    {
      if (mStartLines[i].scanlineEdgeCount > 0)
      {
        mCurrentStartLine = i;
        return;
      }
    }
    mCurrentStartLine = 1601;
  }

  inline void advanceStart()
  {
    if (mCurrentStartEdge < mStartLines[mCurrentStartLine].scanlineEdgeCount-1)
      mCurrentStartEdge++;
    else
      advanceStartLine();
  }

  inline edge* addEdge(int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool left)
  {
    edge *e = mEdgeArray+mEdgeCount;

    e->mX1 = x1;
    e->mY1 = y1;
    e->mX2 = x2;
    e->mY2 = y2;

    e->mLeft = left?1:-1;

    mStartLines[y1].scanlineEdges[mStartLines[y1].scanlineEdgeCount] = e;
    mStartLines[y1].scanlineEdgeCount++;

    if (y1 < mFirstStart)
      mFirstStart = y1;
    if (y1 > mLastStart)
      mLastStart = y1;

    mEdgeCount++;

    return e;
  }

  inline sortEdges()
  {
    resetStart();
  }

public:
  edge* mEdgeArray;
  int mEdgeCount;
  int32_t mCurrentStartLine;
  int32_t mCurrentStartEdge;
  scanlineDesc* mStartLines;

  int32_t mFirstStart, mLastStart;
};


#elif 1 // current version
//*************************************

#ifndef EDGEBUCKETS
struct scanlineDesc
{
  int32_t scanlineEdgeCount;
  edge* tail;
  edge* scanlineEdges;
};
#endif

class edgeManager
{
public:
  edgeManager() : mEdgeCount(0), mCurrentStartLine(0), mCurrentStartEdge(0),
                  mStartLines(NULL), mFirstStart(0), mLastStart(0), mMaxScanlines(0)
  {
#ifdef EDGEBUCKETS
#if 0
    mStartLines = new edgeLine[8000];
    for (int i = 0; i < 8000; i++)
    {
      mStartLines[i].init();
    }
#endif
#else
    mStartLines = new scanlineDesc[8000];
    for (int i = 0; i < 8000; i++)
    {
      mStartLines[i].scanlineEdgeCount = 0;
      mStartLines[i].scanlineEdges = new edge[1000];
    }
#endif
    mEdgeCount = 0;

    mFirstStart = mMaxScanlines;
    mLastStart = -1;
  }
  ~edgeManager()
  {

    term();
  }

  inline void init(uint32_t maxScanlines)
  {
    if (maxScanlines > mMaxScanlines)
    {
      //            mPoolManager.reset();
      if (mStartLines)
      {
        delete [] mStartLines;
      }
      mStartLines = new edgeLine[maxScanlines+100];  // BUGBUG padding bogus... clean up  somehow trashing heap
#if 0
      for (int i = 0; i < maxScanlines; i++)
      {
        mStartLines[i].init();
      }
#endif
      mMaxScanlines = maxScanlines;
    }
  }

  void term()
  {
#if 0
    delete [] mEdgeArray;
    mEdgeArray = NULL;
    
    SAFE_DELETE(mStarts);
    SAFE_DELETE(mEnds);
#endif

    delete [] mStartLines;
    mStartLines = NULL;
  }

  inline void reset()
  {
    mEdgeCount = 0;

#ifdef EDGEBUCKETS
    for (int i = mFirstStart; i <= mLastStart; i++)
    {
      mStartLines[i].init();
    }
    mPoolManager.reset();
#else
    for (int i = mFirstStart; i <= mLastStart; i++)
    {
      mStartLines[i].scanlineEdgeCount = 0;
    }
#endif

    mFirstStart = mMaxScanlines;
    mLastStart = -1;
  }

  inline int32_t getEdgeCount()
  {
    return mEdgeCount;
  }

#ifndef EDGEBUCKETS
  inline void resetStart()
  {
    mCurrentStartLine = mFirstStart;
    mCurrentStartEdge = 0;
  }

  inline edge* getCurrentStart()
  {
    if (mCurrentStartLine <= mLastStart)
      return &mStartLines[mCurrentStartLine].scanlineEdges[mCurrentStartEdge];
    else return NULL;
  }

  void advanceStartLine()
  {
    mCurrentStartEdge = 0;
    for (int32_t i = mCurrentStartLine+1; i <= mLastStart; i++)
    {
      if (mStartLines[i].scanlineEdgeCount > 0)
      {
        mCurrentStartLine = i;
        return;
      }
    }
    mCurrentStartLine = 1601;
  }

  inline void advanceStart()
  {
    if (mCurrentStartEdge < mStartLines[mCurrentStartLine].scanlineEdgeCount-1)
      mCurrentStartEdge++;
    else
      advanceStartLine();
  }
#endif

  inline edge* addEdge(int32_t scanline, int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool left)
  {
#ifdef EDGEBUCKETS
    edge *e = mStartLines[scanline].getNewEdge();
#else
    edge *e = &mStartLines[scanline].scanlineEdges[mStartLines[scanline].scanlineEdgeCount];
    mStartLines[scanline].scanlineEdgeCount++;
#endif

#if 1
    e->mX1 = x1;
    e->mY1 = y1;
    e->mX2 = x2;
#endif
    e->mY2 = y2;

    e->mLeft = left?1:-1;

    if (scanline < mFirstStart)
      mFirstStart = scanline;
    if (scanline > mLastStart)
      mLastStart = scanline;

    mEdgeCount++;

    return e;
  }

  inline void sortEdges()
  {
#ifndef EDGEBUCKETS
    resetStart();
#endif
  }

public:
  //edge* mEdgeArray;
  int mEdgeCount;
  int32_t mCurrentStartLine;
  int32_t mCurrentStartEdge;
#ifdef EDGEBUCKETS
  edgeLine* mStartLines;
#else
  scanlineDesc* mStartLines;
#endif

  int32_t mFirstStart, mLastStart;
  uint32_t mMaxScanlines;
};


#endif
#endif


#if 1  // still used for textures... replace at some point
struct endPointArray
{
  endPointArray()
  {
    memset(mEndPoints, 0, sizeof(endPoint) * MINEDGES);
    mCount = 0;
  }

  // Can do a binary search here
#if 0
  int32_t find(int y)
  {
    for (int i = 0; i < mCount; i++)
    {
      if (mEndPoints[i].mY > y) return i;
    }
    return -1;
  }
#else
  int32_t find(int y)
  {
#if 0
    int a = 0;
    int z = mCount-1;
    int m = (a+z)/2;
    while (a < z)
    {
      if (mEndPoints[m].mY < y)
      {
        a = m;
      }
      else if (mEndPoints[m].mY > y)
      {
        z = m-1;
      }
      else return m;
      m = (a+z)/2;
    }
    //if (mEndPoints[m].mY < y) return -1;
    //else return m;
    return m;
#else
   // int lastS = -1;
    int result = -1;

    if (mCount > 0)
    {
      int s = 0;
      int e = mCount-1;
      int m = s + ((e-s+1)>>1);

      while (s < m) // Given this condition alone we know that m-1 exists
      {
        if (y <  mEndPoints[m].mY) e = m-1;
        else if (y >= mEndPoints[m].mY) s = m;
        m = s + ((e-s+1)>>1);
      }

      if (y > mEndPoints[s].mY)
        result = s+1;
      else result = s;
  //    lastS = s;
    }

#if 0

    char b[8000];
    b[0] = 0;
    for (int i = 0; i < mCount; i++)
    {
      char buffer[4096];

      if (i == result)
      {
        sprintf(buffer, "(%d), ", y);
        strcat(b, buffer);
      }

      sprintf(buffer, "%d, ", mEndPoints[i].mY);
      strcat(b, buffer);

    }

    if (result == -1)
    {
      char buffer[4096];
      sprintf(buffer, "(%d), ", y);
      strcat(b, buffer);
    }

//        rtLog("edges => %s :: s(%d)\n", b, lastS);
#endif

    return result;
#endif
  }
#endif

  void add(int y, textureedge *e)
  {
    int i = find(y);
#if 1
    if (i >= 0) // make a hole
    {
      for (int m = mCount; m > i; m--)
      {
        mEndPoints[m] = mEndPoints[m-1];
      }
    }
    else i = mCount;
#else
    i = mCount;
#endif

    mEndPoints[i].mY = y;
    mEndPoints[i].mEdge = e;

    mCount++;
  }

  inline void removeAll()
  {
    mCount = 0;
  }

  inline const endPoint* get(int32_t index) const
  {
    if (index < mCount)
      return &mEndPoints[index];
    else return NULL;
  }

  inline int32_t getCount() const
  {
    return mCount;
  }

  int32_t mCount;
  endPoint mEndPoints[MINEDGES];
};
#endif

#define EDGE_ARRAY_SIZE 100000
struct edgeArray
{
  edgeArray(): mCount(0)
  {
    memset(mEdges, 0, sizeof(edge*) * EDGE_ARRAY_SIZE);
  }

  inline void Add(edge * e)
  {
    mEdges[mCount++] = e;
  }

  inline int GetSize()
  {
    return mCount;
  }

  inline void RemoveAll()
  {
    mCount = 0;
  }

  inline edge* get(int index)
  {
    return mEdges[index];
  }


  edge* mEdges[EDGE_ARRAY_SIZE];
  int mCount;

};

typedef edgeArray edges;


textureedge mTextureEdges[10];
int mTextureEdgeCount = 0;

endPointArray textureStartsX;
endPointArray textureEndsX;

endPointArray* textureStarts = &textureStartsX;
endPointArray* textureEnds = &textureEndsX;

//##

//    public: // BUGBUG
bool mTextureClamp;
bool mTextureClampColor;
bool mBiLerp;
bool mAlphaTexture;

bool mOverdraw;

//##

pxRasterizer::pxRasterizer():
  mYOversample(0), mXResolution(0),
  mFirst(0), mLast(0), mLeftExtent(0), mRightExtent(0),
  mBuffer(NULL),
#ifndef EDGECLEANUP
  mEdgeArray(NULL), miStarts(NULL), miEnds(NULL), mEdgeCount(0),
#else
  mEdgeManager(NULL),
#endif
   mCoverage(NULL), mFillMode(fillWinding), mColor(pxRed), mAlpha(1.0),
   mAlphaDirty(false), mEffectiveAlpha(1.0), mClipValid(false),
   mClipInternalCalculated(false), mCachedBufferHeight(0), mCachedBufferWidth(0),
   overSampleAdd(0), overSampleAddMinusOne(0), overSampleAdd4MinusOne(0),
   overSampleAdd4(0), overSampleFlush(0), overSampleMask(0),
   mTexture(NULL),
   mTextureClamp(false), mTextureClampColor(false), mBiLerp(false),
   mAlphaTexture(false), mOverdraw(false),
   mTextureOriginX(0), mTextureOriginY(0)
{
  mMatrix22.identity();
  mTextureMatrix22.identity();

  mMatrix.identity();
  mTextureMatrix.identity();
}

pxRasterizer::~pxRasterizer()
{
  term();
}

void pxRasterizer::init(pxBuffer* buffer)
{
  term();

  xShift = 4;
  mBuffer = buffer;

#ifdef FRONT2BACK
  // use fixed pt x coordinates
  mSpanBuffer.init(mBuffer->bounds());

  // Some test clipping
#if 0
  for(int i = 0; i < 200; i++)
  {
    mSpanBuffer.setCurrentRow(i);
    for(int j = 0; j < 10; j++)
    {
      mSpanBuffer.addSpan((i+(j*100))<<4, (i+(j*100)+50)<<4);
    }
  }
#endif
#endif

#ifndef EDGECLEANUP
  mEdgeArray = (void*) new edge [MINEDGES];  // blech more than 1000 edges?
  miStarts   = (void*) new edge*[MINEDGES];
  miEnds     = (void*) new edge*[MINEDGES];
#else
  if (!mEdgeManager)
    mEdgeManager = new edgeManager;
#endif

  mCachedBufferHeight = -1;
  mCachedBufferWidth = -1;

  setClip(NULL);
  mColor.u = 0xff000000;  // black
  setAlpha(1.0);
  //mYOversample = 2;

  setYOversample(4);
  mXResolution = 16;

  mFillMode = fillWinding;
  //  mFillMode = fillEvenOdd;

  //mFirst = mBuffer->height() * mYOversample;
  //mLast = 0;

  reset();
}

void pxRasterizer::term()
{
#ifndef EDGECLEANUP
  edge** iStarts = (edge**)miStarts;
  edge** iEnds = (edge**)miEnds;
  edge* edgeArray = (edge*)mEdgeArray;

  SAFE_DELETE(miStarts);
  SAFE_DELETE(iEnds);
  SAFE_DELETE(edgeArray);

#else
#if 0
  // edgeManager destructor not freeing everything
  edgeManager* edgeMgr = (edgeManager*)mEdgeManager;
  
  SAFE_DELETE(edgeMgr);
  
#endif
#endif

  edgeManager* edgeMgr = (edgeManager*)mEdgeManager;
  SAFE_DELETE(edgeMgr);
  mEdgeManager = NULL;
  
  delete [] mCoverage;
  mCoverage = NULL;
}

void pxRasterizer::reset()
{
  mFirst = mBuffer->height() * mYOversample;
  mLast = 0;
  mLeftExtent = 0x7fffffff;
  mRightExtent = ~0x7fffffff + 1;

#ifndef EDGECLEANUP
  mEdgeCount = 0;
#else
  edgeManager *edgeMgr = (edgeManager*)mEdgeManager;
  edgeMgr->reset();
#endif

  resetTextureEdges();

  mClipInternalCalculated = false;
}

pxRect pxRasterizer::clip()
{
  if (mClipValid)
  {
    return mClip;
  }
  else
  {
    return mBuffer->bounds();
  }
}

void pxRasterizer::setClip(const pxRect* r)
{
  mClipValid = r?true:false;
  if (r) mClip = *r;
  mClipInternalCalculated = false;
}

pxFillMode pxRasterizer::fillMode() const
{
  return mFillMode;
}

void pxRasterizer::setFillMode(const pxFillMode& m)
{
  mFillMode = m;
}

pxColor pxRasterizer::color() const
{
  return mColor;
}

void pxRasterizer::setColor(const pxColor& c)
{
  mColor = c;
  mAlphaDirty = true;
}

double pxRasterizer::alpha() const
{
  return mAlpha;
}

void pxRasterizer::setAlpha(double alpha)
{
  mAlpha = alpha;
  mAlphaDirty = true;
}

void pxRasterizer::calculateEffectiveAlpha()
{
  mEffectiveAlpha = (unsigned char)xs_RoundToInt(mColor.a * mAlpha);
  if (mEffectiveAlpha < 255)
  {
    for (int i = 0; i < 256; i++)
    {
      mCoverage2Alpha[i] = (i * mEffectiveAlpha) / 255;
    }
  }

}

void pxRasterizer::addTextureEdge(double x1, double y1, double x2, double y2,
                                  double u1, double v1, double u2, double v2)
{
  textureedge textureEdge;

  textureEdge.mX1 = xs_CRoundToInt(x1 * UVFIXED);
  textureEdge.mY1 = xs_CRoundToInt(y1 * UVFIXED);
  textureEdge.mX2 = xs_CRoundToInt(x2 * UVFIXED);
  textureEdge.mY2 = xs_CRoundToInt(y2 * UVFIXED);

#if 0
  textureEdge.mU1 = xs_CRoundToInt(u1) -1;
  textureEdge.mV1 = xs_CRoundToInt(v1) -1 ;
  textureEdge.mU2 = xs_CRoundToInt(u2) -1;
  textureEdge.mV2 = xs_CRoundToInt(v2) -1;
  textureEdge.mU1 = textureEdge.mU1 << UVFIXEDSHIFT;
  textureEdge.mV1 = textureEdge.mV1 << UVFIXEDSHIFT;
  textureEdge.mU2 = textureEdge.mU2 << UVFIXEDSHIFT;
  textureEdge.mV2 = textureEdge.mV2 << UVFIXEDSHIFT;
#else
  textureEdge.mU1 = xs_CRoundToInt(u1 * UVFIXED);
  textureEdge.mV1 = xs_CRoundToInt(v1 * UVFIXED);
  textureEdge.mU2 = xs_CRoundToInt(u2 * UVFIXED);
  textureEdge.mV2 = xs_CRoundToInt(v2 * UVFIXED);
#endif

  // trivial reject of horixontal edges
  //if (textureEdge.mY1 == textureEdge.mY2) return;

  // flip endpoints
  if (textureEdge.mY1 > textureEdge.mY2)
  {
    int32_t t;

    t = textureEdge.mY1;
    textureEdge.mY1 = textureEdge.mY2;
    textureEdge.mY2 = t;

    t = textureEdge.mX1;
    textureEdge.mX1 = textureEdge.mX2;
    textureEdge.mX2 = t;

    t = textureEdge.mU1;
    textureEdge.mU1 = textureEdge.mU2;
    textureEdge.mU2 = t;

    t = textureEdge.mV1;
    textureEdge.mV1 = textureEdge.mV2;
    textureEdge.mV2 = t;
  }

  textureEdge.mdy = (textureEdge.mY2 - textureEdge.mY1) >> UVFIXEDSHIFT;
  if (textureEdge.mdy == 0) return;

  textureEdge.mdx = (textureEdge.mX2 - textureEdge.mX1) / textureEdge.mdy;
  textureEdge.mdu = (textureEdge.mU2 - textureEdge.mU1) / textureEdge.mdy;
  textureEdge.mdv = (textureEdge.mV2 - textureEdge.mV1) / textureEdge.mdy;

  textureEdge.mCurrentX = textureEdge.mX1;

  textureEdge.mCurrentU = textureEdge.mU1;
  textureEdge.mCurrentV = textureEdge.mV1;

  mTextureEdges[mTextureEdgeCount] = textureEdge;
#if 1
  textureStarts->add(textureEdge.mY1 >> UVFIXEDSHIFT, &mTextureEdges[mTextureEdgeCount]);
  textureEnds->add(textureEdge.mY2 >> UVFIXEDSHIFT, &mTextureEdges[mTextureEdgeCount]);
#else
  textureStarts->add(FIXEDSCANLINE(textureEdge.mY1), &mTextureEdges[mTextureEdgeCount]);
  textureEnds->add(FIXEDSCANLINE(textureEdge.mY2), &mTextureEdges[mTextureEdgeCount]);
#endif
  mTextureEdgeCount++;
}

void pxRasterizer::resetTextureEdges()
{
  mTextureEdgeCount = 0;
  textureStarts->removeAll();
  textureEnds->removeAll();
}

void pxRasterizer::addEdge(double x1, double y1, double x2, double y2)
{
  //return;
  int32_t dy;
  // Should I have a matched begin/end or open/close for the rasterizer for things like this?
#if 1
  if (!mClipInternalCalculated)
  {
    edgeManager* edgeMgr = (edgeManager*)mEdgeManager;
    edgeMgr->init(mBuffer->height() * mYOversample);
    mClipInternal = mBuffer->bounds();
    if (mClipValid)
      mClipInternal.intersect(mClip);
    mClipInternalCalculated = true;
//        mSpanBuffer.init(mBuffer->width()<<4, mBuffer->height());
  }
#endif

#if 0
  // Clip is empty so bail
  // Failure to do this was trashing the heap
  if (mClipInternal.bottom() < mClipInternal.top())
    return;
#endif

  edgeManager* edgeMgr = (edgeManager*)mEdgeManager;

  int ix1, ix2, iy1, iy2;

  // Convert to fixed point coordinates

#ifdef FIXEDPOINTEDGES
#if 0
	// with bias only for oversample of 2
  ix1 = xs_CRoundToInt(x1 * FIXEDFAC);
  iy1 = xs_CRoundToInt((y1+0.20) * FIXEDFAC);
  ix2 = xs_CRoundToInt(x2 * FIXEDFAC);
  iy2 = xs_CRoundToInt((y2+0.20) * FIXEDFAC);
#else
  ix1 = xs_CRoundToInt(x1 * FIXEDFAC);
  iy1 = xs_CRoundToInt((y1) * FIXEDFAC);
  ix2 = xs_CRoundToInt(x2 * FIXEDFAC);
  iy2 = xs_CRoundToInt((y2) * FIXEDFAC);
#endif
#else
  ix1 = xs_CRoundToInt(x1 * mXResolution);
  iy1 = xs_CRoundToInt(y1 * mYOversample);
  ix2 = xs_CRoundToInt(x2 * mXResolution);
  iy2 = xs_CRoundToInt(y2 * mYOversample);
#endif

  bool left = (iy1 < iy2);

#if 1
  //#ifndef FIXEDPOINTEDGES
  if (iy1 == iy2) return; // don't add horizontal lines
  //#endif

#ifdef FIXEDPOINTEDGES
  int iClipTop = mClipInternal.top() * FIXEDFAC;
  int iClipBot = (mClipInternal.bottom() * FIXEDFAC);
#else
  int iClipTop = mClipInternal.top() * mYOversample;
  int iClipBot = (mClipInternal.bottom() * mYOversample);
#endif

  // trivial reject if we have an empty clipping rectangle;
  // If we don't do this we can overshoot the mStartLines
  // array
  if (iClipTop >= iClipBot)
    return;

  // trivially reject edges that are above or below or current clip bounds
  if (iy1 < iClipTop && iy2 < iClipTop)
    return;
  if (iy1 >= iClipBot && iy2 >= iClipBot)
    return;


#if 1
  // Flip the endpoints if necessary to ensure that the edge is laid
  // out in scanline order
  if (iy1 > iy2)
  {
    int t;
    t = ix1;
    ix1 = ix2;
    ix2 = t;
    t = iy1;
    iy1 = iy2;
    iy2 = t;
  }

#endif



#if 1
  if (iy1 < iClipTop)
  {

#ifdef FIXEDPOINTEDGES
    int32_t scanlineY1 = iy1>>16;//FIXEDSCANLINE(iy1);
    int32_t scanlineY2 = iy2>>16;//FIXEDSCANLINE(iy2);
    int32_t clipY = iClipTop>>16;
    //int xIntersection = ix1 + ((ix2-ix1) * (clipY - scanlineY1)) / (scanlineY2-scanlineY1);
    int xIntersection = ix1 + (((ix2-ix1) / (scanlineY2-scanlineY1)) * (clipY - scanlineY1));
#else
    int xIntersection = ix1 + (((ix2-ix1)/(iy2-iy1)) * (iClipTop - iy1));
#endif
    iy1 = iClipTop;
    ix1 = xIntersection;
  }

#if 0
  if (iy2 >= iClipBot)
  {
#if 0 //FIXEDPOINTEDGES
    int32_t scanlineY1 = FIXEDSCANLINE(iy1);
    int32_t scanlineY2 = FIXEDSCANLINE(iy2);
#else
    int32_t scanlineY1 = iy1>>16;
    int32_t scanlineY2 = iy2>>16;
#endif
    int xIntersection = (ix2-ix1) * (iClipBot - iy1) / (scanlineY2-scanlineY1) + ix1;
    iy2 = iClipBot;
    ix2 = xIntersection;
  }
#else
  //iy2 = iClipBot;

  if (iy2 >= iClipBot)
  {

#ifdef FIXEDPOINTEDGES
    int32_t scanlineY1 = iy1>>16;//FIXEDSCANLINE(iy1);
    int32_t scanlineY2 = iy2>>16;//FIXEDSCANLINE(iy2);
    int32_t clipY = iClipBot>>16;
    //int xIntersection = ix1 + ((ix2-ix1) * (clipY - scanlineY1)) / (scanlineY2-scanlineY1);
    int xIntersection = ix1 + (((ix2-ix1) / (scanlineY2-scanlineY1)) * (clipY - scanlineY1));
#else
    int xIntersection = ix1 + (((ix2-ix1)/(iy2-iy1)) * (iClipTop - iy1));
#endif
    iy2 = iClipBot;
    ix2 = xIntersection;
  }

#endif

#ifdef FIXEDPOINTEDGES
  int32_t scanlineY1 = FIXEDSCANLINE(iy1);
  int32_t scanlineY2 = FIXEDSCANLINE(iy2);

  dy = scanlineY2 - scanlineY1;

  if (!dy)
    return;
#else
  int32_t scanlineY1 = iy1;
#endif


#if 0
  edge* e = edgeMgr->addEdge(scanlineY1, ix1, iy1, ix2, iy2, left);
#else
#ifdef EDGEBUCKETS
#if 0
  if (scanlineY1 >= edgeMgr->mMaxScanlines)
    rtLog("Scanline(%d) Too Large\n", scanlineY1);
#endif
  edge *e = edgeMgr->mStartLines[scanlineY1].getNewEdge();
#else
  edge *e = &mStartLines[scanlineY1].scanlineEdges[mStartLines[scanlineY1].scanlineEdgeCount];
  mStartLines[scanlineY1].scanlineEdgeCount++;
#endif

  //return;

#if 1
  e->mX1 = ix1;
  e->mY1 = iy1;
  e->mX2 = ix2;
#endif
  e->mY2 = iy2;

  e->mLeft = left?1:-1;

#if 1
#if 1
  if (scanlineY1 < edgeMgr->mFirstStart)
    edgeMgr->mFirstStart = scanlineY1;
  //  return;
#endif
  /// This is to avoid some bizarre stall
  //return;
  if (scanlineY1> edgeMgr->mLastStart)
    edgeMgr->mLastStart = scanlineY1;

  edgeMgr->mEdgeCount++;
#else
  edgeMgr->mFirstStart = 0;
  edgeMgr->mLastStart = 400;
#endif
#endif

#endif

#ifndef FIXEDPOINTEDGES
  int iFirst = e->mY1 & ~overSampleMask;
  int iLast = ((e->mY2  + mYOversample) & ~overSampleMask);


  if (mLeftExtent > e->mX1) mLeftExtent = e->mX1;
  if (mLeftExtent > e->mX2) mLeftExtent = e->mX2;

  if (mRightExtent < e->mX1) mRightExtent = e->mX1;
  if (mRightExtent < e->mX2) mRightExtent = e->mX2;

  if (iFirst < mFirst)
    mFirst = iFirst;
  if (iLast > mLast)
    mLast = iLast;

  e->mHeight = pxAbs(e->mY2 - e->mY1);
  int32_t width =  pxAbs(e->mX2 - e->mX1);

  e->mSide = (e->mX2 < e->mX1)?-1:1;

  if (e->mHeight >= width) // is steep?
  {
    e->mXDelta = 0;
    e->mErrorDelta = width;
    e->mXCurrent = e->mX1;
    e->mError = -e->mHeight / 2;
  }
  else
  {
    e->mXDelta = ((width/e->mHeight) * e->mSide);
    e->mErrorDelta = (width % e->mHeight);
    e->mXCurrent = e->mX1 - (e->mXDelta/2);
    e->mError = -e->mXDelta/2;
  }
#else


  if (mLeftExtent > ix1)
    mLeftExtent = ix1;
  if (mLeftExtent > ix2)
    mLeftExtent = ix2;

  if (mRightExtent < ix1)
    mRightExtent = ix1;
  if (mRightExtent < ix2)
    mRightExtent = ix2;

  if (scanlineY1 < mFirst)
    mFirst = scanlineY1;
  if (scanlineY2 > mLast)
    mLast = scanlineY2;

  e->mXCurrent = ix1;
  e->mXDelta = ((ix2-ix1))/dy;
#endif

#endif
}

void pxRasterizer::setTextureCoordinates(pxVertex& e1, pxVertex& e2, pxVertex& e3, pxVertex& e4,
                                         pxVertex& t1, pxVertex& t2, pxVertex& t3, pxVertex& t4)
{
  resetTextureEdges();
//  mTextureOriginX = xs_CRoundToInt(e1.x * UVFIXED);
//  mTextureOriginY = xs_CRoundToInt(e1.y * UVFIXED);
//
//  addTextureEdge(e1.x, e1.y, e2.x, e2.y, t1.x, t1.y, t2.x, t2.y);
//  addTextureEdge(e2.x, e2.y, e3.x, e3.y, t2.x, t2.y, t3.x, t3.y);
//  addTextureEdge(e3.x, e3.y, e4.x, e4.y, t3.x, t3.y, t4.x, t4.y);
//  addTextureEdge(e4.x, e4.y, e1.x, e1.y, t4.x, t4.y, t1.x, t1.y);

  mTextureOriginX = xs_CRoundToInt(e1.x() * UVFIXED);
  mTextureOriginY = xs_CRoundToInt(e1.y() * UVFIXED);

  addTextureEdge(e1.x(), e1.y(), e2.x(), e2.y(), t1.x(), t1.y(), t2.x(), t2.y() );
  addTextureEdge(e2.x(), e2.y(), e3.x(), e3.y(), t2.x(), t2.y(), t3.x(), t3.y() );
  addTextureEdge(e3.x(), e3.y(), e4.x(), e4.y(), t3.x(), t3.y(), t4.x(), t4.y() );
  addTextureEdge(e4.x(), e4.y(), e1.x(), e1.y(), t4.x(), t4.y(), t1.x(), t1.y() );
}

int log2__[] = {0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4};

int compareEdge(const void* e1, const void* e2)
{
  return ((*(edge**)e1)->mXCurrent - (*(edge**)e2)->mXCurrent);
}

void pxRasterizer::rasterize()
{
  //    reset();
  //return;
  if (mAlphaDirty)
  {
    calculateEffectiveAlpha();
    mAlphaDirty = false;
  }

#if 1
  int clipLeft = mClipInternal.left() * mXResolution;
  int clipRight = ((mClipInternal.right()) * mXResolution);

  // trivial reject
  if (FIXEDX(mLeftExtent) <= clipRight && FIXEDX(mRightExtent) >= clipLeft)
  {
   // edgeManager* edgeMgr = (edgeManager*)mEdgeManager;

    edge* e1;
    edge* e2;
    if (mPoolManager.getTwoEdges(e1, e2))
    {
      // special case for "rectanglar" fill
#ifdef FIXEDPOINTEDGES
      int32_t e1x1 = FIXEDX(e1->mX1);
      int32_t e1x2 = FIXEDX(e1->mX2);
      int32_t e1y1 = FIXEDSCANLINE(e1->mY1);
      int32_t e1y2 = FIXEDSCANLINE(e2->mY2);

      int32_t e2x1 = FIXEDX(e2->mX1);
      int32_t e2x2 = FIXEDX(e2->mX2);
      int32_t e2y1 = FIXEDSCANLINE(e1->mY1);
      int32_t e2y2 = FIXEDSCANLINE(e2->mY2);

      if ((e1x1 == e1x2 && e1x2 == e2x2  &&
           e1y1 == e2y1 && e1y2 == e2y2) &&
          (!(e1x1 & 0xf) && !(e2x1 & 0xf) && !(e1y1 & overSampleMask) && !(e1y2 & overSampleMask)))
#else
        if ((e1->mX1 == e1->mX2 && e2->mX1 == e2->mX2 && e1->mY1 == e2->mY1 && e1->mY2 == e2->mY2) &&                (!(e1->mX1 & 0xf) && !(e2->mX1 & 0xf) && !(e1->mY1 & overSampleMask) && !(e1->mY2 & overSampleMask)))
#endif
        {
#ifdef FIXEDPOINTEDGES
          int top = e1y1 >> overSampleShift;
          int bot = e1y2 >> overSampleShift;
          int left = e1x1 >> xShift;
          int right = e2x1 >> xShift;
#else
          int top = e1->mY1 >> overSampleShift;
          int bot = e1->mY2 >> overSampleShift;
          int left = e1->mX1 >> xShift;
          int right = e2->mX1 >> xShift;
#endif


          // flip edges left to right if necessary
          int t;
          if (left > right)
          {
            t = left;
            left = right;
            right = t;
          }

#if 1
          // save off texture origin before clipping
          int texTop = mTextureOriginY>>UVFIXEDSHIFT;
          int texLeft = mTextureOriginX>>UVFIXEDSHIFT;

          top = pxClamp<int>(top, mClipInternal.top(), mClipInternal.bottom());
          left = pxClamp<int>(left, mClipInternal.left(), mClipInternal.right());
          right = pxClamp<int>(right, mClipInternal.left(), mClipInternal.right());
          bot = pxClamp<int>(bot, mClipInternal.top(), mClipInternal.bottom());
#endif

          if (!mTexture)
          {
            int ycount = bot-top;
            pxPixel* s = mBuffer->scanline(top) + left;

            int stride = mBuffer->width();
            if (mBuffer->upsideDown())
              stride = -stride;
            int w = (right-left);

            if (!mOverdraw)
            {
              if (mEffectiveAlpha == 255)
              {
                uint32_t *d, *ed;
                while (ycount--)
                {
                  const int c = mColor.u;
                  d = (uint32_t*)s;
                  ed = d + w;
                  while (d<ed)
                    *d++=c;
                  s += stride;
                }
              }
              else
              {
                uint32_t *d, *ed;
                while (ycount--)
                {
                  const int c = mColor.u;
                  d = (uint32_t*)s;
                  ed = d + w;
                  while (d<ed)
                  {
                    pxLerp2(mEffectiveAlpha, *d, c);
                    d++;
                  }
                  s += stride;
                }
              }
            }
            else
            {
              pxColor t = mColor;
              t.r = (int)t.r *t.a >> 8;
              t.g = (int)t.g *t.a >> 8;
              t.b = (int)t.b *t.a >> 8;

              uint32_t *r;
              uint32_t *d, *ed;
              for (int32_t y = top; y < bot; y++)
              {
                const int c = t.u;

                //mSpanBuffer.setCurrentRow(top);
                mSpanBuffer.setCurrentRow(y);
                mSpanBuffer.startClipRow(y);
                mSpanBuffer.startClipSpan(left<<4, right<<4);
                int32_t x0, x1;
                while(mSpanBuffer.getClip(x0, x1))
                {
                  x0 >>= 4;
                  x1 >>= 4;
                  r = (uint32_t*)mBuffer->scanline(y);
                  d =  r + x0;
                  ed = r + x1;
                  int opaqueCount = 0;
                  int opaqueStart = x0;
                  while (d<ed)
                  {
                    if (pxPreMultipliedBlendBehind(*d, c))
                      opaqueCount++;
                    else
                    {
                      if (opaqueCount)
                        mSpanBuffer.addSpan(opaqueStart<<4, (opaqueStart+opaqueCount)<<4);
                      // Add an opaqueSpan
                      opaqueStart = d-r+1;
                      opaqueCount = 0;
                    }
                    if (opaqueCount)
                      mSpanBuffer.addSpan(opaqueStart<<4, (opaqueStart+opaqueCount)<<4);
                    d++;
                  }
                }
//                            mSpanBuffer.addSpan(left<<4, right<<4);
              }
            }

            reset();
            return;
          }
          else if (mMatrix.isTranslatedOnly() && mTextureMatrix.isTranslatedOnly())  // filtered // BUGBUG not taking non texture affine into account
          {
            texLeft -= xs_RoundToInt(mTextureMatrix.translateX());
            texTop  -= xs_RoundToInt(mTextureMatrix.translateY());

            pxPixel* s = mBuffer->scanline(top);

            int stride = mBuffer->width();
            if (mBuffer->upsideDown())
              stride = -stride;
            int w = (right-left);

            //int maxU = mTexture->width()-1;
            //int maxV = mTexture->height()-1;

            //if (mEffectiveAlpha == 255)
            {
              pxPixel *d, *ed;
              int ty = (top-texTop)%mTexture->height();
              for (int y = top; y < bot;)
              {
                for (; y < bot && ty < mTexture->height(); ty++)
                {
                  pxPixel* t0 = mTexture->scanline(ty);
                  int textureOffset = (left-texLeft)%mTexture->width();
                  pxPixel* t = t0 + textureOffset;
                  pxPixel* te = t + (mTexture->width()-textureOffset);

                  d = (pxPixel*)s + left;
                  ed = d + w;
                  while (d<ed)
                  {
                    if (!mOverdraw)
                    {
                      if (!mAlphaTexture)
                      {
                        if (mEffectiveAlpha == 255)
                        {
                          while(d < ed && t < te)
                            *d++=*t++;
                        }
                        else
                        {
                          while(d < ed && t < te)
                          {
                            pxLerp2(mCoverage2Alpha[t->a], d->u, t->u);
                            t++;
                          }
                        }
                      }
                      else
                      {
                        if (mEffectiveAlpha == 255)
                        {
                          while(d < ed && t < te)
                          {
                            if (t->a == 255)
                              *d = *t;
                            else
                              pxLerp2(t->a, d->u, t->u);
                            d++;
                            t++;
                          }
                        }
                        else
                        {
                          while(d < ed && t < te)
                          {
                            if (t->a == 255)
                              *d = *t;
                            else
                              pxLerp2(mCoverage2Alpha[t->a], d->u, t->u);
                            d++;
                            t++;
                          }
                        }
                      }
                    }
                    else // overdraw
                    {
                      int dw = ed - d;
                      pxPixel* sd = d;
                      int tw = te - t;
                      pxPixel* st = t;
                      int ww = pxMin<int>(dw, tw);
#if 0
                      ed = d+ww;
                      while(d < ed)
                      {
                        t->a = 255;
                        pxPreMultipliedBlendBehind(d->u, t->u);
                        // pxBlend(d->u, t->u);
                        d++;
                        t++;
                      }
#else
#if 1
                      int32_t x0, x1;
                      mSpanBuffer.startClipRow(y);
                      int curSpanLeft = (d-(pxPixel*)s);
                      int curSpanRight = curSpanLeft + ww;
                      mSpanBuffer.startClipSpan(curSpanLeft<<4, curSpanRight<<4);
                      while(mSpanBuffer.getClip(x0, x1))
                      {
                        mSpanBuffer.setCurrentRow(y);
                        mSpanBuffer.addSpan(x0, x1);
                        int tX0 = (x0>>4)-curSpanLeft;
                        int tX1 = (x1>>4)-curSpanLeft;

                        t = st + tX0;
                        d = sd + tX0;
                        pxPixel* ted = sd + tX1;
#if 1
                        while(d < ted)
                        {
                          //   t->a = 255;
                          //*d = *t;
                          pxPreMultipliedBlendBehind(d->u, t->u);
                          d++;
                          t++;
                        }
#endif
                      }
#endif
#endif
                      d = sd + ww;
                      t = st + ww;
                    }
                    if (mTextureClamp)
                    {
                      pxPixel *lastTextureSample = mTexture->pixel(mTexture->width()-1, ty);
                      // doesn't deal with alpha properly

                      if (!mOverdraw)
                      {
                        while(d<ed)
                          *d++ = *lastTextureSample;
                      }
                      else
                      {
                        lastTextureSample->a = 255;
                        while(d<ed)
                        {
                          pxPreMultipliedBlendBehind(d->u, lastTextureSample->u);
                          d++;
                        }
                      }

                    }
                    else
                    {
                      t = t0;
                      te = t + mTexture->width();
                    }
                  }
                  s += stride;
                  y++;
                }
                if (mTextureClamp)
                  ty = mTexture->height()-1;
                else
                  ty = 0;
              }
            }
#if 0
            else
            {
              uint32_t *d, *ed;
              while (ycount--)
              {
                register int c = mColor.u;
                d = (uint32_t*)s + left;
                ed = d + w;
                while (d<ed)
                {
                  pxLerp2(mEffectiveAlpha, *d, c);
                  d++;
                }
                s += stride;
              }
            }
#endif
            reset();
            return;
          }
        }
    }
  }
#endif
  // If we get here then hit the general case
  rasterizeComplex();
}

// Used to scan out the the coverage for non-filtered polys
void pxRasterizer::scanCoverage(pxPixel* scanline, int32_t x0, int32_t x1)
{
#if 0
  char *o = mCoverage+mCoverageFirst;
  char *o2 = mCoverage+mCoverageLast+1;
  uint32_t *p = (uint32_t*)scanline+mCoverageFirst;
  register uint32_t c = mColor.u;

  register int currentCoverage = 0;
  while(o < o2)
  {
    currentCoverage += *o;

    if (currentCoverage == 127)
      *p = c;
    else if (currentCoverage > 0)
      pxLerp2(currentCoverage<<1, *p, c);

    *o = 0;

    o++;
    p++;
  }
  *o2 = 0;
#else
  if (x0 <= x1)
  {
    char *o = mCoverage+x0;
    char *o2 = mCoverage+x1+1;
    uint32_t *p = (uint32_t*)scanline+x0;
    uint32_t *pe;
    const uint32_t c = mColor.u;

    int currentCoverage = 0;
    char* runStart;
    int coverageRun = 1;
    while(o < o2)
    {
      if (*o)
      {
        currentCoverage += *o;

        if (currentCoverage == 127)
          *p = c;
        else if (currentCoverage > 0)
          pxLerp2(currentCoverage<<1, *p, c);

        *o = 0;

        o++;
        p++;
      }
      else
      {
        runStart = o;
        o++;
        while(*o == 0)
          o++;

        coverageRun = o-runStart;

        pe = p + coverageRun;

        if (currentCoverage == 127)
        {
          while(p < pe)
            *p++ = c;
        }
        else if (currentCoverage > 0)
        {
          int cc = currentCoverage<<1;
          while(p < pe)
            pxLerp2(cc, *p++, c);
        }
        else p += coverageRun;

        //coverageRun = 1;
      }

    }
    *o2 = 0;
  }
#endif


}

inline pxPixel* pxRasterizer::getTextureSample(int32_t maxU, int32_t maxV, int32_t& curU, int32_t& curV)
{
  int32_t texU, texV;
  if (mTextureClamp)
  {
#if 1
    if (mTextureClampColor)
    {
      if (curU < 0 || curU >= maxU || curV < 0 || curV >= maxV)
        return &mColor;
      else
      {
        texU = curU>>UVFIXEDSHIFT;
        texV = curV>>UVFIXEDSHIFT;
      }
    }
    else
    {
      texU = pxClamp<int32_t>(curU, 0, maxU);
      texV = pxClamp<int32>(curV, 0, maxV);
      texU = texU>>UVFIXEDSHIFT;
      texV = texV>>UVFIXEDSHIFT;
    }
#else
    if (curU < 0 || curU >= maxU || curV < 0 || curV >= maxV)
      return &mColor;
    else
    {
      texU = curU>>UVFIXEDSHIFT;
      texV = curV>>UVFIXEDSHIFT;
    }
#endif
  }
  else
  {
    curU = pxWrap<int32_t>(curU, 0, maxU);
    curV = pxWrap<int32_t>(curV, 0, maxV);
    texU = curU>>UVFIXEDSHIFT;
    texV = curV>>UVFIXEDSHIFT;
  }

  return mTexture->pixel(texU, texV);
}

void pxRasterizer::rasterizeComplex()
{

  // bool textureUpsideDown = false;

  // int32_t maxU;
  // int32_t maxV;

  // if (mTexture)
  // {
  //   maxU = (mTexture->width()-1)<<UVFIXEDSHIFT;
  //   maxV = (mTexture->height()-1)<<UVFIXEDSHIFT;
  // }

  if (!mCoverage ||
      mBuffer->width() != mCachedBufferWidth)
  {
    //setClip(NULL);

    delete [] mCoverage;
    mCoverage = NULL;


#ifndef USELONGCOVERAGE
    mCoverage = new unsigned char[mBuffer->width()];
#else
    mCoverage = new char[mBuffer->width()+1];
    memset(mCoverage, 0, mBuffer->width()+1);
#endif

    mCachedBufferWidth = mBuffer->width();
  }

//  if (mTexture)
//  {
//    textureUpsideDown = mTexture->upsideDown();
//  }

  edge* mActiveList[1000];
  int mActiveCount = 0;


  int mActiveTextureCount = 0;
  textureedge* mActiveTextureList[10];

  int textureStartsCursor = 0;
  int textureEndsCursor = 0;

#ifdef FIXEDPOINTEDGES
  int clipLeft = mClipInternal.left() * mXResolution;
  int clipRight = ((mClipInternal.right()) * mXResolution)-1;
#else
  int clipLeft = mClipInternal.left() * mXResolution;
  int clipRight = ((mClipInternal.right()) * mXResolution)-1;
#endif

  // trivial reject
#ifdef FIXEDPOINTEDGES
  if (FIXEDX(mLeftExtent) <= clipRight && FIXEDX(mRightExtent) >= clipLeft)
#else
    if (mLeftExtent <= clipRight && mRightExtent >= clipLeft)
#endif
    {
#if 0
      edges* starts = (edges*)mStarts;
      edges* ends = (edges*)mEnds;
#else

      edgeManager* edgeMgr = (edgeManager*)mEdgeManager;


#endif
      int overSampleShift = log2__[mYOversample];
      int xShift = log2__[mXResolution];

      // start of general case
//        int mCoverageFirst = 0;
//        int mCoverageLast = mBuffer->width()-1;

      int mCoverageFirst = mBuffer->width()-1;
      int mCoverageLast = 0;


// BUGBUG Performance hit
      memset(mCoverage, 0, mBuffer->width()+1);


      edgeMgr->sortEdges();

#if 0
      int last = mLast;
      if (last >= mBuffer->height()*mYOversample)
        last = (mBuffer->height()-1)*mYOversample;
#else
      int last = mLast;
      if (last > (mClipInternal.bottom()*mYOversample)-1)
        last = (mClipInternal.bottom())*mYOversample-1;
#endif

      for (int l = mFirst; l <= last; l++)
      {
        //continue;
        bool spanBufferFull = false;

#if 1
#ifdef FRONT2BACK
        // Clip Edges against complex region
        if (mOverdraw)
        {
          mSpanBuffer.setCurrentRow(l>>overSampleShift);
          spanBufferFull = mSpanBuffer.isCurrentRowFull();
        }
#endif
#endif

        uint32_t subline = l & overSampleMask;

        if (!spanBufferFull)
        {

#if 1

          int mCoverageAmount;

          if (subline == 0)
          {
            if (mCoverageFirst <= mCoverageLast)
            {
              // BUGBUG Performance hit
              //memset(mCoverage, 0, mBuffer->width()+1);
              mCoverageFirst = mBuffer->width()-1;
              mCoverageLast = 0;
            }

            mCoverageAmount = (overSampleAdd>>1)-1;
          }
          else
          {
            mCoverageAmount = overSampleAdd >> 1;
          }

          //        continue;
#endif

#if 0
          edge** s = edgeMgr->mStartLines[l].scanlineEdges;
          edge** se = s + edgeMgr->mStartLines[l].scanlineEdgeCount;
          edge** t = mActiveList+mActiveCount;
          while (s < se)
            *t++ = *s++;
          mActiveCount = t-mActiveList;
#else
#ifdef EDGEBUCKETS
          edgeBucket* b = edgeMgr->mStartLines[l].headBucket;
          edge** t = mActiveList+mActiveCount;
          while(b)
          {
            edge* s = b->edges;
            edge* se = b->edgePos;
            while(s < se)
              *t++ = s++;
            b = b->nextBucket;
          }
          mActiveCount = t-mActiveList;
#else
          edge* s = edgeMgr->mStartLines[l].scanlineEdges;
          edge* se = s + edgeMgr->mStartLines[l].scanlineEdgeCount;
          edge** t = mActiveList+mActiveCount;
          while (s < se)
            *t++ = s++;
          mActiveCount = t-mActiveList;
#endif
#endif

          // continue;
#if 1
          // Advance Edges, Removing InActive Edges Along the Way
          {
            edge ** t = mActiveList;
            edge ** te = mActiveList + mActiveCount;

            edge** b = mActiveList;

            while (t < te)
            {
              edge* e = *t;

#ifdef FIXEDPOINTEDGES
              if (FIXEDSCANLINE(e->mY2) <= l)
#else
                if (e->mY2 <= l)
#endif
                {
                  mActiveCount--;
                }
                else
                {
                  e->mXCurrent += e->mXDelta;
#ifndef FIXEDPOINTEDGES
                  e->mError += e->mErrorDelta;

                  if (e->mError > 0)
                  {
                    e->mXCurrent += e->mSide;
                    e->mError -= e->mHeight;
                  }
#endif
                  *b++ = *t;
                }
              t++;
            }
          }
          //continue;
          // Sort along the X axis
          std::sort(mActiveList, mActiveList+mActiveCount, CompareX());
          //continue;
#endif

#if 1

          mSpanBuffer.startClipRow(l>>overSampleShift);

          // Fill scan line
          bool done = false;
          int z = 0;
          int winding = 0;
          bool inside = false;
          while (!done)
          {
            // find candidate edges
            bool foundEdges = false;

            int32_t p = 0, pEnd = 0;

            if (mFillMode == fillWinding)
            {
              for (; z < mActiveCount; z++)
              {
                winding += mActiveList[z]->mLeft;

                if (!inside && winding != 0)
                {
                  inside = true;
#ifdef FIXEDPOINTEDGES
                  p = FIXEDX(mActiveList[z]->mXCurrent);
#else
                  p = mActiveList[z]->mXCurrent;
#endif
                }
                else if (inside && winding == 0)
                {
                  inside = false;
#ifdef FIXEDPOINTEDGES
                  pEnd = FIXEDX(mActiveList[z]->mXCurrent);
#else
                  pEnd = mActiveList[z]->mXCurrent;
#endif
                  if (p != pEnd)
                  {
                    z += 1; // skip to next edge
                    foundEdges = true;
                    break;
                  }
                }
              }
            }
            else
            {
              if (z+1 < mActiveCount)
              {
#ifdef FIXEDPOINTEDGES
                p = FIXEDX(mActiveList[z]->mXCurrent);
                pEnd = FIXEDX(mActiveList[z+1]->mXCurrent);
#else
                p = mActiveList[z]->mXCurrent;
                pEnd = mActiveList[z+1]->mXCurrent;
#endif
                foundEdges = true;
                z += 2;
              }
              else foundEdges = false;
            }

            if (!foundEdges) done = true;
            else
            {
              //foundEdges = false;
              //continue;
#if 1
#ifdef FRONT2BACK
              // Do Complex Clipping
              int32_t pSave = p;
              int32_t pEndSave = pEnd;
              mSpanBuffer.startClipSpan(p, pEnd);
              while(mSpanBuffer.getClip(p, pEnd)) // This is destructive to p and pEnd
              {
#endif
#endif

                if (p == pEnd)
                  continue;  // these edges are a noop
#if 1
                if (pEnd < clipLeft)
                  continue;

                if (p > clipRight)
                  continue;

                if (p < clipLeft)
                  p = clipLeft;

                if (pEnd > clipRight)
                  pEnd = clipRight;

                // Do complex clipping
                // could result in muliple spans to be filled

                int cp = p >> xShift;

                int pEndSave = pEnd;
                pEnd = pEnd >> xShift;

                //continue;
#ifdef USELONGCOVERAGE
                if (cp == pEnd)
                {
#if 1
                  int ca = 0;
                  // if (pEndSave-p)
                  //   rtEdgeCover[pEndSave-p];
#if 1
                  if (subline == 0)
                  {
                    if (ca >= 1)
                      ca -= 1;
                  }
#endif
                  mCoverage[cp] += ca;
                  mCoverage[cp+1] -= ca;
#endif
                }
                else if (cp+1 == pEnd)
                {
#if 1
                  int pCoverage = ltEdgeCover[p&0xf];
                  int pEndCoverage = rtEdgeCover[pEndSave&0xf];
#if 1
                  if (subline == 0)
                  {
                    if (pCoverage >= 1)
                      pCoverage -= 1;
                    if (pEndCoverage >= 1)
                      pEndCoverage -= 1;
                  }
#endif
                  mCoverage[cp] += pCoverage;
                  mCoverage[pEnd] += (pEndCoverage-pCoverage);
                  mCoverage[pEnd+1] -= pEndCoverage;
#endif
                }
                else
                {
#if 1
                  int pCoverage = ltEdgeCover[p&0xf];
                  int pEndCoverage = rtEdgeCover[pEndSave&0xf];

#if 1
                  if (subline == 0)
                  {
                    if (pCoverage >= 1)
                      pCoverage -= 1;
                    if (pEndCoverage >= 1)
                      pEndCoverage -= 1;
                  }
#endif
                  mCoverage[cp] += pCoverage;
                  mCoverage[cp+1] += (mCoverageAmount - pCoverage);
                  mCoverage[pEnd] += (pEndCoverage-mCoverageAmount);
                  mCoverage[pEnd+1] -= pEndCoverage;
#endif
                }
                if (cp < mCoverageFirst)
                  mCoverageFirst = cp;
                if (pEnd > mCoverageLast)
                  mCoverageLast = pEnd;
#endif
#endif
#if 1
#ifdef FRONT2BACK
              } // end loop
#endif
#endif
            }
          }

#endif
        } // spanBufferFull

#if 1
        // scan out mCoverage
        if (subline == overSampleFlush)
        {
          pxPixel* s = mBuffer->scanline(l>>overSampleShift);
          {
            if (!mTexture)
            {
              scanCoverage(s, mCoverageFirst, mCoverageLast);

#if 0
              // BUGBUG
              for (int i = mCoverageFirst; i <= mCoverageLast+1; i++)
                mCoverage[i] = 0;
#else
//memset(mCoverage, 0, mBuffer->width()+1);

#endif

            }
            else//  TEXTURE MAPPING=====================================================
            {
#if 0
              bool overdrawDetected = false;
              // quick and dirty overdraw check
              if (mCoverageFirst <= mCoverageLast && mOverdraw)
              {
                int32_t discard1, discard2;
                mSpanBuffer.startClipRow(l>>overSampleShift);
                mSpanBuffer.startClipSpan(mCoverageFirst<<4, mCoverageLast<<4);
                overdrawDetected = !mSpanBuffer.getClip(discard1, discard2);
                if (overdrawDetected)
                {
                  char* s = mCoverage + mCoverageFirst;
#if 0
                  char* e = mCoverage + mCoverageLast+1;
                  while(s < e)
                    *s++ = 0;
#else
                  memset(s, 0, mCoverageLast-mCoverageFirst+2);
#endif

                }
              }

#endif
              if (mCoverageFirst <= mCoverageLast /*&& !overdrawDetected*/)
              {
                // pxPixel *t = mTexture->scanLinePixels32((l >> overSampleShift) % mTexture->height());
                int textureX = 0;
#if 1
                int textureY = l>>overSampleShift;
#else
                int textureY = l;
#endif

                // Add any edges starting on this line
                {
                  const endPoint* p = textureStarts->get(textureStartsCursor);
                  while(p && p->mY <= textureY)
                  {
                    //                        if (p->mY == textureY)
                    {
                      mActiveTextureList[mActiveTextureCount++] = p->mEdge;
                      int delta = (textureY - (p->mEdge->mY1 >> UVFIXEDSHIFT));
                      p->mEdge->mCurrentX += (p->mEdge->mdx * delta);
                      p->mEdge->mCurrentU += (p->mEdge->mdu * delta);
                      p->mEdge->mCurrentV += (p->mEdge->mdv * delta);
                    }
                    textureStartsCursor++;
                    p = textureStarts->get(textureStartsCursor);
                  }
                }

                // Remove any edges that end on this scanline
                {
                  const endPoint* p = textureEnds->get(textureEndsCursor);
                  while(p && p->mY < textureY)
                  {
                    //if (p->mY <= textureY)
                    {
                      int packCount = 0;
                      int newCount = mActiveTextureCount;
                      for (int k = 0; k < mActiveTextureCount; k++)
                      {
                        if (mActiveTextureList[k] != p->mEdge)
                          mActiveTextureList[packCount++] = mActiveTextureList[k];
                        else newCount--;
                      }
                      mActiveTextureCount = newCount;
                    }
                    textureEndsCursor++;
                    p = textureEnds->get(textureEndsCursor);
                  }
                }
#if 0
#ifdef FRONT2BACK

                if (mOverdraw)
                {
                  mSpanBuffer.setCurrentRow(l>>overSampleShift);
                  int32_t t1 = mCoverageFirst+1;
                  int32_t t2 = mCoverageLast-1;
                  if (t1 < t2)
                    mSpanBuffer.addSpan(t1<<4, t2<<4);
                }
#endif
#endif

                if (mActiveTextureCount < 2)
                {

#if 1
                  uint32_t color = 0xff0000ff;
                  //  rtLog("Less than two texture edges\n");
                  int currentCoverage = 0;
                  int c;
                  int i;
                  for (i = mCoverageFirst; i <= mCoverageLast; i++)
                  {
                    currentCoverage += mCoverage[i];
                    mCoverage[i] = 0;
                    c = currentCoverage<<1;
#if 1
                    if (c == 255)
                    {
                      s[i].u = color;
                    }
                    else
#endif
                    {
                      if (c > 0)
                        pxLerp2(c, s[i].u, color);
                      // todo handle color with alpha
                    }
                  }
                  mCoverage[i] = 0;
#else
                  memset(mCoverage, 0, mBuffer->width()+1);
#endif
                }
                else
                {
                  textureedge* tedge;
                  if (mActiveTextureList[0]->mCurrentX > mActiveTextureList[1]->mCurrentX)
                  {
                    tedge = mActiveTextureList[0];
                    mActiveTextureList[0] = mActiveTextureList[1];
                    mActiveTextureList[1] = tedge;
                  }

                  textureedge& leftTexture = *mActiveTextureList[0];
                  textureedge& rightTexture = *mActiveTextureList[1];

                  // we have two points in uv space need to draw a line in between
                  // setup

                  // calc distance between texture edges in xy space on same line so just subtract x values

                  int32_t widthXY = (rightTexture.mCurrentX - leftTexture.mCurrentX) >> UVFIXEDSHIFT;

                  if (widthXY == 0)  // workaround for now
                    widthXY = 1;

                  if (widthXY > 0)
                  {
                    int32_t du = (rightTexture.mCurrentU - leftTexture.mCurrentU) / widthXY;
                    int32_t dv = (rightTexture.mCurrentV - leftTexture.mCurrentV) / widthXY;

                    int32_t maxU = (mTexture->width() << UVFIXEDSHIFT)-1;
                    int32_t maxV = (mTexture->height() << UVFIXEDSHIFT)-1;

//                    int32_t baseX = 0;

                    int32_t startSpan;
                    int32_t endSpan;


                    bool done2 = false;

#if 1
                    if (mOverdraw)
                    {
                      mSpanBuffer.startClipRow(l>>overSampleShift);
                      mSpanBuffer.startClipSpan(mCoverageFirst<<4, mCoverageLast<<4);
                      if (mSpanBuffer.getClip(startSpan, endSpan))
                      {
                        startSpan = startSpan >> 4;
                        endSpan = endSpan >> 4;
                      }
                      else
                        done2 = true;
                    }
                    else
#endif
                    {
                      startSpan = mCoverageFirst;
                      endSpan = mCoverageLast;
                    }


                    while (!done2)
                    {
#if 0
                      if (mOverdraw)
                      {
                        mSpanBuffer.setCurrentRow(l>>overSampleShift);
#if 1
                        int32_t t1 = startSpan+1;
                        int32_t t2 = endSpan-1;
                        if (t1 <= t2)
                          mSpanBuffer.addSpan(t1<<4, t2<<4);
#else
                        mSpanBuffer.addSpan(startSpan<<4, endSpan<<4);
#endif
                      }
#endif

                      int32_t textureOffset = startSpan-(leftTexture.mCurrentX >> UVFIXEDSHIFT);

#if 1
                      if (textureOffset < 0)
                      {
                        textureOffset = 0;
                        // This shouldn't happen
                      }
#else
                      textureOffset = pxMax<int32_t>(0, textureOffset);
#endif

#if 0 // I think this is right.. but it was whacking the mag in screenjot ... try later
                      int32_t curU = leftTexture.mCurrentU + (textureOffset * du) + du/2 - (1<<15);
                      int32_t curV = leftTexture.mCurrentV + (textureOffset * dv) + dv/2 - (1<<15);
#else
                      int32_t curU = leftTexture.mCurrentU + (textureOffset * du) + du/2;
                      int32_t curV = leftTexture.mCurrentV + (textureOffset * dv) + /*dv/2*/ leftTexture.mdv/2;
#endif

                      if (!mTextureClamp)
                      {
                        curU = curU % (mTexture->width() << UVFIXEDSHIFT);
                        curV = curV % (mTexture->height() << UVFIXEDSHIFT);

                        if (curU < 0)
                          curU += (mTexture->width() << UVFIXEDSHIFT);
                        if (curV < 0)
                          curV += (mTexture->height() << UVFIXEDSHIFT);
                      }


#if 1
                      if (!mBiLerp)
                      {
#ifdef EXPERIMENTAL
                        // Use this one for textures
                        char *o = mCoverage+startSpan;
                        char *o2 = mCoverage+endSpan+1;
                        uint32_t *p = (uint32_t*)s+startSpan;
                        uint32_t *pe;
                        const uint32_t c = mColor.u;

                        int currentCoverage = 0;
                        char* runStart;
                        int coverageRun = 1;
                        while(o < o2)
                        {
                          if (*o)
                          {
                            currentCoverage += *o;
                            *o++ = 0;
                          }
                          else
                          {
                            runStart = o;
                            o++;

                            // eek this isn't guaranteed to terminate
                            // sentinel value?
                            signed char savedCoverage = *o2;
                            *o2 = 0xee;
                            while(*o == 0)
                              o++;
                            *o2 = savedCoverage;

                            coverageRun = o-runStart;
                          }

                          pe = p + coverageRun;

                          if (mOverdraw)
                          {
#if 1
                            if (!mAlphaTexture && currentCoverage == 127)
                            {
                              int32_t startOpaque = ((pxPixel*)p-s);
                              while(p < pe)
                              {
                                //pxLerp2(cc, *p++, getTextureSample(maxU, maxV, curU, curV)->u);
                                // doesn't do alpha textures right.
                                pxPixel ts = *getTextureSample(maxU, maxV, curU, curV);
                                // ts.a = 255;
                                pxPreMultipliedBlendBehind(*p++, ts.u);
                                textureX++;
                                curU += du;
                                curV += dv;
                              }
                              // add this span since it's opaque
                              if (coverageRun > 0)
                              {
                                int32_t endOpaque = startOpaque + coverageRun;
                                endOpaque = pxMin<int32_t>(endOpaque, mCoverageLast);
                                mSpanBuffer.setCurrentRow(l>>overSampleShift);
                                mSpanBuffer.addSpan(startOpaque<<4, endOpaque<<4);
                              }
                              //mSpanBuffer.addSpan(0, mBuffer->width()<<4);

                            }
                            else

#endif
                              if (currentCoverage > 0)
                              {
                                int cc = currentCoverage<<1;
                                while(p < pe)
                                {
                                  //pxLerp2(cc, *p++, getTextureSample(maxU, maxV, curU, curV)->u);
                                  // doesn't do alpha textures right.
                                  pxPixel ts = *getTextureSample(maxU, maxV, curU, curV);
                                  //ts.a = cc * ts.a / 255;
                                  ts.a = ((cc+1)*ts.a)>>8;
                                  pxBlendBehind(*p++, ts.u);
                                  //pxPreMultipliedBlendBehind(*p++, ts.u);
                                  textureX++;
                                  curU += du;
                                  curV += dv;
                                }
                              }
                              else
                              {
                                // instead of stepping across the texture have to decide whether it is better to
                                // recalc
#if 1
                                while(p < pe)
                                {
                                  getTextureSample(maxU, maxV, curU, curV);// have to call this to update curU and curV
                                  p++;
                                  textureX++;
                                  curU += du;
                                  curV += dv;
                                }
#else
                                p += (pe - p);
#endif
                              }
                          }
                          else
                          {
                            if (currentCoverage == 127)
                            {
                              while(p < pe)
                              {
                                *p++ = getTextureSample(maxU, maxV, curU, curV)->u;
                                textureX++;
                                curU += du;
                                curV += dv;
                              }
                            }
                            else if (currentCoverage > 0)
                            {
                              int cc = currentCoverage<<1;
                              while(p < pe)
                              {
                                pxLerp2(cc, *p++, getTextureSample(maxU, maxV, curU, curV)->u);
                                textureX++;
                                curU += du;
                                curV += dv;
                              }
                            }
                            else
                            {
                              // instead of stepping across the texture have to decide whether it is better to
                              // recalc
#if 1
                              while(p < pe)
                              {
                                getTextureSample(maxU, maxV, curU, curV);// have to call this to update curU and curV
                                p++;
                                textureX++;
                                curU += du;
                                curV += dv;
                              }
#else
                              textureX += coverageRun;
                              while(p < pe)

                                getTextureSample(maxU, maxV, curU, curV);
                              p++;
                            }
                            curU = curU + (coverageRun * du);
                            curV = curV + (coverageRun * dv);
#endif
                          }
                        }
                        coverageRun = 1;
                        //    o++;
                        //    p++;
                      }
                      // this was commented for overdraw...
                      // requires me to memset coverage tho
                      //              if (!mOverdraw)
                      //      *o2 = 0;
#else

                      int currentCoverage = 0;
                      int i;
                      for (i = startSpan; i <= endSpan; i++)
                      {
                        currentCoverage += mCoverage[i];
                        mCoverage[i] = 0;
                        int c;

                        if (mEffectiveAlpha == 255)
                          c = currentCoverage<<1;
                        else
                          c = mCoverage2Alpha[currentCoverage<<1];

#if 0

                        int32_t texU, texV;
                        if (mTextureClamp)
                        {
                          texU = pxClamp<int32_t>(curU, 0, maxU);
                          texV = pxClamp<int32_t>(curV, 0, maxV);
                          texU = texU>>UVFIXEDSHIFT;
                          texV = texV>>UVFIXEDSHIFT;
                        }
                        else
                        {
                          curU = pxWrap<int32_t>(curU, 0, maxU);
                          curV = pxWrap<int32_t>(curV, 0, maxV);
                          texU = curU>>UVFIXEDSHIFT;
                          texV = curV>>UVFIXEDSHIFT;
                        }

                        pxPixel* textureSample = mTexture->pixel(texU, texV);
#else
                        pxPixel* textureSample = getTextureSample(maxU, maxV, curU, curV);
#endif

                        if (!mAlphaTexture || textureSample->a == 255)
                        {
                          if (c == 255)
                          {
                            s[i].u = textureSample->u;
                          }
                          else
                          {
                            if (c != 0)
                              pxLerp2(c, s[i].u, textureSample->u);
                          }
                        }
                        else
                        {
                          // a dreaded divide
                          int a = (textureSample->a * c) / 255;

                          pxLerp2(a, s[i].u, textureSample->u);
                        }


                        textureX++;
                        curU += du;
                        curV += dv;

                      }
                      mCoverage[i] = 0;
#endif
                    }
                    else  // bilerp
                    {
                      unsigned int currentCoverage = 0;
                      int i;
                      for (i = startSpan; i <= endSpan; i++)
                      {
                        currentCoverage += mCoverage[i];
                        mCoverage[i] = 0;

#if 1
                        int32_t texV = ((curV>>UVFIXEDSHIFT)/*%mTexture->height()*/);
                        //                                if (texV < 0) texV += mTexture->height();
                        int32_t texU =  ((curU>>UVFIXEDSHIFT)/*%mTexture->width()*/);
#else
                        int32_t texV = (curV-dv/2)>>UVFIXEDSHIFT/*%mTexture->height()*/;
                        //                                if (texV < 0) texV += mTexture->height();
                        int32_t texU =  (curU-du/2)>>UVFIXEDSHIFT/*%mTexture->width()*/;
                        //                                if (texU < 0) texU += mTexture->width();
                        int32_t texV2 = (curV+du/2) >> UVFIXEDSHIFT;
                        int32_t texU2 = (curU+du/2) >> UVFIXEDSHIFT;


#endif

#if 0
                        if (texV2 >= mTexture->height())
                          texV2 = mTexture->height()-1;

                        if (texU2 >= mTexture->width())
                          texU2 = mTexture->width()-1;
#else
                        texU = pxWrap<int32_t>(texU,0,mTexture->width()-1);
                        texV = pxWrap<int32_t>(texV,0,mTexture->height()-1);
                        //          texU2 = pxWrap<int32_t>(texU2,0,mTexture->width()-1);
                        //          texV2 = pxWrap<int32_t>(texV2,0,mTexture->height()-1);
#endif
                        bool blend = true;
                        if (mTextureClamp)
                        {
                          bool clampedV = true;
                          bool clampedU = true;
                          // Clamp to texture extents
                          if (texV < 0) texV = 0;
                          else if (texV >= mTexture->height()) texV = mTexture->height()-1;
                          else clampedV = false;
                          if (texU < 0) texU = 0;
                          else if (texU >= mTexture->width()) texU = mTexture->width()-1;
                          else clampedU = false;

                          blend = (!clampedV && !clampedU);
                        }
                        else
                        {
                          // Simple Wrap
#if 1
                          texV %= mTexture->height();
                          texU %= mTexture->width();
                          if (texV < 0) texV += mTexture->height();
                          if (texU < 0) texU += mTexture->width();
#else

                          texU = pxWrap<int32_t>(texU, 0, mTexture->width()-1);
                          texV = pxWrap<int32_t>(texV, 0, mTexture->height()-1);
#endif
                        }

                        pxPixel textureSample;

                        pxPixel* texel = mTexture->pixel(texU, texV);
                        if (blend)
                        {
#if 1
#if 0  // I think this is right.. but it was whacking the mag in screenjot ... try later
                          int fracU = ((((curU) >> 8)) & 0xff);
                          int fracV = ((((curV) >> 8)) & 0xff);
#else
                          //int dd = 1 << 15;
                          //int dd = (1<<14)+(du>>2);

                          int fracU;// = ((((curU+(dd)) >> 8)) & 0xff);
                          int fracV;// = ((((curV+(dd)) >> 8)) & 0xff);
#endif
#else
                          int fracU = ((((curU >> 8)) & 0xff)>>2) + baseX;
                          int fracV = (((curV >> 8)) & 0xff);
                          baseX+=16;
                          if (baseX >= 128) baseX = 0;
#endif

                          pxPixel *texel2, *texel3, *texel4;
#if 0
                          int nextU = (curU + du) >> 16;
                          int nextV = (curV + dv) >> 16;
                          //int nextU = (texU < mTexture->width()-1);
                          if (nextU >= mTexture->width()-1)
                            nextU = texU;
                          if (nextV >= mTexture->height()-1)
                            nextV = texV;
                          //int nextV = (texV < mTexture->height()-1);
#else
                          //int nextU = (texU < mTexture->width()-1);
                          //int nextV = (texV < mTexture->height()-1);
#endif
//                          if (textureUpsideDown)
//                            nextV = - nextV;

                        //  texel2 = texel + nextU;
                        //  texel3 = texel + (mTexture->width() * nextV);
                        //  texel4 = texel3 + nextU;

#if 1
#if 0
                          textureSample = pxBlend4(
                            *texel,
                            *texel2,
                            *texel3,
                            *texel4,
                            fracU, fracV);
#else
                          {
                            int32_t texU2, texV2;

                            int up = (curU >> 8) & 0xff;
                            int vp = (curV >> 8) & 0xff;

                            if (up < 128)
                            {
                              texU2 = (curU >> 16);
                              texU = texU2-1;
                              fracU = 128+up;
                            }
                            else
                            {
                              texU = (curU >> 16);
                              texU2 = texU+1;
                              fracU = up-127;
                            }

                            if (vp < 128)
                            {
                              texV2 = (curV >> 16);
                              texV = texV2-1;
                              fracV = 128+vp;
                            }
                            else
                            {
                              texV = (curV >> 16);
                              texV2 = texV+1;
                              fracV = vp-127;
                            }

                            texU = pxClamp<int32_t>(texU, 0, mTexture->width()-1);
                            texU2 = pxClamp<int32_t>(texU2, 0, mTexture->width()-1);
                            texV = pxClamp<int32_t>(texV, 0, mTexture->height()-1);
                            texV2 = pxClamp<int32_t>(texV2, 0, mTexture->height()-1);

                            texel = mTexture->pixel(texU, texV);
                            texel2 = mTexture->pixel(texU2, texV);
                            texel3 = mTexture->pixel(texU, texV2);
                            texel4 = mTexture->pixel(texU2, texV2);

                            textureSample = pxBlend4(
                              *texel,
                              *texel2,
                              *texel3,
                              *texel4,
                              fracU, fracV);
                          }

#endif
#else
                          textureSample = pxBlend4(
                            *texel,
                            *mTexture->pixel(texU2, texV),
                            *mTexture->pixel(texU, texV2),
                            *mTexture->pixel(texU2, texV2),
                            fracU, fracV);
#endif
                        }
                        else
                        {
                          textureSample = *texel;
                        }


                        textureX++;
                        int c;

#if 0
                        if (mEffectiveAlpha == 255) c = mCoverage[i];
                        else c = mCoverage2Alpha[mCoverage[i]];
#else
                        if (mEffectiveAlpha == 255)
                          c = currentCoverage<<1;
                        else
                          c = mCoverage2Alpha[currentCoverage<<1];
#endif


                        if (!mAlphaTexture || texel->a == 255)
                        {
                          if (c == 255 )
                          {
                            s[i].u = textureSample.u;
                          }
                          else
                          {
                            if (c != 0)
                              pxLerp2(c, s[i].u, textureSample.u);
                            // todo handle color with alpha
                          }
                        }
                        else
                        {
                          int a = (texel->a * c) / 255;

                          pxLerp2(a, s[i].u, textureSample.u);

                        }

                        curU += du;
                        curV += dv;

                      }
                      mCoverage[i] = 0;
                    }

                    if (mOverdraw)
                    {
#if 1
                      done2 = !mSpanBuffer.getClip(startSpan, endSpan);
                      if (!done2)
                      {
                        startSpan >>= 4;
                        endSpan >>= 4;
                      }
#else
                      done2 = true;
#endif
                    }
                    else
                      done2 = true;
                  }// end done2
#else
                  int count = endSpan-startSpan+1;
                  if (count > 0)
                  {
                    unsigned char *o = (unsigned char*)&mCoverage[startSpan];
                    uint32_t *p = (uint32_t*)&s[startSpan];
                    //uint32_t c = mColor.u;
                    while(count--)
                    {
                      int32_t texV = ((curV>>UVFIXEDSHIFT)%mTexture->height());
                      int32_t texU =  ((curU>>UVFIXEDSHIFT)%mTexture->width());

                      if (mTextureClamp)
                      {
                        if (texV < 0) texV = 0;
                        else if (texV >= mTexture->height()) texV = mTexture->height()-1;
                        if (texU < 0) texU = 0;
                        else if (texU >= mTexture->width()) texU = mTexture->width()-1;
                      }
                      else
                      {
                        if (texV < 0) texV += mTexture->height();
                        if (texU < 0) texU += mTexture->width();
                      }
                      //                                uint32_t c = mTexture->pixel(texU, texV)->u;


                      uint32_t c = *(mTexture->scanLine32(texV)+texU);

                      if (*o == 255) *p = c;
                      else
                      {
                        if (*o != 0)
                          pxLerp2(*o, *p, c);
                      }
                      p++;
                      o++;
                      curU += du;
                      curV += dv;
                      textureX++;
                    }

                  }



#endif

                }
#if 1
                else
                {
                  uint32_t color = pxRed.u;
                  int currentCoverage = 0;
                  //   rtLog("texturewidth is 0\n");
                  int i;
                  for (i = mCoverageFirst; i <= mCoverageLast; i++)
                  {
                    currentCoverage += mCoverage[i];
                    const int c = currentCoverage << 1;
                    mCoverage[i] = 0;
#if 1
                    if (c == 255)
                    {
                      s[i].u = color;
                    }
                    else
#endif
                    {
                      if (c != 0)
                        pxLerp2(c, s[i].u, color);
                      // todo handle color with alpha
                    }
                  }

                  mCoverage[i] = 0;
                }
#endif
              }
              // hack hack... this may not be right.
              mCoverage[mCoverageLast+1] = 0;
#if 1
              if (mOverdraw)
              {
#if 0
#if 1
                mSpanBuffer.setCurrentRow(l>>overSampleShift);
                int32_t t1 = mCoverageFirst;
                int32_t t2 = mCoverageLast;
#if 1
                t1++;
                t2--;
#endif
                if (t1 <= t2)
                  mSpanBuffer.addSpan(t1<<4, t2<<4);
#else
                mSpanBuffer.setCurrentRow(l>>overSampleShift);
                mSpanBuffer.addSpan(0, mBuffer->width()<<4);
#endif
#endif
                //if (mCoverageFirst <= mCoverageLast)
                //        memset(mCoverage + mCoverageFirst, 0, mCoverageLast - mCoverageFirst+2);
                //                           memset(mCoverage, 0, mBuffer->width()+1);
              }
#endif
            }
            // Advance along texture edges
            for (int z = 0; z < mActiveTextureCount; z++)
            {
              mActiveTextureList[z]->mCurrentX += mActiveTextureList[z]->mdx;
              mActiveTextureList[z]->mCurrentU += mActiveTextureList[z]->mdu;
              mActiveTextureList[z]->mCurrentV += mActiveTextureList[z]->mdv;
            }
          }

        }
      }
#endif




    }
}
reset();
}


void pxRasterizer::setYOversample(int i)
{
  mYOversample = i;

  // This should move into the setter
  // should be able to trim this down...
  overSampleAdd = (256 / mYOversample);
  overSampleAddMinusOne = (overSampleAdd-1);
  overSampleAdd4MinusOne = ((overSampleAddMinusOne << 24) | (overSampleAddMinusOne << 16) | (overSampleAddMinusOne << 8) | overSampleAddMinusOne);
  overSampleAdd4 = ((overSampleAdd << 24) | (overSampleAdd << 16) | (overSampleAdd << 8) | overSampleAdd);
  overSampleFlush = (mYOversample-1);
  overSampleMask = (mYOversample-1);
  overSampleShift = log2__[mYOversample];
  fixedScanlineShift = 16-overSampleShift;

#ifndef USELONGCOVERAGE
  int xSlice = (overSampleAdd) >> 4;
#else
  int xSlice = (overSampleAdd >> 1) >> 4;
#endif
  for (int i = 0; i < 16; i++)
  {
    ltEdgeCover[i] = (16-i) *xSlice;
    rtEdgeCover[i] = i * xSlice;
  }

  reset();
}

void pxRasterizer::setXResolution(int i)
{
  mXResolution = i;
  xShift = log2__[mXResolution];
  reset();
}

void pxRasterizer::setTexture(pxBuffer* texture)
{
  mTexture = texture;
}

void pxRasterizer::setMatrix(const pxMatrix4T<float>& m)
{
  mMatrix = m;
}

void pxRasterizer::matrix(pxMatrix4T<float>& m) const
{
  m = mMatrix;
}

void pxRasterizer::setTextureMatrix(const pxMatrix4T<float>& m)
{
  mTextureMatrix = m;
}

void pxRasterizer::textureMatrix(pxMatrix4T<float>& m) const
{
  m = mTextureMatrix;
}

void pxRasterizer::clear()
{
  if (!mClipInternalCalculated)
  {
    edgeManager* edgeMgr = (edgeManager*)mEdgeManager;
    edgeMgr->init(mBuffer->height() * mYOversample);
    
    mClipInternal = mBuffer->bounds();
    
    if (mClipValid)
      mClipInternal.intersect(mClip);
    mClipInternalCalculated = true;
//        mSpanBuffer.init(mBuffer->width()<<4, mBuffer->height());
  }
  pxRect br = mClipInternal;
  pxColor c(0,0,0,0); // rgba
  
  mBuffer->fill(br, c);
  
  br.setLeft(br.left() << 4);
  br.setRight(br.right() << 4);
  
  mSpanBuffer.init(br);
}

bool pxRasterizer::alphaTexture() const { return mAlphaTexture; }
void pxRasterizer::setAlphaTexture(bool f)
{
  mAlphaTexture = f;
}

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic pop
#endif
