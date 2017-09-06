#include "pxScene2d.h"
#include "pxWaylandContainer.h"

#include "test_includes.h" // Needs to be included last

using namespace std;

class pxWaylandContainerTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
       mScene = new pxScene2d(false);
       mWaylandContainer= new pxWaylandContainer(mScene);
       
       pxWayland *wayland= new pxWayland();
       if ( wayland )
       {
         mWaylandContainer->setView( wayland );
         wayland->onSize(100,100);
       }
    }
    
    virtual void TearDown()
    {
       if ( mWaylandContainer )
       {
          mWaylandContainer->setView(NULL);
          mWaylandContainer= NULL;          
       }
       if ( mScene )
       {
          delete mScene;
          mScene= 0;
       }
    }

    void autoNameTest()
    {
      rtError err;
      rtString s;
      
      mWaylandContainer->onInit();
      err= mWaylandContainer->displayName( s );
      EXPECT_TRUE( err == RT_OK );
      EXPECT_TRUE( !s.isEmpty() );
      printf("name=%s\n", (const char*)s );
    }

    
private:
  pxScene2d *mScene;
  pxWaylandContainer *mWaylandContainer;
};

TEST_F( pxWaylandContainerTest, nameTests )
{
   autoNameTest();
}

