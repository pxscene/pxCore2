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

#include "pxPath.h"
#include "pxCanvas.h"
#include "pxContext.h"

#include <stdio.h>
#include "math.h"

extern pxContext context;

#ifdef USE_PERF_TIMERS
#include "pxTimer.h"
#endif

rtDefineMethod(pxCanvas, drawPath);

pxCanvas::pxCanvas(pxScene2d* scene): pxObject(scene)
{
  mw = 1280;// scene->w();
  mh =  720;// scene->h();
}

pxCanvas::~pxCanvas()
{
  //term();
}

void pxCanvas::draw()
{
  // NO OP

//  mCanvasCtx.draw();
//  mCanvasCtx.clear(); // HACK
}

void pxCanvas::onInit()
{
  mInitialized = true;
  
  mCanvasCtx.init(mw, mh);
//  initOffscreen(mw, mh);
  setUpsideDown(true);
}

void pxCanvas::sendPromise()
{ 
  mReady.send("resolve",this);
}

rtError pxCanvas::drawPath(rtObjectRef path)
{
  pxPath *p = (pxPath *) path.getPtr();

  if(!p)
  {
    rtLogError(" - malformed pxPath object op stream.");
    return RT_FAIL;
  }

  uint8_t opcode = 0;
  uint8_t *op    = p->getStream();
  uint8_t *fin   = p->getLength() + op;

  mCanvasCtx.newPath();

  float x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0, x2 = 0.0, y2 = 0.0;

  while(op < fin)
  {
    opcode = *op++; // skip opcode
    
    switch(opcode)
    {
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_MOVE:
      {
        x0 = p->getFloatAt(op); op += sizeof(float);
        y0 = p->getFloatAt(op); op += sizeof(float);

        mCanvasCtx.moveTo(x0, y0);

//        printf("\nCanvas: SVG_OP_MOVE( %.1f, %.1f ) ", x0,y0);
      }
      break;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_LINE:
      {
        x0 = p->getFloatAt(op); op += sizeof(float);
        y0 = p->getFloatAt(op); op += sizeof(float);

        mCanvasCtx.moveTo(x0, y0);

//        printf("\nCanvas: SVG_OP_LINE( x0: %.1f, y0: %.1f) ", x0, y0);
      }
      break;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_Q_CURVE:
      {
        x1 = p->getFloatAt(op); op += sizeof(float);
        y1 = p->getFloatAt(op); op += sizeof(float);
        x0 = p->getFloatAt(op); op += sizeof(float);
        y0 = p->getFloatAt(op); op += sizeof(float);

        mCanvasCtx.curveTo(x1, y1, x0, y0);

//        printf("\nCanvas: SVG_OP_Q_CURVE( x1: %.1f, y1: %.1f,  x0: %.1f, y0: %.1f) ", x1, y1, x0, y0);
      }
      break;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_C_CURVE:
      {
        x1 = p->getFloatAt(op); op += sizeof(float);
        y1 = p->getFloatAt(op); op += sizeof(float);
        x2 = p->getFloatAt(op); op += sizeof(float);
        y2 = p->getFloatAt(op); op += sizeof(float);
        x0 = p->getFloatAt(op); op += sizeof(float);
        y0 = p->getFloatAt(op); op += sizeof(float);
        
        mCanvasCtx.curveTo(x1, y1, x2, y2, x0, y0);
        
//        printf("\nCanvas: SVG_OP_C_CURVE( x1: %.1f, y1: %.1f,  x2: %.1f, y2: %.1f,  x0: %.1f, y0: %.1f) ", x1, y1, x2, y2, x0, y0);
      }
      break;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_CLOSE:
      {
        mCanvasCtx.closePath();
      }
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      default:
        rtLogError(" unrecoginzed OpCode: [%c] 0x%02X", (char)opcode, opcode);

    }//SWITCH
  }//WHILE

  bool needsFill   = false;
  bool needsStroke = false;

  mCanvasCtx.setAlpha(1.0);

  if(p->mFillColor.a > 0)
  {
    mCanvasCtx.setFillColor(p->mFillColor);
    needsFill = true;
  }

  if(p->mStrokeColor.a > 0 && p->mStrokeWidth > 0)
  {
    mCanvasCtx.setStrokeColor(p->mStrokeColor);
    mCanvasCtx.setStrokeWidth(p->mStrokeWidth);
    needsStroke = true;
  }

  // Drawing
  if(needsFill || needsStroke)
  {
    pxMatrix4f m;
    
    float ss = p->mStrokeWidth/2;
    
    if(ss > 0)
    {
      m.translate(ss, ss);
      mCanvasCtx.setMatrix(m);
    }
  
    // - - - - - - - - - - - - - - - - - - -
    if(needsFill)   mCanvasCtx.fill();
    if(needsStroke) mCanvasCtx.stroke();
    // - - - - - - - - - - - - - - - - - - -

    if(ss > 0)
    {
      m.translate(ss*2, -ss*2);
      p->applyMatrix(m);
    }
    
#if 0
#ifdef PX_PLATFORM_MAC

    extern void *makeNSImage(void *rgba_buffer, int w, int h, int depth);

    // HACK
    // HACK
    // HACK
    static int frame = 20;
    if(frame-- == 0)
    {
      void *img_raster = makeNSImage(offscreen().base(), offscreen().width(), offscreen().height(), 4);
      frame = -1;
    }
    // HACK
    // HACK
    // HACK
#endif
#endif
    
    p->setExtentLeft(   mCanvasCtx.extentLeft   );
    p->setExtentTop(    mCanvasCtx.extentTop    );
    p->setExtentRight(  mCanvasCtx.extentRight  );
    p->setExtentBottom( mCanvasCtx.extentBottom );
    
    p->setW(mCanvasCtx.extentRight  + p->mStrokeWidth);
    p->setH(mCanvasCtx.extentBottom + p->mStrokeWidth);
 }
  
  return RT_OK;
}

//====================================================================================================================================
//====================================================================================================================================

rtDefineObject(pxCanvas, pxObject);

//====================================================================================================================================
//====================================================================================================================================

