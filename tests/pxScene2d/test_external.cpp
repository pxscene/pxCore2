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

#include "test_includes.h" // Needs to be included last

using namespace std;

class externalTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }
    
    virtual void TearDown()
    {
    }

    void createExternalDirectTest()
    {
      pxScene2d* scene = new pxScene2d();
      rtObjectRef ref;
      rtObjectRef map = new rtMapObject;
      map.set("t","external");
      rtError err= scene->create(map,ref);
      EXPECT_TRUE( err == RT_OK );
      delete scene;
      map = NULL;
    }
    
    void createExternalWithWaylandTest()
    {
      pxScene2d* scene = new pxScene2d();
      rtObjectRef ref;
      rtObjectRef map = new rtMapObject;
      map.set("t","wayland");
      rtError err= scene->create(map,ref);
      EXPECT_TRUE( err == RT_OK );
      delete scene;
      map = NULL;
    }
};

TEST_F( externalTest, externalTests )
{
  createExternalDirectTest();
  createExternalWithWaylandTest();
}
