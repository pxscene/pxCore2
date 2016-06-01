#include <gtest/gtest.h>
#include <stdio.h>
#include "../../rtDefs.h"
#include "../../rtError.h"
#include "../../rtValue.h"
#include "../../rtObjectMacros.h"
#include "../../rtObject.h"

class testObj: public rtObject {
public:
  rtDeclareObject(testObj, rtObject);
  rtMethodNoArgAndNoReturn("noArgNoReturn", noArgNoReturn);
  rtMethod1ArgAndNoReturn("boolArgNoReturn", boolArgNoReturn, bool);
  rtMethod1ArgAndNoReturn("intArgNoReturn", intArgNoReturn, int);
  rtMethod1ArgAndReturn("int8ReturnInt8", int8ReturnInt8, int8_t, int8_t);
  rtMethod1ArgAndReturn("uint8ReturnUInt8", uint8ReturnUInt8, uint8_t, uint8_t);
  rtMethod1ArgAndReturn("int32ReturnInt32", int32ReturnInt32, int32_t, int32_t);
  rtMethod1ArgAndReturn("uint32ReturnUInt32", uint32ReturnUInt32, uint32_t, uint32_t);
  rtMethod1ArgAndReturn("doubleReturnDouble", doubleReturnDouble, double, double);
  rtMethod1ArgAndReturn("stringReturnString", stringReturnString, rtString, rtString);
  rtMethod1ArgAndReturn("objectReturnObject", objectReturnObject, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndNoReturn("doubleArgNoReturn", doubleArgNoReturn, double);
  rtMethod1ArgAndReturn("doubleReturnBool", doubleReturnBool, double, bool);
  rtMethod1ArgAndReturn("doubleReturnString", doubleReturnString, double, rtString);

  rtError noArgNoReturn() {
    printf("In noArgNoReturn()\n");
    return RT_OK;
  }

  rtError boolArgNoReturn(bool /*a1*/) {
    printf("In boolArgNoReturn()\n");
    return RT_OK;
  }

  rtError intArgNoReturn(int /*a1*/) {
    printf("In intArgNoReturn()\n");
    return RT_OK;
  }

  rtError int8ReturnInt8(int8_t a1, int8_t& r) {
    r = a1;
    return RT_OK;
  }

  rtError uint8ReturnUInt8(uint8_t a1, uint8_t& r) {
    r = a1;
    return RT_OK;
  }

  rtError int32ReturnInt32(int32_t a1, int32_t& r) {
    r = a1;
    return RT_OK;
  }

  rtError uint32ReturnUInt32(uint32_t a1, uint32_t& r) {
    r = a1;
    return RT_OK;
  }

  rtError doubleReturnDouble(double a1, double& r) {
    r = a1;
    return RT_OK;
  }

  rtError stringReturnString(rtString a1, rtString& r) {
    r = a1;
    return RT_OK;
  }

  // JRXXX todo be able to take non-const ref
  rtError objectReturnObject(const rtObjectRef& a1, rtObjectRef& r) {
    printf("In objectReturnObject\n");
    r = a1;
    return RT_OK;
  }

  rtError doubleArgNoReturn(double /*a1*/) {
    printf("In doubleArgNoReturn()\n");
    
    return RT_OK;
  }
  
  rtError doubleReturnBool(double a1, bool& r) {
    printf("In doubleReturnBool %f\n", a1);
    r = (a1 == 0.0)?false:true;
    return RT_OK;
  }

  rtError doubleReturnString(double a1, rtString& r) {
    printf("In doubleReturnString %f\n", a1);
    r = (a1 == 0.0)?"false":"true";    
    printf("%s\n", r.cString());
    return RT_OK;
  }
};

rtDefineObject(testObj, rtObject);
rtDefineMethod(testObj, noArgNoReturn);
rtDefineMethod(testObj, boolArgNoReturn);
rtDefineMethod(testObj, intArgNoReturn);
rtDefineMethod(testObj, int8ReturnInt8);
rtDefineMethod(testObj, uint8ReturnUInt8);
rtDefineMethod(testObj, int32ReturnInt32);
rtDefineMethod(testObj, uint32ReturnUInt32);
rtDefineMethod(testObj, doubleReturnDouble);
rtDefineMethod(testObj, stringReturnString);
rtDefineMethod(testObj, objectReturnObject);
rtDefineMethod(testObj, doubleArgNoReturn);
rtDefineMethod(testObj, doubleReturnBool);
rtDefineMethod(testObj, doubleReturnString);


class TestObjectClassTest : public ::testing::Test
{
protected:
virtual void SetUp() {
}

virtual void TearDown() {
}
};

TEST(TestObjectClassTest,TestObject)
{
   rtObjectRef Obj(new testObj());

   EXPECT_EQ(RT_OK,Obj.send("noArgNoReturn"));

   bool b = false;
   EXPECT_EQ(RT_OK,Obj.send("boolArgNoReturn"));

   EXPECT_EQ(RT_OK,Obj.send("intArgNoReturn"));

   int8_t i8;
   EXPECT_EQ(RT_OK,Obj.sendReturns<int8_t>("int8ReturnInt8",INT8_MAX,i8));
   EXPECT_EQ(INT8_MAX,i8);

   uint8_t u8;
   EXPECT_EQ(RT_OK,Obj.sendReturns<uint8_t>("uint8ReturnUInt8",UINT8_MAX,u8));
   EXPECT_EQ(UINT8_MAX,u8);

   int32_t i32;
   EXPECT_EQ(RT_OK,Obj.sendReturns<int32_t>("int32ReturnInt32",INT32_MAX,i32));
   EXPECT_EQ(INT32_MAX,i32);

   uint32_t u32;
   EXPECT_EQ(RT_OK,Obj.sendReturns<uint32_t>("uint32ReturnUInt32",UINT32_MAX,u32));
   EXPECT_EQ(UINT32_MAX,u32);

   double dVal;
   EXPECT_EQ(RT_OK,Obj.sendReturns<double>("doubleReturnDouble",3.14,dVal));
   EXPECT_EQ(3.14,dVal);

   rtString str;
   EXPECT_EQ(RT_OK,Obj.sendReturns<rtString>("stringReturnString","hello",str));
   EXPECT_STREQ(str.cString(),"hello");

   rtObjectRef valObj;
   valObj = Obj;
   rtObjectRef rtObj;
   EXPECT_EQ(RT_OK,Obj.sendReturns<rtObjectRef>("objectReturnObject",valObj,rtObj));
   EXPECT_EQ(valObj,rtObj);

   dVal = 2.1;
   EXPECT_EQ(RT_OK,Obj.send("doubleArgNoReturn",dVal));
   EXPECT_EQ(UINT32_MAX,u32);

   bool bVal = 0.0;
   EXPECT_EQ(RT_OK,Obj.sendReturns<bool>("doubleReturnBool",dVal,bVal));
   EXPECT_TRUE(bVal);

   EXPECT_EQ(RT_OK,Obj.sendReturns<rtString>("doubleReturnString",dVal,str));
   EXPECT_TRUE(str);

}

int main(int argc,char **argv)
{
   ::testing::InitGoogleTest(&argc,argv);
return RUN_ALL_TESTS();
}

