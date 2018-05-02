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

#include "pxWayland.h"

#include "test_includes.h" // Needs to be included last

using namespace std;

class pxWaylandTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
       mWayland= new pxWayland();
    }
    
    virtual void TearDown()
    {
       if ( mWayland )
       {
          delete mWayland;
          mWayland= NULL;
       }
    }

    void initialNameTest()
    {
      rtError err;
      rtString s;
      
      err= mWayland->displayName( s );
      EXPECT_TRUE( err == RT_OK );
      EXPECT_STREQ( "", (const char*)s );
    }
    
    void setNameTest()
    {
      rtError err;
      rtString s;

      err= mWayland->setDisplayName( "foo0" );
      EXPECT_TRUE( err == RT_OK );
            
      err= mWayland->displayName( s );
      EXPECT_TRUE( err == RT_OK );
      EXPECT_STREQ( "foo0", (const char*)s );
    }
    
    void autoNameTest()
    {
      rtError err;
      rtString s;
      
      mWayland->onSize(100,100);
      mWayland->onInit();
      err= mWayland->displayName( s );
      EXPECT_TRUE( err == RT_OK );
      EXPECT_TRUE( !s.isEmpty() );
      printf("name=%s\n", (const char*)s );
    }

    void initialCmdTest()
    {
      rtError err;
      rtString s;
      
      err= mWayland->cmd( s );
      EXPECT_TRUE( err == RT_OK );
      EXPECT_STREQ( "", (const char*)s );
    }
    
    void setCmdTest()
    {
      rtError err;
      rtString s;

      err= mWayland->setCmd( "rmfPlayer" );
      EXPECT_TRUE( err == RT_OK );
            
      err= mWayland->cmd( s );
      EXPECT_TRUE( err == RT_OK );
      EXPECT_STREQ( "rmfPlayer", (const char*)s );
    }

    void initialHasAPITest()
    {
      rtError err;
      bool hasAPI;
      
      err= mWayland->hasApi( hasAPI );
      EXPECT_TRUE( err == RT_OK );
      EXPECT_TRUE( false == hasAPI );
    }
    
private:
  pxWayland *mWayland;
};

TEST_F( pxWaylandTest, nameTests )
{
   initialNameTest();
   autoNameTest();
   setNameTest();
}

TEST_F( pxWaylandTest, cmdTests )
{
   initialCmdTest();
   setCmdTest();
}

TEST_F( pxWaylandTest, apiTests )
{
   initialHasAPITest();
}

