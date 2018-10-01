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

template<typename FloatT> class pxMatrix4T;

template <typename FloatT = float>
class pxVector4T 
{
friend class pxMatrix4T<FloatT>;
public:
  pxVector4T(): mX(0), mY(0), mZ(0), mW(1) {}
  pxVector4T(float x, float y, float z = 0, float w = 1) 
  {
    mX = x; mY = y, mZ = z; mW = w;
  }

  inline void setXY( FloatT x, FloatT y) { mX = x; mY = y; }
  inline void setXYZ(FloatT x, FloatT y, FloatT z) { mX = x; mY = y; mZ = z; }

  inline void setX(FloatT x) { mX = x; }
  inline void setY(FloatT y) { mY = y; }
  inline void setZ(FloatT z) { mZ = z; }
  inline void setW(FloatT w) { mW = w; }
  
  inline FloatT x() {return mX;}
  inline FloatT y() {return mY;}
  inline FloatT z() {return mZ;}
  inline FloatT w() {return mW;}

  void dump()
  {
    printf("Dumping pxVector4f\n");
    printf("[ %f, %f, %f, %f]\n", mX, mY, mZ, mW);
  }

private:
  float mX, mY, mZ, mW;
};


///
///
#define pxVertex pxVector4T<float>
///
///

template <typename FloatT = float>
class pxMatrix4T 
{
public:
  pxMatrix4T()                    {identity();}
  pxMatrix4T(const pxMatrix4T& m) {copy(m);   }
  
  inline FloatT* data() {return mValues;}
  FloatT constData(int i) const {return mValues[i];}
  
  void copy(const pxMatrix4T& m) {memcpy(mValues, m.mValues, sizeof(mValues));}
  
  void multiply(pxMatrix4T& mat) 
  {
    FloatT* a = mValues;
    FloatT* b = mat.mValues;
    
    FloatT out[16];

    float a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3],
        a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7],
        a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11],
        a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

    // Cache only the current line of the second matrix
    float b0  = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
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
  
  pxVector4T<FloatT> multiply(const pxVector4T<FloatT>& v) 
  {
    pxVector4T<FloatT> out;
    float x = v.mX;
    float y = v.mY;
    float z = v.mZ;
    float w = v.mW;
    float* m = mValues;
    out.mX = m[0] * x + m[4] * y + m[8] * z + m[12] * w;
    out.mY = m[1] * x + m[5] * y + m[9] * z + m[13] * w;
    out.mZ = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
    out.mW = m[3] * x + m[7] * y + m[11] * z + m[15] * w;
    return out;
  }
  
  
  
  inline void rotate(double angle) { rotateInRadians(angle, 0, 0, 1); }  // LEGACY
  
  inline void rotateInRadians(FloatT angle) 
  {
    rotateInRadians(angle, 0, 0, 1);
  }
  
  inline void rotateInDegrees(FloatT angle) 
  {
    rotateInRadians(angle * M_PI/180.0, 0, 0, 1);
  }
