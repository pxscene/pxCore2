#include "pxCore.h"
#include "pxPixel.h"
#include "test_includes.h" // Needs to be included last

using namespace std;

class pxPixelTest : public testing::Test
{
    public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
    void pxPixelParam0()
    {
        pxPixel p = pxPixel();
        EXPECT_TRUE(sizeof(p) == sizeof(pxPixel));
    }
    void pxPixelParam1()
    {
        uint32_t u = 1;
        pxPixel p = pxPixel(u);
        EXPECT_TRUE(1 == p.u);
    }
    void pxPixelParam1Struct()
    {
        uint32_t u = 2;
        pxPixel p = pxPixel(u);
	EXPECT_TRUE(2 == p.u);
        p = pxPixel((const pxPixel&)p); 
    }
    void pxPixelParam4()
    {
        uint32_t r= 1;
        uint32_t g= 1;
        uint32_t b= 1;
        uint32_t a= 1;
        pxPixel p = pxPixel(r, g, b, a);
        EXPECT_TRUE(1 == p.r);
        EXPECT_TRUE(1 == p.g);
        EXPECT_TRUE(1 == p.b);
        EXPECT_TRUE(1 == p.a);
       	
	p = pxPixel(r, g, b);
        EXPECT_TRUE(1 == p.r);
        EXPECT_TRUE(1 == p.g);
        EXPECT_TRUE(1 == p.b);
        EXPECT_TRUE(255 == p.a);
    }
};
TEST_F(pxPixelTest, pxPixelCompleteTest)
{
    pxPixelParam0();
    pxPixelParam1();
    pxPixelParam1Struct();
    pxPixelParam4();
}
