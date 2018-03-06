#include <list>
#include <sstream>

#define private public
#define protected public

#include "rtString.h"
#include "pxScene2d.h"
#include "pxImage.h"
#include "pxRectangle.h"
#include <string.h>
#include <sstream>

#include "test_includes.h" // Needs to be included last

class childRemovalTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      mScene = new pxScene2d(false);
      mImage1 = new pxImage(mScene);
      mImage2 = new pxImage(mScene);
      mRect = new pxRectangle(mScene);
      mRect->setParent(mScene->getRoot());
    }

    virtual void TearDown()
    {
    }

    void childRemovalBeforeDisposeTest ()
    {
	 mImage1->setParent(mRect);
	 mImage2->setParent(mRect);
         EXPECT_TRUE (mRect->numChildren() == 2);
	 mImage1->remove();
         EXPECT_TRUE (mRect->numChildren() == 1);
	 mRect->removeAll();
         EXPECT_TRUE (mRect->numChildren() == 0);
    }

    void childRemovalAfterDisposeTest ()
    {
	 mImage1->setParent(mRect);
	 mImage2->setParent(mRect);
         EXPECT_TRUE (mRect->numChildren() == 2);
	 mScene.send("dispose");
	 mImage1->remove();
         EXPECT_TRUE (mRect->numChildren() == 0);
	 mRect->removeAll();
         EXPECT_TRUE (mRect->numChildren() == 0);
    }
  
    private: 
      pxScene2dRef mScene;
      pxObjectRef mImage1;
      pxObjectRef mImage2;
      pxObjectRef mRect;
}  ;

TEST_F(childRemovalTest, childRemovalTest)
{
    childRemovalBeforeDisposeTest();
    childRemovalAfterDisposeTest();
}

