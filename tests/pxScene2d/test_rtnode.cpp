#include "gtest/gtest.h"

#include "rtNode.h"

#include "pxTimer.h"


bool rtNodeInit()
{
    return true;
}

TEST(pxScene2dTests, rtNodeTests)
{
    // Create rtNode
    rtNode mynode;

    v8::Isolate *isolate = mynode.getIsolate();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test as have isolate + init
    EXPECT_TRUE ( isolate      != NULL);
    EXPECT_TRUE ( rtNodeInit() == true);

    double s1 = pxMilliseconds();

    // Create rtNodeContextRef
    rtNodeContextRef ctx = mynode.createContext();

    double e1 = pxMilliseconds();

    EXPECT_TRUE( ctx.getPtr() != NULL ); // Should exist

    // Create test rtString and add() to JS ctx
    rtString myString("Foo");
    ctx->add("Foo", myString);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that has() works
    EXPECT_TRUE( ctx->has("Bar") == false); // Should not exist
    EXPECT_TRUE( ctx->has("Foo") == true);  // Should exist !

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that get() works and can get obj from JS
    rtValue foo = ctx->get("Foo");

    EXPECT_TRUE( foo.isEmpty() == false);
    EXPECT_TRUE( strcmp(foo.toString().cString(), "Foo") == 0);

    // rtObjectRef retObj = ctx->runScript("3 + 1");
    // rtValue     retVal(retObj);

    // printf("\n retVal \"3 + 1\" = %s\n\n", retVal.toString().cString());

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    double total = 0.0;
    double count = 1000;

    for(int i = 0; i< count; i++)
    {
        double s = pxMilliseconds();

        // Create rtNodeContextRef - CLONE
        rtNodeContextRef ctx1 = mynode.createContext();

        double e = pxMilliseconds();

        total += (e - s);
    }

    double t1 = (e1 - s1);
    double t2 = total / count;

    // printf("\n createContext() -  Non-Clone = %f ms", t1 );
    // printf("\n createContext() - With-Clone = %f ms\n\n", t2 );

    // Clone create (t2) should be significantly faster than Reference create.
    EXPECT_TRUE( t2 < (t1 / 4) == true);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

