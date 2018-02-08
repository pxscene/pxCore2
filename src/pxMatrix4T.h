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

// pxMatrix4T.h

#ifndef PX_MATRIX4T_H
#define PX_MATRIX4T_H

#include <stdlib.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
template<class X, class S, class C>
void sincosf(X x, S* s, C* c)
{
  if (s)
    *s = sin(x);
  if (c)
    *c = cos(x);
}
#endif
#include <math.h>
#include <string.h>
#include <stdio.h>

#if defined(PX_PLATFORM_GENERIC_EGL) || defined(PX_PLATFORM_GENERIC_DFB) || defined(PX_PLATFORM_DFB_NON_X11) || defined(__APPLE__)
#ifndef sincos

void sincos(double x, double *s, double *c);

#endif
#endif //defined(PX_PLATFORM_GENERIC_EGL) || defined(PX_PLATFORM_GENERIC_DFB) 

#if defined(PX_PLATFORM_GENERIC_EGL) || defined(PX_PLATFORM_GENERIC_DFB) || defined(PX_PLATFORM_DFB_NON_X11) || defined(__APPLE__)
#ifndef sincosf

void sincosf(float x, float *s, float *c);

#endif
#endif //defined(PX_PLATFORM_GENERIC_EGL) || defined(PX_PLATFORM_GENERIC_DFB)


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
///
///

typedef float RealType;   // float

///
///
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


template<typename RealT> class pxMatrix4T;

template <typename RealT = RealType>
class pxVector4T 
{
friend class pxMatrix4T<RealT>;
public:
  pxVector4T(): mX(0), mY(0), mZ(0), mW(1) {}
  pxVector4T(RealT x, RealT y, RealT z = 0, RealT w = 1)
  {
    mX = x; mY = y, mZ = z; mW = w;
  }
  
  inline void setX(RealT x) { mX = x; }
  inline void setY(RealT y) { mY = y; }
  inline void setZ(RealT z) { mZ = z; }
  inline void setW(RealT w) { mW = w; }
  
  inline RealT x() {return mX;}
  inline RealT y() {return mY;}
  inline RealT z() {return mZ;}
  inline RealT w() {return mW;}

  void dump()
  {
    printf("Dumping pxVector4f\n");
    printf("[ %f, %f, %f, %f]\n", mX, mY, mZ, mW);
  }

private:
  float mX, mY, mZ, mW;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
///
///
typedef pxVector4T<RealType> pxVertex;
///
///
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template <typename RealT = RealType>
class pxMatrix4T 
{
public:
  pxMatrix4T()                    {identity();}
  pxMatrix4T(const pxMatrix4T& m) {copy(m);   }
  
  inline RealT* data()            { return mValues;    }
  RealT constData(int i)    const { return mValues[i]; }
  
  void copy(const pxMatrix4T& m) {memcpy(mValues, m.mValues, sizeof(mValues));}
  
  void multiply(pxMatrix4T& mat) 
  {
    RealT* a = mValues;
    RealT* b = mat.mValues;
    
    RealT out[16];

    RealT a00 = a[0],  a01 = a[1],  a02 = a[2],  a03 = a[3],
        a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7],
        a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11],
        a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

    // Cache only the current line of the second matrix
    RealT b0  = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
    out[0] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
    out[1] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
    out[2] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
    out[3] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

    b0 = b[4]; b1 = b[5]; b2 = b[6]; b3 = b[7];
    out[4] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
    out[5] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
    out[6] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
    out[7] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

    b0 = b[8]; b1 = b[9]; b2 = b[10]; b3 = b[11];
    out[8] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
    out[9] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
    out[10] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
    out[11] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

    b0 = b[12]; b1 = b[13]; b2 = b[14]; b3 = b[15];
    out[12] = b0*a00 + b1*a10 + b2*a20 + b3*a30;
    out[13] = b0*a01 + b1*a11 + b2*a21 + b3*a31;
    out[14] = b0*a02 + b1*a12 + b2*a22 + b3*a32;
    out[15] = b0*a03 + b1*a13 + b2*a23 + b3*a33;

    memcpy(a, &out, sizeof out);
  }
  
  pxVector4T<RealT> multiply(const pxVector4T<RealT>& v) 
  {
    pxVector4T<RealT> out;
    RealT x = v.mX;
    RealT y = v.mY;
    RealT z = v.mZ;
    RealT w = v.mW;
    RealT* m = mValues;
    out.mX = m[0] * x + m[4] * y + m[8] * z + m[12] * w;
    out.mY = m[1] * x + m[5] * y + m[9] * z + m[13] * w;
    out.mZ = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
    out.mW = m[3] * x + m[7] * y + m[11] * z + m[15] * w;
    return out;
  }
  
  
  
  inline void rotate(double angle) { rotateInRadians(angle, 0, 0, 1); }  // LEGACY
  
  inline void rotateInRadians(RealT angle) 
  {
    rotateInRadians(angle, 0, 0, 1);
  }
  
  inline void rotateInDegrees(RealT angle) 
  {
    rotateInRadians(angle * M_PI/180.0, 0, 0, 1);
  }
