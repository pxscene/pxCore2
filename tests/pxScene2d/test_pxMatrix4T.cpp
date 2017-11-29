#include <sstream>

#define _USE_MATH_DEFINES
#include <cmath>

#define private public
#define protected public
#include "pxMatrix4T.h"

#define  M_ERR  0.0001

#include "test_includes.h" // Needs to be included last

class pxMatrix4Test : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void pxMatrix4TSinCosTestD()
    {
      double rads    = (double) M_PI;
      double sin_val = 0.0;
      double cos_val = 0.0;

      sincos( rads, &sin_val, &cos_val);

      EXPECT_NEAR(  0.0, sin_val, M_ERR);
      EXPECT_NEAR( -1.0, cos_val, M_ERR);
    }

    void pxMatrix4TSinCosTestF()
    {
      float rads    = M_PI;
      float sin_val = 0.0f;
      float cos_val = 0.0f;

      sincosf( rads, &sin_val, &cos_val);

      EXPECT_NEAR(   0.0f, sin_val, M_ERR);
      EXPECT_NEAR(  -1.0f, cos_val, M_ERR);
    }

    void pxMatrix4TVector4Test()
    {
       pxVector4f vec4(1,2,3,4);

       EXPECT_NEAR( 1.0f, vec4.x(), M_ERR );
       EXPECT_NEAR( 2.0f, vec4.y(), M_ERR );
       EXPECT_NEAR( 3.0f, vec4.z(), M_ERR );
       EXPECT_NEAR( 4.0f, vec4.w(), M_ERR );

       vec4.setX( 4 );
       vec4.setY( 3 );
       vec4.setZ( 2 );
       vec4.setW( 1 );

       EXPECT_NEAR( 4.0f, vec4.x(), M_ERR );
       EXPECT_NEAR( 3.0f, vec4.y(), M_ERR );
       EXPECT_NEAR( 2.0f, vec4.z(), M_ERR );
       EXPECT_NEAR( 1.0f, vec4.w(), M_ERR );
    }

    void pxMatrix4TidentityTest()
    {
      pxMatrix4f m;

      float *vals = m.data();
      float ans[] = { 1,0,0,0 };

      EXPECT_TRUE( vals != NULL );

//      m.dump("pxMatrix4TidentityTest() m ");

      for(int i = 0; i < 4; i+=4)
      {
        EXPECT_EQ( (int) ans[0], (int) vals[0] );
        EXPECT_EQ( (int) ans[1], (int) vals[1] );
        EXPECT_EQ( (int) ans[2], (int) vals[2] );
        EXPECT_EQ( (int) ans[3], (int) vals[3] );

        ans[0] = ans[1] = ans[2] = ans[3] = 0.0f;
        ans[ (int) (i/4) ] = 1.0f;

        // if(i%4==0) printf("\n   ans[%02d]:  %f  %f  %f  %f  ", i,  ans[0  ],  ans[1  ],  ans[2  ],  ans[3  ]);
        // if(i%4==0) printf("\n  vals[%02d]:  %f  %f  %f  %f  ", i, vals[0+i], vals[1+i], vals[2+i], vals[3+i]);

        vals+=4;
      }
    }

    void pxMatrix4TisIdentityTest()
    {
      pxMatrix4f m;

      m.identity();

      EXPECT_TRUE( m.isIdentity() );

      m.translate(10.0f, 5.0f);

      EXPECT_FALSE( m.isIdentity() );
    }

    void pxMatrix4TcopyTest()
    {
      pxMatrix4f m1;

      m1.translate(10,5);

      pxMatrix4f m2(m1);

      const float *vals1 = m1.data();
      const float *vals2 = m2.data();

      for(int i = 0; i < 16; i++)
      {
        EXPECT_NEAR( vals1[i], vals2[i], M_ERR );
      }
    }

    void pxMatrix4TtranslateTest()
    {
      pxMatrix4f m;

      m.identity();
      m.translate(10.0f, 5.0f);

      EXPECT_TRUE( m.isTranslatedOnly() );

      EXPECT_EQ(  m.translateX(), 10.0f );
      EXPECT_EQ(  m.translateY(),  5.0f );

      EXPECT_EQ(  m.constData(12), 10.0f );
      EXPECT_EQ(  m.constData(13),  5.0f );

      m.translate(10.0f, 5.0f, 0.0);

      EXPECT_EQ(  m.translateX(), 20.0f );
      EXPECT_EQ(  m.translateY(), 10.0f );
    }


    void pxMatrix4Tscale2dTest()
    {
      pxMatrix4f m;

      m.identity();
      m.scale(0.5f, 0.5f);

      float *vals = m.data();

      EXPECT_EQ(  0.5f, vals[0]  );
      EXPECT_EQ(  0.5f, vals[5]  );
    }

    void pxMatrix4Tscale3dTest()
    {
      pxMatrix4f m;

      m.identity();
      m.scale(0.5f, 0.5f, 0.5f);

      float *vals = m.data();

      EXPECT_EQ( 0.5f, vals[0]  );
      EXPECT_EQ( 0.5f, vals[5]  );
      EXPECT_EQ( 0.5f, vals[10] );
    }

    void pxMatrix4TmultiplyTest()
    {
      pxMatrix4f m;
      pxVector4f v(1,2,3,4);

      m.identity();

      pxVector4f ans = m.multiply(v);

      EXPECT_EQ(  ans.mX, 1.0f );
      EXPECT_EQ(  ans.mY, 2.0f );
      EXPECT_EQ(  ans.mZ, 3.0f );
    }


    void pxMatrix4Trotate1Test()
    {
      pxMatrix4f m;

      m.identity();

      m.rotateInDegrees(45);

      float *vals = m.data();
      float ans45 = 0.70710676908493042;

      EXPECT_NEAR( ans45,  vals[0], M_ERR );
      EXPECT_NEAR( ans45,  vals[1], M_ERR );
      EXPECT_NEAR( ans45, -vals[4], M_ERR );
      EXPECT_NEAR( ans45,  vals[5], M_ERR );
    }

    void pxMatrix4Trotate2Test()
    {
      pxMatrix4f m;

      m.identity();

      m.rotateInRadians(M_PI/4);

      float ans45 = 0.70710676908493042;

      EXPECT_NEAR( ans45,  m.constData(0), M_ERR );
      EXPECT_NEAR( ans45,  m.constData(1), M_ERR );
      EXPECT_NEAR( ans45, -m.constData(4), M_ERR );
      EXPECT_NEAR( ans45,  m.constData(5), M_ERR );
    }

    void pxMatrix4TtransposeTest()
    {
      pxMatrix4f m;

      m.identity();

      m.translate(10.0f, 5.0f, 2.0f);

      EXPECT_EQ(  m.constData(12), 10.0f );
      EXPECT_EQ(  m.constData(13),  5.0f );
      EXPECT_EQ(  m.constData(14),  2.0f );

      m.transpose();

      EXPECT_EQ(  m.constData(3),  10.0f );
      EXPECT_EQ(  m.constData(7),   5.0f );
      EXPECT_EQ(  m.constData(11),  2.0f );
    }


    void pxMatrix4TinvertTest()
    {
      pxMatrix4f m;

      m.identity();
      m.translate(10.0f, 5.0f, 2.0f);

      pxMatrix4f inv(m);
      inv.invert();

      // Identity = Matrix * Inverse;
      m.multiply(inv);

      EXPECT_TRUE(  m.isIdentity() );
    }
};


TEST_F(pxMatrix4Test, pxMatrix4CompleteTest)
{
    pxMatrix4TSinCosTestD();
    pxMatrix4TSinCosTestF();
    pxMatrix4TSinCosTestD();
    pxMatrix4TVector4Test();
    pxMatrix4TSinCosTestD();
    pxMatrix4TidentityTest();
    pxMatrix4TisIdentityTest();
    pxMatrix4TcopyTest();
    pxMatrix4TtranslateTest();
    pxMatrix4Tscale2dTest();
    pxMatrix4Tscale3dTest();
    pxMatrix4TmultiplyTest();
    pxMatrix4Trotate1Test();
    pxMatrix4Trotate2Test();
    pxMatrix4TtransposeTest();
    pxMatrix4TinvertTest();
}

