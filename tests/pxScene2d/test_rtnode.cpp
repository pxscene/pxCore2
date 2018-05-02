#include "rtScript.h"
#include "pxTimer.h"

#include "test_includes.h" // Needs to be included last


bool rtNodeInit()
{
    return true;
}

TEST(pxScene2dTests, rtNodeTests)
{
    // Create rtNode
    extern rtScript script;

    //v8::Isolate *isolate = script.getIsolate();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test as have isolate + init
    //EXPECT_TRUE ( isolate      != NULL);
    EXPECT_TRUE ( rtNodeInit() == true);

    double s1 = pxMilliseconds();

    // Create rtNodeContextRef
    rtScriptContextRef ctx;
    script.createContext("javascript", ctx);

    double e1 = pxMilliseconds();

    EXPECT_TRUE( ctx.getPtr()      != NULL ); // Should exist
    //EXPECT_TRUE( ctx->getIsolate() != NULL ); // Should exist

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Create test rtString and add() to JS ctx
    
    rtString myString("Foo String");
    rtError addErr = ctx->add("Foo", myString);

    EXPECT_TRUE( addErr == RT_OK);  // Should fail !

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that add() fails when expected
    {
        rtError err1 = ctx->add(NULL, rtValue());
        EXPECT_TRUE( err1 == RT_FAIL);  // Should fail !

        rtError err2 = ctx->add("EmptyFoo", rtValue());
        EXPECT_TRUE( err2 == RT_FAIL);  // Should fail !
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that has() works
    
    EXPECT_TRUE( ctx->has(NULL)  == false);              // Should not exist
    EXPECT_TRUE( ctx->has("Bar") == false);              // Should not exist
    EXPECT_TRUE( ctx->has("Foo") == true);               // Should exist !
    //EXPECT_TRUE( ctx->has(std::string("Foo")) == true);  // Should exist !

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that get() works and behaves with bad params
    {
        rtValue noFoo = ctx->get("NoFoo");
        EXPECT_TRUE( noFoo.isEmpty() == true);
    }
    #if 0
    {
        rtValue noFoo = ctx->get( std::string("NoFoo") );
        EXPECT_TRUE( noFoo.isEmpty() == true);
    }
    #endif
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that get() works and behaves with bad params
    {
        rtValue foo = ctx->get("Foo");

        EXPECT_TRUE( foo.isEmpty() == false);
        EXPECT_TRUE( strcmp(foo.toString().cString(), "Foo String") == 0);
    }    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that get() works  and behaves with bad params
    {
        rtValue foo = ctx->get(NULL);

        EXPECT_TRUE( foo.isEmpty() == true);
    }
    #if 0
    {
        rtValue foo = ctx->get( std::string("") );

        EXPECT_TRUE( foo.isEmpty() == true);
    }    
    #endif
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that get() works and can get obj from JS
    #if 0
    {
        rtValue foo = ctx->get( std::string("Foo") );

        EXPECT_TRUE( foo.isEmpty() == false);
        EXPECT_TRUE( strcmp(foo.toString().cString(), "Foo String") == 0);
    }
    #endif
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that runScript() works and can get result JS
    {
        rtValue val;
        rtError err = ctx->runScript("3 + 1", &val);

//    printf("\n val \"3 + 1\" = [%s]  \n\n", val.toString().cString());

        EXPECT_TRUE( err == RT_OK );
        EXPECT_TRUE( strcmp(val.toString().cString(), "4") == 0);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that runScript() works and detects param errors
    {
        rtValue val;
        rtError err = ctx->runScript(NULL, &val);

        EXPECT_TRUE( err == RT_FAIL );
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that runScript() works and detects param errors
    #if 0
    {
        rtValue val;
        rtError err = ctx->runScript(std::string(""), &val);

        EXPECT_TRUE( err == RT_FAIL );
    }
    #endif
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that runFile() works and detects i/o errors
    {
        rtValue val;
        rtError err = ctx->runFile("supportfiles/nothere.js", &val);

        EXPECT_TRUE( err == RT_FAIL );
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that runFile() works and detects NULL errors
    {
        rtValue val;
        rtError err = ctx->runFile(NULL, &val);

        EXPECT_TRUE( err == RT_FAIL );
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test that runFile() works and can get result JS
    {
        rtValue val;
        rtError err = ctx->runFile("supportfiles/simple.js", &val);

        EXPECT_TRUE( err == RT_OK );
        EXPECT_TRUE( strcmp(val.toString().cString(), "Hello World") == 0);
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    #if 0
    {
        EXPECT_TRUE( ctx->getIsolate()   != NULL); // Should be Non-NULL
        EXPECT_TRUE( ctx->getContextId() != 0);    // Should be Non-Zero
    }
    #endif
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Test Clone create performance.
    double total = 0.0;
    double count = 1000;

    for(int i = 0; i< count; i++)
    {
        double s = pxMilliseconds();

        // Create rtNodeContextRef - CLONE
        rtScriptContextRef ctx1;
        script.createContext("javascript", ctx1);
        EXPECT_TRUE(ctx1 != NULL);
  
        double e = pxMilliseconds();

        total += (e - s);
    }

    double t1 = (e1 - s1);      // REFERENCE  CONTEXT - create time
    double t2 = total / count;  // CONTEXTIFY CONTEXT - create time (avg)

    // printf("\n createContext() -  Non-Clone = %f ms", t1 );
    // printf("\n createContext() - With-Clone = %f ms\n\n", t2 );

    // Clone create (t2) should be significantly (4 x)  faster than Reference create.
    EXPECT_TRUE((t2 < (t1 / 4)) == true);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}
