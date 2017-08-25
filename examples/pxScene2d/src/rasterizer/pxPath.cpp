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

#include "pxScene2d.h"

#include "pxCanvas.h"
#include "pxContext.h"

#include "pxPath.h"

pxPath::pxPath(pxScene2d* scene): pxObject(scene), mStrokeColor(pxClear), mFillColor(pxClear), mStrokeWidth(0)
{
/*
  pushFloat(123.456);
  
  uint8_t *s = getStream();
  float  ans = getFloatAt(s);
    
  printf("\n TEST = %f ", ans);
  
  
  opStream.clear();
  
  pushOpcode(SVG_OP_MOVE);
  pushFloat(100,200);
  
  s = getStream();

  uint8_t  op = getByteAt(s);  s+= sizeof(uint8_t);
  uint8_t len = getByteAt(s);  s+= sizeof(uint8_t);
  float     x = getFloatAt(s); s+= sizeof(float);
  float     y = getFloatAt(s); s+= sizeof(float);
 
  printf("\n TEST... op: %d  len: %d  >>>   moveTo(%f, %f) ",op, len, x,y);
*/
 // canvas
}


void pxPath::onInit()
{
  mInitialized = true;
  
  sendPromise();
}

pxPath::~pxPath()
{

}

void pxPath::sendPromise()
{
    mReady.send("resolve",this);
}

void pxPath::draw()
{
  printf("\n pxPath::draw() !!!!!! ");
  
  if(mRepaint)
  {
    rtObjectRef c = mScene->getCanvas();
    
    c.send("drawPath", this);
  }
}

//====================================================================================================================================


static pxColor getColor(const uint32_t c)
{
  uint8_t r = (c >> 24) & 0x000000ff;
  uint8_t g = (c >> 16) & 0x000000ff;
  uint8_t b = (c >>  8) & 0x000000ff;
  uint8_t a = (c      ) & 0x000000ff;
  
  return pxColor(r,g,b,a);
}

rtError pxPath::setStrokeColor(const uint32_t c)
{
  mStrokeColor = getColor(c);
  
  return RT_OK;
}

rtError pxPath::setFillColor(const uint32_t c)
{
  mFillColor = getColor(c);
  
  return RT_OK;
}

rtError pxPath::setStrokeWidth(const float w)
{
  mStrokeWidth = w;
  return RT_OK;
}


rtError pxPath::setPath(const rtString d)
{
    mPath = d;
    return parsePath(d, this);
}


static float pen_x = 0;
static float pen_y = 0;

void updatePen(float x, float y)
{
  pen_x = x;
  pen_y = y;
}

#define is_relative(x) (islower(x))

