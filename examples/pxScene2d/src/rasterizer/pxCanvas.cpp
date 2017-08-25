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
  mw = scene->w();
  mh = scene->h();
}

pxCanvas::~pxCanvas()
{
  //term();
}

void pxCanvas::draw()
{
  mCanvasCtx.draw();
}

void pxCanvas::onInit()
{
  mInitialized = true;
  
  mCanvasCtx.init(mw, mh);
  
//  this->setX(0);
//  this->setY(0);
//  this->setW(mw);
//  this->setH(mh);
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
  
  uint8_t *op  = p->getStream();
  uint8_t *fin = p->getLength() + op;

  uint8_t opcode = 0;
 
  mCanvasCtx.newPath();
  
  while(op < fin)
  {
    opcode = *op++; // skip opcode
    
    switch(opcode)
    {
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_MOVE:
      {
//        uint8_t len = p->getByteAt(op);  op += sizeof(uint8_t);
        float   x   = p->getFloatAt(op); op += sizeof(float);
        float   y   = p->getFloatAt(op); op += sizeof(float);
        
        mCanvasCtx.moveTo(x, y);
        
//        printf("\n SVG_OP_MOVE( %f, %f ) ", x,y);
      }
      break;
      
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_LINE:
      {
//        uint8_t len = p->getFloatAt(op); op += sizeof(uint8_t);
        float   x   = p->getFloatAt(op); op += sizeof(float);
        float   y   = p->getFloatAt(op); op += sizeof(float);
        
        mCanvasCtx.moveTo(x, y);
        
//        printf("\n SVG_OP_LINE( %f, %f ) ", x,y);
      }
      break;
      
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_Q_CURVE:
      {
//        uint8_t len = p->getFloatAt(op); op += sizeof(uint8_t);
        float   x1  = p->getFloatAt(op); op += sizeof(float);
        float   y1  = p->getFloatAt(op); op += sizeof(float);
        float   x   = p->getFloatAt(op); op += sizeof(float);
        float   y   = p->getFloatAt(op); op += sizeof(float);
        
        mCanvasCtx.curveTo(x1, y1, x, y);
        
//        printf("\n SVG_OP_Q_CURVE( x1: %f, y1: %f,  x: %f, y: %f) ", x1, y1, x, y);
      }
      break;
      
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_C_CURVE:
      {
//        uint8_t len = p->getFloatAt(op); op += sizeof(uint8_t);
        float   x1  = p->getFloatAt(op); op += sizeof(float);
        float   y1  = p->getFloatAt(op); op += sizeof(float);
        float   x2  = p->getFloatAt(op); op += sizeof(float);
        float   y2  = p->getFloatAt(op); op += sizeof(float);
        float   x   = p->getFloatAt(op); op += sizeof(float);
        float   y   = p->getFloatAt(op); op += sizeof(float);
        
        mCanvasCtx.curveTo(x1, y1, x2, y2, x, y);
        
//        printf("\n SVG_OP_C_CURVE( x1: %f, y1: %f,  x2: %f, y2: %f,  x: %f, y: %f) ", x1, y1, x2, y2, x, y);
      }
      break;
   
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      case SVG_OP_CLOSE:
      {
//        uint8_t len = p->getFloatAt(op); op += sizeof(uint8_t);
        mCanvasCtx.closePath();
      }
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      default:
        rtLogError(" unrecoginzed OpCode: %0x02X", opcode);
      
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
    pxMatrix4f currentM;
    mCanvasCtx.matrix(currentM);

    mCanvasCtx.setMatrix(context.getMatrix());  // Apply CTM

    // - - - - - - - - - - - - - - - - - - -
    if(needsFill)   mCanvasCtx.fill();
    if(needsStroke) mCanvasCtx.stroke();
    // - - - - - - - - - - - - - - - - - - -
    
    mCanvasCtx.needsRedraw();
//    mRepaint = true;

    mCanvasCtx.setMatrix(currentM);  // Restore CTM
 }
  
  return RT_OK;
}

//====================================================================================================================================
//====================================================================================================================================

rtDefineObject(pxCanvas, pxObject);

//====================================================================================================================================
//====================================================================================================================================