#ifdef ANIMATION_ROTATE_XYZ
  void rotateInDegrees(FloatT angle, FloatT x, FloatT y, FloatT z) 
  {
    rotateInRadians(static_cast<float> (angle * M_PI/180.0), static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
  }
#endif // ANIMATION_ROTATE_XYZ  

void multiply(FloatT* m, FloatT* n) 
{
  
  FloatT tmp[16];
  const FloatT *row, *column;
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

  void rotateInRadians(FloatT angle, FloatT x, FloatT y, FloatT z) 
  {
#if 1
    FloatT* m = mValues;
    FloatT s, c;
    
    sincosf(angle, &s, &c);

    float r[16] = {
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

  void rotateZInDegrees(FloatT angle) 
  {
    rotateZInRadians(angle * M_PI/180.0);
  }

  void rotateZInRadians(FloatT angle) 
  {
    FloatT *out = mValues;
    FloatT *a = mValues;
    FloatT s, c;
    FloatT a00, a01, a02, a03, a10, a11, a12, a13;
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

  void scale(FloatT sx, FloatT sy) 
  {
    FloatT *out = mValues;
    FloatT *a = mValues;

    out[0] = a[0] * sx;
    out[1] = a[1] * sx;
    out[2] = a[2] * sx;
    out[3] = a[3] * sx;
    out[4] = a[4] * sy;
    out[5] = a[5] * sy;
    out[6] = a[6] * sy;
    out[7] = a[7] * sy;
  }

  void scale(FloatT sx, FloatT sy, FloatT sz) 
  {
    FloatT *out = mValues;
    FloatT *a = mValues;

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

  void translate(FloatT x, FloatT y) 
  {
    FloatT *m = mValues;
    
    m[12] = m[0] * x + m[4] * y + m[12];
    m[13] = m[1] * x + m[5] * y + m[13];
    m[14] = m[2] * x + m[6] * y + m[14];
    m[15] = m[3] * x + m[7] * y + m[15];
  }

  void translate(FloatT x, FloatT y, FloatT z) 
  {
    FloatT *m = mValues;
    
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
    FloatT *m = mValues;

    return (m[0]  == 1.0 && m[5]  == 1.0 && m[10] == 1.0 && /*m[15] == 1.0 && */
            m[1]  == 0.0 && m[2]  == 0.0 && m[3]  == 0.0 &&
            m[4]  == 0.0 && m[6]  == 0.0 && m[7]  == 0.0 &&
            m[8]  == 0.0 && m[9]  == 0.0 && m[11] == 0.0  );
  }

  FloatT translateX()
  {
    FloatT *m = mValues;
    
    return m[12];
  }
  
  FloatT translateY()
  {
    FloatT *m = mValues;
    
    return m[13];
  }

  void identity() 
  {
    FloatT* m = mValues;
    FloatT t[16] = 
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
    FloatT* m = mValues;
    
    return (m[0]  == 1.0 && m[5]  == 1.0 && m[10] == 1.0 && m[15] == 1.0 &&
            m[1]  == 0.0 && m[2]  == 0.0 && m[3]  == 0.0 &&
            m[4]  == 0.0 && m[6]  == 0.0 && m[7]  == 0.0 &&
            m[8]  == 0.0 && m[9]  == 0.0 && m[11] == 0.0 &&
            m[12] == 0.0 && m[13] == 0.0 && m[14] == 0.0);
  }

  void transpose() 
  {
    FloatT* m = mValues;
    FloatT t[16] = {
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
    float* a = mValues;
    float* out = mValues;
    
    float a00 = a[0];
    float a01 = a[1];
    float a02 = a[2];
    float a03 = a[3];
    float a10 = a[4];
    float a11 = a[5];
    float a12 = a[6];
    float a13 = a[7];
    float a20 = a[8];
    float a21 = a[9];
    float a22 = a[10];
    float a23 = a[11];
    float a30 = a[12];
    float a31 = a[13];
    float a32 = a[14];
    float a33 = a[15];
    
    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;
    
    // Calculate the determinant
    float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

#if 0 // not invertable... error
    if (!det) 
      return null;
#endif

    det = static_cast<float> (1.0 / det);

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
    FloatT* m = mValues;
    FloatT tmp[16];
    identity(tmp);
    
    double sine, cosine, cotangent, deltaZ;
    float radians = fovy / 2 * M_PI / 180;
    
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
    FloatT* p = mValues;
    printf("Dump 4x4 Matrix: %s\n", n?n:"none");
    for (int i = 0; i < 4; i++)
    {
      printf("[ %f, %f, %f, %f]\n", p[0], p[1], p[2], p[3]);
      p +=4;
    }
  }
  bool isEqual(const pxMatrix4T& m)
  {
    // translation euality check
    if (mValues[3] != m.mValues[3] || mValues[7] != m.mValues[7] || mValues[11] != m.mValues[11])
        return false;
        
    // scale equality check
    if (mValues[12] != m.mValues[12] || mValues[13] != m.mValues[13] || mValues[14] != m.mValues[14])
        return false;
        
    // rotation equality check
    if (mValues[1] != m.mValues[1] || mValues[2] != m.mValues[2] || mValues[4] != m.mValues[4] || mValues[6] != m.mValues[6] || mValues[8] != m.mValues[8] || mValues[9] != m.mValues[9])
        return false;
        
    /*for (int i = 14; i > 0 && i != 5; --i)
        if (mValues[i] != m.mValues[i])
        return false;*/
    return true;
    }
    
private:
  FloatT mValues[16];
};

typedef pxVector4T<float> pxVector4f;
typedef pxMatrix4T<float> pxMatrix4f;

#endif
