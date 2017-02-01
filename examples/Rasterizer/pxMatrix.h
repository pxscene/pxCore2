#ifndef PX_MATRIX_H
#define PX_MATRIX_H

#define pxPI 3.141519

#define PX_INLINE  inline

class pxVertex
{
public:
  pxVertex() { this->x = this->y = this->z = 0.0; }
  pxVertex(double x, double y, double z = 0.0)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

public:
  double x, y, z;
};

class pxMatrix
{
public:

  PX_INLINE void identity()
  {
    mMatrix[0][0] = mMatrix[1][1] = mMatrix[2][2] = mMatrix[3][3] = 1.0;

    mMatrix[0][1] = mMatrix[0][2] = mMatrix[0][3] =
    mMatrix[1][0] = mMatrix[1][2] = mMatrix[1][3] =
    mMatrix[2][0] = mMatrix[2][1] = mMatrix[2][3] =
    mMatrix[3][0] = mMatrix[3][1] = mMatrix[3][2] = 0.0;
  }

  PX_INLINE bool isIdentity()
  {
    return (mMatrix[0][0] == 1.0 && mMatrix[1][1] == 1.0 && mMatrix[2][2] == 1.0 && mMatrix[3][3] == 1.0 &&
            mMatrix[0][1] == 0.0 && mMatrix[0][2] == 0.0 && mMatrix[0][3] == 0.0 &&
            mMatrix[1][0] == 0.0 && mMatrix[1][2] == 0.0 && mMatrix[1][3] == 0.0 &&
            mMatrix[2][0] == 0.0 && mMatrix[2][1] == 0.0 && mMatrix[2][3] == 0.0 &&
            mMatrix[3][0] == 0.0 && mMatrix[3][1] == 0.0 && mMatrix[3][2] == 0.0);
  }


  PX_INLINE bool isTranslatedOnly()
  {
    return (mMatrix[0][0] == 1.0 && mMatrix[1][1] == 1.0 && mMatrix[2][2] == 1.0 &&
            mMatrix[0][1] == 0.0 && mMatrix[0][2] == 0.0 &&
            mMatrix[1][0] == 0.0 && mMatrix[1][2] == 0.0 &&
            mMatrix[2][0] == 0.0 && mMatrix[2][1] == 0.0);
  }

  PX_INLINE void multiply(const pxMatrix& mat1, const pxMatrix& mat2, pxMatrix& result)
  {
    int i,j;
    for(i=0; i<4; i++)
      for(j=0; j<4; j++)
        result.mMatrix[i][j]=mat1.mMatrix[i][0]*mat2.mMatrix[0][j]+
          mat1.mMatrix[i][1]*mat2.mMatrix[1][j]+
          mat1.mMatrix[i][2]*mat2.mMatrix[2][j]+
          mat1.mMatrix[i][3]*mat2.mMatrix[3][j];
  }

  PX_INLINE void multiply(const pxVertex& ver, const pxMatrix& mat, pxVertex& result)
  {
    result.x = ver.x * mat.mMatrix[0][0] +
               ver.y * mat.mMatrix[1][0] +
               ver.z * mat.mMatrix[2][0] +
                 mat.mMatrix[3][0];

    result.y = ver.x * mat.mMatrix[0][1] +
               ver.y * mat.mMatrix[1][1] +
               ver.z * mat.mMatrix[2][1] +
                 mat.mMatrix[3][1];
    result.z = ver.x * mat.mMatrix[0][2] +
               ver.y * mat.mMatrix[1][2] +
               ver.z * mat.mMatrix[2][2] +
                 mat.mMatrix[3][2];
  }

  PX_INLINE void rotateXY(double angle)
  {
    pxMatrix t;
    t.identity();

    double sinValue, cosValue;

    cosValue = pxCos(angle);
    sinValue = pxSin(angle);

#if 0
    t.mMatrix[0][0] = mMatrix[0][0] * cosValue +
      mMatrix[0][1] * sinValue;
    t.mMatrix[1][0] = mMatrix[1][0] * cosValue +
      mMatrix[1][1] * sinValue;
    t.mMatrix[2][0] = mMatrix[2][0] * cosValue +
      mMatrix[2][1] * sinValue;
    t.mMatrix[3][0] = mMatrix[3][0] * cosValue +
      mMatrix[3][1] * sinValue;

    t.mMatrix[0][1] = mMatrix[0][0] * -sinValue +
      mMatrix[0][1] * cosValue;
    t.mMatrix[1][1] = mMatrix[1][0] * -sinValue +
      mMatrix[1][1] * cosValue;
    t.mMatrix[2][1] = mMatrix[2][0] * -sinValue +
      mMatrix[2][1] * cosValue;
    t.mMatrix[3][1] = mMatrix[3][0] * -sinValue +
      mMatrix[3][1] * cosValue;

    t.mMatrix[0][2] = mMatrix[0][2];
    t.mMatrix[1][2] = mMatrix[1][2];
    t.mMatrix[2][2] = mMatrix[2][2];
    t.mMatrix[3][2] = mMatrix[3][2];

    *this = t;
#else
#if 1
    // z
    t.mMatrix[0][0] =  cosValue;
    t.mMatrix[0][1] =  sinValue;
    t.mMatrix[1][0] = -sinValue;
    t.mMatrix[1][1] =  cosValue;
#elif 0
    // x
    t.mMatrix[1][1] =  cosValue;
    t.mMatrix[1][2] =  sinValue;
    t.mMatrix[2][1] = -sinValue;
    t.mMatrix[2][2] =  cosValue;
#else
    // y
    t.mMatrix[0][0] =  cosValue;
    t.mMatrix[2][0] = -sinValue;
    t.mMatrix[0][2] =  sinValue;
    t.mMatrix[2][2] =  cosValue;
#endif

    pxMatrix r;
    r.identity();
    multiply(*this, t, r);
    *this = r;
#endif

  }

  PX_INLINE void rotate(double angle) { rotateXY(angle); }

  PX_INLINE void scale(double sx, double sy, double sz)
  {
    mMatrix[0][0] *= sx;
    mMatrix[1][0] *= sx;
    mMatrix[2][0] *= sx;

    mMatrix[0][1] *= sy;
    mMatrix[1][1] *= sy;
    mMatrix[2][1] *= sy;

    mMatrix[0][2] *= sz;
    mMatrix[1][2] *= sz;
    mMatrix[2][2] *= sz;

    mMatrix[3][0] *= sx;
    mMatrix[3][1] *= sy;
    mMatrix[3][2] *= sz;
  }

  PX_INLINE void scale(double s)
  {
    scale(s, s, s);
  }

  PX_INLINE void translate(double dx, double dy, double dz = 0.0)
  {
    mMatrix[3][0] += dx;
    mMatrix[3][1] += dy;
    mMatrix[3][2] += dz;
  }

  PX_INLINE double translateX()
  {
    return mMatrix[3][0];
  }

  PX_INLINE double translateY()
  {
    return mMatrix[3][1];
  }

  PX_INLINE void transpose()
  {
    double t;
    t = mMatrix[1][0];
    mMatrix[1][0] = mMatrix[0][1];
    mMatrix[0][1] = t;
    t = mMatrix[2][0];
    mMatrix[2][0] = mMatrix[0][2];
    mMatrix[0][2] = t;
    t = mMatrix[2][2];
    mMatrix[2][2] = mMatrix[1][2];
    mMatrix[1][2] = t;
  }

private:
  double mMatrix[4][4];
};

#endif
