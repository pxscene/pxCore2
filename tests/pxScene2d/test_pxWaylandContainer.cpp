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

