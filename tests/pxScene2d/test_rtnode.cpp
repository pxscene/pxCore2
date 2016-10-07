#include "gtest/gtest.h"

#include "rtNode.h"

#include "pxContext.h"
#include "pxTimer.h"


rtNode mynode;


bool rtNodeInit()
{
    return true;
}
 
TEST(pxScene2dTests, rtNodeTests)
{ 
    v8::Isolate *isolate = mynode.getIsolate();
   
    EXPECT_TRUE ( isolate      != NULL);  
    EXPECT_TRUE ( rtNodeInit() == true);  
}


