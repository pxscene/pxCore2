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


extern pxContext context;

//--------------------------------------------
// For ARC to BEZIER
typedef double a2cReal_t;
typedef struct point2d
{
  a2cReal_t x;
  a2cReal_t y;
}
point2d_t;


typedef struct bcurve
{
  point2d_t xy1;
  point2d_t xy2;
  point2d_t xy;
}
bcurve_t;

typedef std::list<bcurve_t> bcurves_t;
typedef bcurves_t::const_iterator   bcurves_iter_t;

//void testA2C(); //fwd
static bcurves_t arcToBezier(a2cReal_t px, a2cReal_t py,
                             a2cReal_t cx, a2cReal_t cy,
                             a2cReal_t rx, a2cReal_t ry,
                             a2cReal_t xAxisRotation,
                             a2cReal_t largeArcFlag,
                             a2cReal_t sweepFlag);//fwd
//--------------------------------------------



pxPath::pxPath(pxScene2d* scene): pxObject(scene), mStrokeColor(pxClear), mStrokeWidth(0), mFillColor(pxClear)
{
  mx = 0;
  my = 0;
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
 
  printf("\n TEST... op: %d  len: %d  >>>   moveTo(%.0f, %f) ",op, len, x,y);
*/
  
//  testA2C();
  
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
  rtObjectRef canvasRef = mScene->getCanvas();
  
//  canvasRef.send("drawPath", this);
  
  pxCanvas *c = (pxCanvas *) canvasRef.getPtr();
  
  if(c)
  {
    pxOffscreen &o = c->offscreen();

   // o.fill(pxColor(255,0,0,8));                    // HACK BACKFILL

    canvasRef.send("drawPath", this);

    context.drawOffscreen(0,0, 0,0, 1280, 720, /* o.width(), o.height(),*/ o);
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

// static float max_w = 0;
// static float max_h = 0;

static float min_x = 99999;
static float min_y = 99999;

static float max_x = 0;
static float max_y = 0;


static float last_pen_x = 0;
static float last_pen_y = 0;

static float pen_x = 0;
static float pen_y = 0;

void updatePen(float px, float py)
{
  pen_x = px;
  pen_y = py;
}

void updateBounds()
{
  if( pen_x <= min_x) min_x = pen_x;
  if( pen_x >= max_x) max_x = pen_x;
  
  if( pen_y <= min_y) min_y = pen_y;
  if( pen_y >= max_y) max_y = pen_y;
}

#define is_relative(xx) (islower(xx))

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
    float x0, y0, x1, y1, x2, y2, rx, ry;
    
    float last_x2 = 0.0, last_y2 = 0.0, xrot;
    
    int n;
    
    int lflag; // ARC ... "large-arc-flag"
    int sflag; // ARC ... "sweep-flag"
    
    last_pen_x = pen_x;
    last_pen_y = pen_y;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // MOVE / LINE instruction
    //
    // SHORTHAND:  moveto (M, m), lineto (L, l) (2 arguments)
    //
    if (sscanf(s, " %1[MmLl] %f %f %n", op, &x0, &y0, &n) == 3)
    {
      do
      {
        x0 += p->x();
        y0 += p->y();
        
        if(is_relative(*op))
        {
          x0 += pen_x;  y0 += pen_y;
        }
        
        p->pushOpcode( *op );
        p->pushFloat(x0,y0);

        updatePen(x0, y0); // POSITION
        updateBounds();
        
//        if(*op == 'M' || *op == 'm')
//        {
//          last_pen_x = pen_x;
//          last_pen_y = pen_y;
//        }
        
//        if(*op == 'M' || *op == 'm')
//        {
//          printf("\nPath:   SVG_OP_MOVE( %.0f, %.0f) ",x0, y0);
//        }
//        else
//        {
//          printf("\nPath:   SVG_OP_LINE( %.0f, %.0f) ",x0, y0);
//        }
        
        s += n;
      }
      while (sscanf(s, "%f %f %n", &x0, &y0, &n) == 2);
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
    if (sscanf(s, " %1[Hh] %f %n", op, &x0, &n) == 2)
    {
      do
      {
        if(is_relative(*op))
        {
          x0 += pen_x;
        }
        
        p->pushOpcode( 'L' );
        p->pushFloat(x0,y0);
        
        updatePen(x0, y0); // POSITION
        updateBounds();

//        printf("\nPath:   SVG_OP_H_LINE_TO( x0: %.0f, y0: %.0f) ", x0, y0);
        
        s += n; x0 = y0 = 0;
      }
      while (sscanf(s, "%f %n", &x0, &n) == 1);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // VERTICAL LINE instruction
    //
    // vertical lineto (V, v) (1 argument)
    //
    else
    if (sscanf(s, " %1[Vv] %f %n", op, &y0, &n) == 2)
    {
      do
      {
        if(is_relative(*op))
        {
          y0 += pen_y;
        }
        
        p->pushOpcode( 'L' );
        p->pushFloat(x0,y0);
        
        updatePen(x0, y0); // POSITION
        updateBounds();

//        printf("\nPath:   SVG_OP_V_LINE_TO( x0: %.0f, y0: %.0f) ", x0, y0);
        
        s += n; x0 = y0 = 0;
      }
      while (sscanf(s, "%f %n", &y0, &n) == 1);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // ARC instruction
    //
    //  "A rx ry x-axis-rotation large-arc-flag sweep-flag x y"
    //
    else
    if (sscanf(s, " %1[Aa] %f %f %f %d %d %f %f %n", op,
               &rx, &ry, &xrot, &lflag, &sflag, &x0, &y0, &n) == 8)
    {
      if(is_relative(*op))
      {
     //   x0 += pen_x;   y0 += pen_y;
     //   rx += pen_x;   ry += pen_y;
      }
      
      bcurves_t ans = arcToBezier(pen_x, pen_y, // pxy            PREVIOUS
                                     x0,  y0,   // cxy            CURRENT
                                     rx,  ry,   // rxy            CURVE CENTER
                                   xrot,        // xAxisRotation
                                  lflag,        // largeArcFlag
                                  sflag         // sweepFlag
                                  );

      for (bcurves_iter_t it = ans.begin(), end = ans.end();
           it != end; ++it)
      {
        bcurve_t c = *it;
        
        x1 = c.xy1.x;  x2 = c.xy2.x;  x0 = c.xy.x;
        y1 = c.xy1.y;  y2 = c.xy2.y;  y0 = c.xy.y;
        
//        printf("\nARC:  x1: %f   y1: %f   x2: %f   y2: %f   x0: %f   y0: %f", x1, y1, x2, y2, x0, y0);
        
        // Queue BEZIER curve control points.
        p->pushOpcode( 'C' );
        p->pushFloat(x1, y1, x2, y2, x0, y0);
      }
      
      s += n;
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // CUBIC CURVE instruction
    //
    //  curveTo (C, c) (6 arguments)
    //
    else
    if (sscanf(s, " %1[Cc] %f %f %f %f %f %f %n", op,
               &x1, &y1, &x2, &y2, &x0, &y0, &n) == 7)
    {
//      printf("\nPath:   SVG_OP_C_CURVE( x1: %.0f, y1: %.0f,  x2: %.0f, y2: %.0f,  x0: %.0f, y0: %.0f) ", x1, y1, x2, y2, x0, y0);
      
      if(is_relative(*op))
      {
        x0 += pen_x;   y0 += pen_y;
        x1 += pen_x;   y1 += pen_y;
        x2 += pen_x;   y2 += pen_y;
      }
      
      last_x2 = x2;
      last_y2 = y2;
      
      p->pushOpcode( *op );
      p->pushFloat(x1, y1, x2, y2, x0, y0);
      
      updatePen(x0, y0); // POSITION
      updateBounds();
      
      s += n;
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // SMOOTH CURVE instruction
    //
    // shorthand/smooth curveto (S, s) (4 arguments)
    //
    else
    if (sscanf(s, " %1[Ss] %f %f %f %f %n", op,
               &x2, &y2, &x0, &y0, &n) == 5)
    {
      do
      {
        if(is_relative(*op))
        {
          x0 += pen_x;   y0 += pen_y;
          x1 += pen_x;   y1 += pen_y;
          x2 += pen_x;   y2 += pen_y;
        }
        
        //TODO : fix
        x1 = 2 * pen_x - last_x2;
        y1 = 2 * pen_y - last_y2;
        
        last_x2 = x2;
        last_y2 = y2;
        
        p->pushOpcode( 'C' );
        p->pushFloat(x1, y1, x2, y2, x0, y0);
        
//        printf("\nPath:   SVG_OP_S_CURVE( x1: %.0f, y1: %.0f,  x2: %.0f, y2: %.0f,  x0: %.0f, y0: %.0f) ", x1, y1, x2, y2, x0, y0);

        updatePen(x0, y0); // POSITION
        updateBounds();
        
        s += n;
      }
      while (sscanf(s, "%f %f %f %f %n", &x2, &y2, &x0, &y0, &n) == 4);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // QUADRATIC CURVE instruction
    //
    // Quadratic Bezier curveTo (Q, q) (4 arguments)
    //
    else
    if (sscanf(s, " %1[Qq] %f %f %f %f %n", op,
               &x1, &y1, &x0, &y0, &n) == 5)
    {
      if(is_relative(*op))
      {
        x0 += pen_x;   y0 += pen_y;
        x1 += pen_x;   y1 += pen_y;
      }
      
      p->pushOpcode( *op );
      p->pushFloat(x1, y1, x0, y0);
      
//      printf("\nPath:   SVG_OP_Q_CURVE( x1: %.0f, y1: %.0f,  x0: %.0f, y0: %.0f) ", x1, y1, x0, y0);
      
      updatePen(x0, y0); // POSITION
      updateBounds();
      
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
               &x0, &y0, &n) == 3)
    {
      do
      {
        if(is_relative(*op))
        {
          x0 += pen_x;
          y0 += pen_y;
        }
        
        x1 = (2 * pen_x) - x1;
        y1 = (2 * pen_y) - y1;
        
        p->pushOpcode( 'Q' );
        p->pushFloat(x1, y1, x0, y0);

//        printf("\nPath:   SVG_OP_T_CURVE( x1: %.0f, y1: %.0f,  x0: %.0f, y0: %.0f) ", x1, y1, x0, y0);

        updatePen(x0, y0); // POSITION
        updateBounds();
        
        s += n;
      }
      while (sscanf(s, "%f %f %n", &x0, &y0, &n) == 2);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    else
    {
      fprintf(stderr, "\n path parse failed at \"%s\"\n", s);
      break;
    }
    
    //fprintf(stderr, "\n dbg - x0: %.0f  x1: %.0f  x2: %.0f", x0, x1, x2);

  }//WHILE
  
  float dw = fabs(max_x - min_x);
  float dh = fabs(max_y - min_y);
  
  dw = (dw > 0) ? dw : p->mStrokeWidth;
  dh = (dh > 0) ? dh : p->mStrokeWidth;
  
  p->setW( dw );
  p->setH( dh );

 // printf("\n ###  Bounds   WxH:  %.0f x  %.0f ... ", p->w(), p->h());
  
  p->sendPromise();
  
  // Reset
  min_x = 99999;  max_x = 0;
  min_y = 99999;  max_y = 0;
 
  return RT_OK;
}

//====================================================================================================================================

void pxPath::pushOpcode(uint8_t op)
{
  uint8_t opcode = toupper(op); // Always ABSOLUTE
  
  opStream.push_back( opcode );
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

//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================

// Code derived from:
//
// https://raw.githubusercontent.com/colinmeinke/svg-arc-to-cubic-bezier/master/src/index.js
//

const a2cReal_t TAU = M_PI * 2;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static point2d_t mapToEllipse(point2d_t xy, point2d_t rxy, a2cReal_t cosphi, a2cReal_t sinphi, a2cReal_t centerx, a2cReal_t centery)
{
  xy.x *= rxy.x;
  xy.y *= rxy.y;
  
  const a2cReal_t xp = cosphi * xy.x - sinphi * xy.y;
  const a2cReal_t yp = sinphi * xy.x + cosphi * xy.y;
  
  return point2d_t( { .x = xp + centerx,.y = yp + centery } );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct uarc
{
  point2d_t pt1;
  point2d_t pt2;
  point2d_t pt3;
}
uarc_t;

typedef std::list<uarc_t>           uarc_list_t;
typedef uarc_list_t::const_iterator uarc_list_iter_t;


uarc_t approxUnitArc(a2cReal_t ang1, a2cReal_t ang2)
{
  const a2cReal_t a = 4.0 / 3.0 * tan(ang2 / 4.0);
  
  const a2cReal_t x1 = cos(ang1);
  const a2cReal_t y1 = sin(ang1);
  const a2cReal_t x2 = cos(ang1 + ang2);
  const a2cReal_t y2 = sin(ang1 + ang2);
  
  return uarc_t( {
    .pt1 = point2d_t( { .x = x1 - y1 * a , .y = y1 + x1 * a } ),
    .pt2 = point2d_t( { .x = x2 + y2 * a , .y = y2 - x2 * a } ),
    .pt3 = point2d_t( { .x = x2 , .y = y2 } )
  } );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static a2cReal_t vectorAngle(a2cReal_t ux, a2cReal_t uy, a2cReal_t vx, a2cReal_t vy)
{
  const a2cReal_t sign = (ux * vy - uy * vx < 0) ? -1 : 1;
  const a2cReal_t umag = sqrt(ux * ux + uy * uy);
  const a2cReal_t vmag = sqrt(ux * ux + uy * uy);
  const a2cReal_t dot  = ux * vx + uy * vy;
  
  a2cReal_t  div = dot / (umag * vmag);
  
  if (div > 1)  {  div = 1;  }
  if (div < -1) {  div = -1; }
  
  return sign * acos(div);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct acenter
{
  point2d_t c;
  a2cReal_t     ang1;
  a2cReal_t     ang2;
}
acenter_t;

static acenter_t getArcCenter( a2cReal_t px,
                               a2cReal_t py,
                               a2cReal_t cx,
                               a2cReal_t cy,
                               a2cReal_t rx,
                               a2cReal_t ry,
                                    bool largeArcFlag,
                                    bool sweepFlag,
                               a2cReal_t sinphi,
                               a2cReal_t cosphi,
                               a2cReal_t pxp,
                               a2cReal_t pyp)
{
  const a2cReal_t rxsq  = pow(rx,  2);
  const a2cReal_t rysq  = pow(ry,  2);
  const a2cReal_t pxpsq = pow(pxp, 2);
  const a2cReal_t pypsq = pow(pyp, 2);
  
  a2cReal_t radicant = (rxsq * rysq) - (rxsq * pypsq) - (rysq * pxpsq);
  
  if (radicant < 0.0)
  {
    radicant = 0.0;
  }
  
  radicant /= (rxsq * pypsq) + (rysq * pxpsq);
  radicant = sqrt(radicant) * (largeArcFlag == sweepFlag ? -1 : 1);
  
  const a2cReal_t centerxp = radicant *  rx / ry * pyp;
  const a2cReal_t centeryp = radicant * -ry / rx * pxp;
  
  const a2cReal_t centerx = cosphi * centerxp - sinphi * centeryp + (px + cx) / 2;
  const a2cReal_t centery = sinphi * centerxp + cosphi * centeryp + (py + cy) / 2;
  
  const a2cReal_t vx1 = ( pxp - centerxp) / rx;
  const a2cReal_t vy1 = ( pyp - centeryp) / ry;
  const a2cReal_t vx2 = (-pxp - centerxp) / rx;
  const a2cReal_t vy2 = (-pyp - centeryp) / ry;
  
  a2cReal_t ang1 = vectorAngle(1.0, 0.0, vx1, vy1);
  a2cReal_t ang2 = vectorAngle(vx1, vy1, vx2, vy2);
  
  if (sweepFlag == 0.0 && ang2 > 0.0) { ang2 -= TAU; }
  if (sweepFlag == 1.0 && ang2 < 0.0) { ang2 += TAU; }
  
  return acenter_t( { .c    = point2d_t( { .x = centerx,
                                           .y = centery } ),
                      .ang1 = ang1,
                      .ang2 = ang2 }
                   );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static bcurves_t arcToBezier(a2cReal_t px, a2cReal_t py,
                             a2cReal_t cx, a2cReal_t cy,
                             a2cReal_t rx, a2cReal_t ry,
                             a2cReal_t xAxisRotation = 0.0,
                             a2cReal_t largeArcFlag  = 0.0,
                             a2cReal_t sweepFlag     = 0.0)
{
  //  const a2cReal_t curves = [];
  
  bcurves_t    bcurves;
  uarc_list_t  curves;
  
  if (rx == 0 || ry == 0)
  {
    return bcurves;
  }
  
  const a2cReal_t sinphi = sin(xAxisRotation * TAU / 360.0);
  const a2cReal_t cosphi = cos(xAxisRotation * TAU / 360.0);
  
  const a2cReal_t pxp =  cosphi * (px - cx) / 2.0 + sinphi * (py - cy) / 2.0;
  const a2cReal_t pyp = -sinphi * (px - cx) / 2.0 + cosphi * (py - cy) / 2.0;
  
  if (pxp == 0 && pyp == 0)
  {
    return bcurves;
  }
  
  rx = fabs(rx);
  ry = fabs(ry);
  
  const a2cReal_t lambda = pow(pxp, 2) / pow(rx, 2) +
  pow(pyp, 2) / pow(ry, 2);
  
  if (lambda > 1)
  {
    rx *= sqrt(lambda);
    ry *= sqrt(lambda);
  }
  
  acenter_t center =  getArcCenter( px, py,
                                    cx, cy,
                                    rx, ry,
                                      largeArcFlag,
                                      sweepFlag,
                                      sinphi,
                                      cosphi,
                                    pxp, pyp
                                   );
  
  a2cReal_t ang1 = center.ang1;
  a2cReal_t ang2 = center.ang2;
  
  const a2cReal_t segments = fmax(ceil(fabs(ang2) / (TAU / 4)), 1);
  
  ang2 /= segments;
  
  for (int i = 0; i < segments; i++)
  {
    curves.push_back( approxUnitArc(ang1, ang2) ); // uarc_t ... 3 points in a struct
    
    ang1 += ang2;
  }
  
  const point2d_t rxy = { .x = rx, .y = ry };
  
  for (uarc_list_iter_t it = curves.begin(), end = curves.end();
       it != end; ++it)
  {
    uarc_t curve = *it;
    
    point2d_t xy1 = mapToEllipse(curve.pt1, rxy, cosphi, sinphi, center.c.x, center.c.y);
    point2d_t xy2 = mapToEllipse(curve.pt2, rxy, cosphi, sinphi, center.c.x, center.c.y);
    point2d_t xy  = mapToEllipse(curve.pt3, rxy, cosphi, sinphi, center.c.x, center.c.y);
    
    bcurves.push_back( bcurve_t( {
                                  .xy1 = xy1,
                                  .xy2 = xy2,
                                  .xy = xy
                              } )
                      );
  }
 
  return bcurves;
}

#if 0
void testA2C()
{
  /*
   const previousPoint = { x: 100, y: 315 };
   
   const currentPoint = {
       x: 162,
       y: 162,
       curve: {
       type: 'arc',
       rx: 30,
       ry: 50,
       largeArcFlag: 0,
       sweepFlag: 1,
       xAxisRotation: 0,
     },
   };

   */
  
  bcurves_t ans = arcToBezier(100, 315, // pxy            PREVIOUS
                              162, 162, // cxy            CURRENT
                              30,  50,  // rxy            CURVE CENTER
                              false,    // xAxisRotation
                              false,    // largeArcFlag
                              true      // sweepFlag
                              );
  
  
  for (bcurves_iter_t it = ans.begin(), end = ans.end();
       it != end; ++it)
  {
    bcurve_t c = *it;
    
    printf("\n  x1: %f , y1: %f , x2: %f , y2: %f , x: %f , y: %f  ",
           c.xy1.x, c.xy1.y, c.xy2.x, c.xy2.y, c.xy.x, c.xy.y);
  }
  
  /*
   EXPECTED:
   
   Curve >> {"x1":74.65012998276657,"y1":286.465287925409,"x2":67.97917275524541,"y2":229.083116695389,"x":85.10000000000001,"y":186.83333333333331}
   Curve >> {"x1":102.2208272447546,"y1":144.58354997127765,"x2":136.6501299827666,"y2":133.46528792540903,"x":162,"y":162}
   */
}
#endif

//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
