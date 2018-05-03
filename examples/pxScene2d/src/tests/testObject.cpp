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

// rtCore

#include "testObject.h"

#include "rtCore.h"
#include "rtValue.h"
#include "rtObjectMacros.h"
#include "rtObject.h"
#include "rtTest.h"

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

rtValue v;
rtObjectRef o = new testObj;

void testObject() 
{
  printf("testObject\n");
  rtError e;
  printf("pretest\n");
  printf("test\n"); 
  o.send("init");
  o.send("noArgNoReturn");

  {
    int8_t i8;
    e = o.sendReturns<int8_t>("int8ReturnInt8", (int8_t)127, i8);
    RT_TEST((i8 == 127) && e == RT_OK);
    uint8_t u8;
    e = o.sendReturns<uint8_t>("uint8ReturnUInt8", (uint8_t)255, u8);
    RT_TEST((u8 == 255) && e == RT_OK);
    int32_t i32;
    e = o.sendReturns<int32_t>("int32ReturnInt32", (int32_t)2000000, i32);
    RT_TEST((i32 == 2000000) && e == RT_OK);
    uint32_t u32;
    e = o.sendReturns<uint32_t>("uint32ReturnUInt32", (uint32_t)2000000, u32);
    RT_TEST((i32 == 2000000) && e == RT_OK);

    double d;
    e = o.sendReturns<double>("doubleReturnDouble", 3.14, d);
    RT_TEST((d == 3.14) && e == RT_OK);
    rtString s;
    e = o.sendReturns<rtString>("stringReturnString", "hello", s);

    RT_TEST((s == "hello") && e == RT_OK);
#if 0
    e = o.sendReturns<rtString>("uint32ReturnUInt32", "hello", s);
    RT_TEST((s == "hello") && e == RT_OK);
#endif

    rtValue tst;

    tst = o;

    printf("Before objectReturnObject\n");
    rtObjectRef o2;
    e = o.sendReturns<rtObjectRef>("objectReturnObject", o, o2);
    //   RT_TEST(o2 && e == RT_OK);
#if 0
    rtObjectRef o3;
    o2.sendReturns<rtObjectRef>("objectReturnObject", o2, o3);
    RT_TEST((o2==o3) && e == RT_OK);
#endif
  }

  bool b;
  e = o.sendReturns<bool>("doubleReturnBool", 1.0, b);
  RT_TEST(b && e == RT_OK);
  e = o.sendReturns<bool>("doubleReturnBool", 0.0, b);
  RT_TEST(!b && e == RT_OK);

  rtString s;
  e = o.sendReturns<rtString>("doubleReturnString", 1.0, s);
  RT_TEST((s == "true") && e == RT_OK);
  e = o.sendReturns<rtString>("doubleReturnString", 0.0, s);
  RT_TEST((s == "false") && e == RT_OK);

  printf("here2\n");
#if 0
  {
    rtFunctionRef vf;
    e = o.sendReturns<rtFunctionRef>("valueForKey", "doubleReturnString", vf);
    RT_TEST(e == RT_OK);
    e = vf.sendReturns<rtString>(0.0, s);
    RT_TEST((s == "false") && e == RT_OK);
    e = vf.sendReturns<rtString>(1.0, s);
    RT_TEST((s == "true") && e == RT_OK);
  }
#endif
  printf("here3\n");

  {
    // Invoke using function object
    rtFunctionRef vf;
    e = o.get<rtFunctionRef>("doubleReturnString", vf);
    RT_TEST(e == RT_OK);
    e = vf.sendReturns<rtString>(0.0, s);
    RT_TEST((s == "false") && e == RT_OK);
    e = vf.sendReturns<rtString>(1.0, s);
    RT_TEST((s == "true") && e == RT_OK);
  }
  printf("here\n");
  {
    // Invoke using convenience methods
    e = o.sendReturns<rtString>("doubleReturnString",0.0, s);
    RT_TEST((s == "false") && e == RT_OK);
    e = o.sendReturns<rtString>("doubleReturnString",1.0, s);
    RT_TEST((s == "true") && e == RT_OK);
  }

}