/*static*/ rtError pxPath::parsePath(const char *d, pxPath *p /*= NULL*/ )
{
  char *s = (char *)d;
  
  if(!d || ! p)
  {
    return RT_ERROR;
  }
  
  while (*s)
  {
    char op[2];
    float x, y, x1, y1, x2, y2;
    
    float last_x2, last_y2;
    
    int n;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // MOVE / LINE instruction
    //
    // SHORTHAND:  moveto (M, m), lineto (L, l) (2 arguments)
    //
    if (sscanf(s, " %1[MmLl] %f %f %n", op, &x, &y, &n) == 3)
    {
      do
      {
        x += p->x();
        y += p->y();
        
        if(is_relative(*op))
        {
          x += pen_x;  y += pen_y;
        }
        
        p->pushOpcode( *op );
        p->pushFloat(x,y);

        updatePen(x, y); // POSITION
        
        s += n;
      }
      while (sscanf(s, "%f %f %n", &x, &y, &n) == 2);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CLOSE instruction
    //
    // closepath (Z, z) (no arguments)
    //
    else
    if (sscanf(s, " %1[Zz] %n", op, &n) == 1)
    {
      p->pushOpcode( *op );
      
      s += n;
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // HORIZONTAL LINE instruction
    //
    // horizontal lineto (H, h) (1 argument)
    //
    else
    if (sscanf(s, " %1[Hh] %f %n", op, &x, &n) == 2)
    {
      do
      {
        if(is_relative(*op))
        {
          x += pen_x;
        }
        
        p->pushOpcode( 'L' );
        p->pushFloat(x,y);
        
        updatePen(x, y); // POSITION

        s += n; x = y = 0;
      }
      while (sscanf(s, "%f %n", &x, &n) == 1);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // VERTICAL LINE instruction
    //
    // vertical lineto (V, v) (1 argument)
    //
    else
    if (sscanf(s, " %1[Vv] %f %n", op, &y, &n) == 2)
    {
      do
      {
        if(is_relative(*op))
        {
          y += pen_y;
        }
        
        p->pushOpcode( 'L' );
        p->pushFloat(x,y);
        
        updatePen(x, y); // POSITION

        s += n; x = y = 0;
      }
      while (sscanf(s, "%f %n", &y, &n) == 1);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CUBIC CURVE instruction
    //
    //  curveTo (C, c) (6 arguments)
    //
    else
    if (sscanf(s, " %1[Cc] %f %f %f %f %f %f %n", op,
               &x1, &y1, &x2, &y2, &x, &y, &n) == 7)
    {
      if(is_relative(*op))
      {
        x  += pen_x;   y  += pen_y;
        x1 += pen_x;   y1 += pen_y;
        x2 += pen_x;   y2 += pen_y;
      }
      
      last_x2 = x2;
      last_y2 = y2;
      
      p->pushOpcode( *op );
      p->pushFloat(x1, y1, x2, y2, x, y);
      
      updatePen(x, y); // POSITION
     
      s += n;
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // SMOOTH CURVE instruction
    //
    // shorthand/smooth curveto (S, s) (4 arguments)
    //
    else
    if (sscanf(s, " %1[Ss] %f %f %f %f %n", op,
               &x2, &y2, &x, &y, &n) == 5)
    {
      do
      {
        if(is_relative(*op))
        {
          x  += pen_x;   y  += pen_y;
          x1 += pen_x;   y1 += pen_y;
          x2 += pen_x;   y2 += pen_y;
        }
        
        //TODO : fix
        x1 = 2 * pen_x - last_x2;
        y1 = 2 * pen_y - last_y2;
        
        last_x2 = x2;
        last_y2 = y2;
        
        p->pushOpcode( 'C' );
        p->pushFloat(x1, y1, x2, y2, x, y);
        
        updatePen(x, y); // POSITION
        
        s += n;
      }
      while (sscanf(s, "%f %f %f %f %n", &x2, &y2, &x, &y, &n) == 4);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // QUADRATIC CURVE instruction
    //
    // Quadratic Bezier curveTo (Q, q) (4 arguments)
    //
    else
    if (sscanf(s, " %1[Qq] %f %f %f %f %n", op,
               &x1, &y1, &x, &y, &n) == 5)
    {
      if(is_relative(*op))
      {
        x  += pen_x;   y  += pen_y;
        x1 += pen_x;   y1 += pen_y;
      }
      
      p->pushOpcode( *op );
      p->pushFloat(x1, y1, x, y);
      
      updatePen(x, y); // POSITION

      s += n;
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // QUADRATIC CURVE  (continuation) instruction
    //
    // shorthand/smooth quadratic Bezier curveto (T, t)
    // (2 arguments)
    //
    else
    if (sscanf(s, " %1[Tt] %f %f %n", op,
               &x, &y, &n) == 3)
    {
      do
      {
        if(is_relative(*op))
        {
          x += pen_x;
          y += pen_y;
        }
        
        x1 = 2 * pen_x + pen_x - x1;
        y1 = 2 * pen_y + pen_y - y1;
        
        p->pushOpcode( 'Q' );
        p->pushFloat(x1, y1, x, y);
        
        updatePen(x, y); // POSITION
        
        s += n;
      }
      while (sscanf(s, "%f %f %n", &x, &y, &n) == 2);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    else
    {
      fprintf(stderr, "\nparse failed at \"%s\"\n", s);
      break;
    }
  }//WHILE
  
  p->sendPromise();
  
  return RT_OK;
}

//====================================================================================================================================

void pxPath::pushOpcode(uint8_t op)
{
  uint8_t opcode = toupper(op); // Always ABSOLUTE
  
  opStream.push_back( opcode );
  
//  // Add Length
//  switch(opcode)
//  {
//    case SVG_OP_MOVE:    opStream.push_back( SVG_LEN_MOVE     ); break;
//    case SVG_OP_LINE:    opStream.push_back( SVG_LEN_LINE     ); break;
//    case SVG_OP_Q_CURVE: opStream.push_back( SVG_LEN_Q_CURVE  ); break;
//    case SVG_OP_C_CURVE: opStream.push_back( SVG_LEN_C_CURVE  ); break;
//    
//    case SVG_OP_CLOSE:   opStream.push_back( 0 );                break;
//    default:
//      rtLogError(" unrecoginzed OpCode: %0x02X", op);
//  }
}

void pxPath::pushFloat(float f)
{
  floatBytes_t fb = {.f = f};
  
  opStream.push_back( fb.bytes[3] );
  opStream.push_back( fb.bytes[2] );
  opStream.push_back( fb.bytes[1] );
  opStream.push_back( fb.bytes[0] );
}

void pxPath::pushFloat(float a, float b)
{
  pushFloat(a);
  pushFloat(b);
}

void pxPath::pushFloat(float a, float b, float c, float d)
{
  pushFloat(a);
  pushFloat(b);
  pushFloat(c);
  pushFloat(d);
}


void pxPath::pushFloat(float a, float b, float c, float d, float e, float f)
{
  pushFloat(a);
  pushFloat(b);
  pushFloat(c);
  pushFloat(d);
  pushFloat(e);
  pushFloat(f);
}


float pxPath::getFloatAt(int i)
{
  if(opStream.size() == 0)
  {
    return 0;
  }
  
  floatBytes_t fb = {.bytes =
    {
      opStream[i + 3], opStream[i + 2],
      opStream[i + 1], opStream[i + 0]
    }
  };
  
  return fb.f;
}

uint8_t pxPath::getByteAt(const uint8_t *p)
{
  return *p;
}

float pxPath::getFloatAt(const uint8_t *p)
{
  if(opStream.size() == 0)
  {
    return 0;
  }
  
  floatBytes_t fb = {.bytes =
    {
      p[3], p[2], p[1], p[0]
    }
  };
  
  return fb.f;
}


rtDefineObject(pxPath, pxObject);

rtDefineProperty(pxPath, d);

rtDefineProperty(pxPath, fillColor);
rtDefineProperty(pxPath, strokeColor);
rtDefineProperty(pxPath, strokeWidth);