#ifdef ANIMATION_ROTATE_XYZ
  void rotateInDegrees(RealT angle, RealT x, RealT y, RealT z) 
  {
    rotateInRadians(angle * M_PI/180.0, x, y, z);
  }
#endif // ANIMATION_ROTATE_XYZ  

void multiply(RealT* m, RealT* n) 
{
  
  RealT tmp[16];
  const RealT *row, *column;
  div_t d;
  int i, j;
  
  for (i = 0; i < 16; i++) {
    tmp[i] = 0;
    d = div(i, 4);
    row = n + d.quot * 4;
    column = m + d.rem;
    for (j = 0; j < 4; j++)
      tmp[i] += row[j] * column[j * 4];
  }
  memcpy(m, &tmp, sizeof tmp);
}

  void rotateInRadians(RealT angle, RealT x, RealT y, RealT z) 
  {
#if 1
    RealT* m = mValues;
    RealT s, c;
    
    sincosf(angle, &s, &c);

    RealT r[16] = {
      x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
      x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0,
      x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
      0, 0, 0, 1
    };

    multiply(m, r);
#else
    rotateZInRadians(angle);
#endif
  }

  void rotateZInDegrees(RealT angle) 
  {
    rotateZInRadians(angle * M_PI/180.0);
  }

  void rotateZInRadians(RealT angle) 
  {
    RealT *out = mValues;
    RealT *a   = mValues;
    RealT s, c;
    RealT a00, a01, a02, a03, a10, a11, a12, a13;
    sincosf(angle, &s, &c);
    
    a00 = a[0];
    a01 = a[1];
    a02 = a[2];
    a03 = a[3];
    a10 = a[4];
    a11 = a[5];
    a12 = a[6];
    a13 = a[7];
    
    // Perform axis-specific matrix multiplication
    out[0] = a00 * c + a10 * s;
    out[1] = a01 * c + a11 * s;
    out[2] = a02 * c + a12 * s;
    out[3] = a03 * c + a13 * s;
    out[4] = a10 * c - a00 * s;
    out[5] = a11 * c - a01 * s;
    out[6] = a12 * c - a02 * s;
    out[7] = a13 * c - a03 * s;
  }

  void scale(RealT sx, RealT sy) 
  {
    RealT *out = mValues;
    RealT *a = mValues;

    out[0] = a[0] * sx;
    out[1] = a[1] * sx;
    out[2] = a[2] * sx;
    out[3] = a[3] * sx;
    out[4] = a[4] * sy;
    out[5] = a[5] * sy;
    out[6] = a[6] * sy;
    out[7] = a[7] * sy;
  }

  void scale(RealT sx, RealT sy, RealT sz) 
  {
    RealT *out = mValues;
    RealT *a   = mValues;

    out[0] = a[0] * sx;
    out[1] = a[1] * sx;
    out[2] = a[2] * sx;
    out[3] = a[3] * sx;
    out[4] = a[4] * sy;
    out[5] = a[5] * sy;
    out[6] = a[6] * sy;
    out[7] = a[7] * sy;

    if (sz != 1.0)
    {
      out[8] = a[8] * sz;
      out[9] = a[9] * sz;
      out[10] = a[10] * sz;
      out[11] = a[11] * sz;
    }
  }

  void translate(RealT x, RealT y) 
  {
    RealT *m = mValues;
    
    m[12] = m[0] * x + m[4] * y + m[12];
    m[13] = m[1] * x + m[5] * y + m[13];
    m[14] = m[2] * x + m[6] * y + m[14];
    m[15] = m[3] * x + m[7] * y + m[15];
  }

  void translate(RealT x, RealT y, RealT z) 
  {
    RealT *m = mValues;
    
    if (z != 0.0)
    {
      m[12] = m[0] * x + m[4] * y + m[8] * z + m[12];
      m[13] = m[1] * x + m[5] * y + m[9] * z + m[13];
      m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
      m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
    }
    else
    {
      m[12] = m[0] * x + m[4] * y + m[12];
      m[13] = m[1] * x + m[5] * y + m[13];
      m[14] = m[2] * x + m[6] * y + m[14];
      m[15] = m[3] * x + m[7] * y + m[15];
    }
  }

  bool isTranslatedOnly()
  {
    RealT *m = mValues;

    return (m[0]  == 1.0 && m[5]  == 1.0 && m[10] == 1.0 && /*m[15] == 1.0 && */
            m[1]  == 0.0 && m[2]  == 0.0 && m[3]  == 0.0 &&
            m[4]  == 0.0 && m[6]  == 0.0 && m[7]  == 0.0 &&
            m[8]  == 0.0 && m[9]  == 0.0 && m[11] == 0.0  );
  }

  RealT translateX()
  {
    RealT *m = mValues;
    
    return m[12];
  }
  
  RealT translateY()
  {
    RealT *m = mValues;
    
    return m[13];
  }

  void identity() 
  {
    RealT* m = mValues;
    RealT t[16] = 
    {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0,
    };
    
    memcpy(m, t, sizeof(t));
  }

  bool isIdentity()
  {
    RealT* m = mValues;
    
    return (m[0]  == 1.0 && m[5]  == 1.0 && m[10] == 1.0 && m[15] == 1.0 &&
            m[1]  == 0.0 && m[2]  == 0.0 && m[3]  == 0.0 &&
            m[4]  == 0.0 && m[6]  == 0.0 && m[7]  == 0.0 &&
            m[8]  == 0.0 && m[9]  == 0.0 && m[11] == 0.0 &&
            m[12] == 0.0 && m[13] == 0.0 && m[14] == 0.0);
  }

  void transpose() 
  {
    RealT*    m = mValues;
    RealT t[16] =
    {
      m[0], m[4], m[8],  m[12],
      m[1], m[5], m[9],  m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15]};
    
    memcpy(m, t, sizeof(t));
  }

  /**
   * Inverts a 4x4 matrix.
   *
   * This function can currently handle only pure translation-rotation matrices.
   * Read http://www.gamedev.net/community/forums/topic.asp?topic_id=425118
   * for an explanation.
   */
  void invert() 
  {
    RealT*   a = mValues;
    RealT* out = mValues;
    
    RealT a00 = a[0];
    RealT a01 = a[1];
    RealT a02 = a[2];
    RealT a03 = a[3];
    RealT a10 = a[4];
    RealT a11 = a[5];
    RealT a12 = a[6];
    RealT a13 = a[7];
    RealT a20 = a[8];
    RealT a21 = a[9];
    RealT a22 = a[10];
    RealT a23 = a[11];
    RealT a30 = a[12];
    RealT a31 = a[13];
    RealT a32 = a[14];
    RealT a33 = a[15];
    
    RealT b00 = a00 * a11 - a01 * a10;
    RealT b01 = a00 * a12 - a02 * a10;
    RealT b02 = a00 * a13 - a03 * a10;
    RealT b03 = a01 * a12 - a02 * a11;
    RealT b04 = a01 * a13 - a03 * a11;
    RealT b05 = a02 * a13 - a03 * a12;
    RealT b06 = a20 * a31 - a21 * a30;
    RealT b07 = a20 * a32 - a22 * a30;
    RealT b08 = a20 * a33 - a23 * a30;
    RealT b09 = a21 * a32 - a22 * a31;
    RealT b10 = a21 * a33 - a23 * a31;
    RealT b11 = a22 * a33 - a23 * a32;
    
    // Calculate the determinant
    RealT det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

#if 0 // not invertable... error
    if (!det) 
      return null;
#endif

    det = 1.0 / det;

    out[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
    out[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
    out[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
    out[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
    out[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
    out[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
    out[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
    out[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
    out[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
    out[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
    out[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
    out[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
    out[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
    out[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
    out[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
    out[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;
  }

  void perspective(float fovy, float aspect, float zNear, float zFar) 
  {
    RealT* m = mValues;
    RealT tmp[16];
    identity(tmp);
    
    RealT sine, cosine, cotangent, deltaZ;
    RealT radians = fovy / 2 * M_PI / 180;
    
    deltaZ = zFar - zNear;
    
    sincos(radians, &sine, &cosine);
    
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
      return;
    
    cotangent = cosine / sine;
    
    tmp[0] = cotangent / aspect;
    tmp[5] = cotangent;
    tmp[10] = -(zFar + zNear) / deltaZ;
    tmp[11] = -1;
    tmp[14] = -2 * zNear * zFar / deltaZ;
    tmp[15] = 0;
    
    memcpy(m, tmp, sizeof(tmp));
  }

  void dump(const char* n = NULL)
  {
    RealT* p = mValues;
    printf("Dump 4x4 Matrix: %s\n", n?n:"none");
    for (int i = 0; i < 4; i++)
    {
      printf("[ %f, %f, %f, %f]\n", p[0], p[1], p[2], p[3]);
      p +=4;
    }
  }

private:
  RealT mValues[16];
};

typedef pxVector4T<float> pxVector4f;
typedef pxMatrix4T<float> pxMatrix4f;

#endif
