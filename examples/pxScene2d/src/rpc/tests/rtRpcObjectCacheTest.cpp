/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include<gtest/gtest.h>
#include "../rtRemote.h"
#include "rtTestCommon.h"
#include "../rtObjectCache.h"
#include <limits.h>
#include <memory>
#include "../rapidjson/document.h"
#include "../rapidjson/rapidjson.h"
#include "../rtRemoteConfig.h"
#include "../rtRemoteMessage.h"
#include "../rtRemoteTypes.h"

#define kFieldNameMessageType "message.type"
#define kFieldNameCorrelationKey "correlation.key"
#define kFieldNameObjectId "object.id"
#define kFieldNamePropertyName "property.name"
#define kFieldNamePropertyIndex "property.index"
#define kFieldNameStatusCode "status.code"
#define kFieldNameStatusMessage "status.message"
#define kFieldNameFunctionName "function.name"
#define kFieldNameFunctionIndex "function.index"
#define kFieldNameFunctionArgs "function.args"
#define kFieldNameFunctionReturn "function.return_value"
#define kFieldNameValue "value"
#define kFieldNameValueType "type"
#define kFieldNameValueValue "value"
#define kFieldNameSenderId "sender.id"
#define kFieldNameKeepAliveIds "keep_alive.ids"
#define kFieldNameIp "ip"
#define kFieldNamePort "port"

#define kMessageTypeInvalidResponse "invalid.response"
#define kMessageTypeSetByNameRequest "set.byname.request"
#define kMessageTypeSetByNameResponse "set.byname.response"
#define kMessageTypeSetByIndexRequest "set.byindex.request"
#define kMessageTypeSetByIndexResponse "set.byindex.response"
#define kMessageTypeGetByNameRequest "get.byname.request"
#define kMessageTypeGetByNameResponse "get.byname.response"
#define kMessageTypeGetByIndexRequest "get.byindex.request"
#define kMessageTypeGetByIndexResponse "get.byindex.response"
#define kMessageTypeOpenSessionResponse "session.open.response"
#define kMessageTypeMethodCallResponse "method.call.response"
#define kMessageTypeKeepAliveResponse "keep_alive.response"
#define kMessageTypeSearch "search"
#define kMessageTypeLocate "locate"
#define kMessageTypeMethodCallRequest "method.call.request"
#define kMessageTypeKeepAliveRequest "keep_alive.request"
#define kMessageTypeOpenSessionRequest "session.open.request"

//#define kInvalidPropertyIndex std::numeric_limits<uint32_t>::max()
//#define kInvalidCorrelationKey std::numeric_limits<uint32_t>::max()RT_functionType

//using rtJsonDocPtr = std::shared_ptr< rapidjson::Document >;

//char const* objectName = "com.xfinity.xsmart.SimpleServer/Comcast";
static char const* objectName = "com.xfinity.xsmart.SimpleServer/Comcast";
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


class RemoteObjectCacheTest : public ::testing::Test {
   protected:
     virtual void SetUp() {
    rtRemoteInit();
    serverObj = new rtThermostat();
    rtRemoteRegisterObject(objectName,serverObj);
}
    virtual void TearDown() {
   rtRemoteShutdown();
}
private:
 
  rtObjectRef serverObj;
};

TEST(RemoteObjectCacheTest,RTRemoveStaleObjectsTest) 
{
  rtObjectRef obj(new testObj());
  rtObjectRef ref = rtObjectCache::findObject(objectName);
  if (!ref)
  {
    rtObjectCache::insert(objectName,obj, -1);
  }

  EXPECT_EQ(RT_OK,rtObjectCache::removeUnused());
}

TEST(RemoteObjectCacheTest,EraseObjectsTest) 
{
  rtObjectRef obj1(new testObj());
  rtObjectRef obj = rtObjectCache::findObject(objectName);  
  if(!obj)
  {
    EXPECT_EQ(RT_OK,rtObjectCache::insert(objectName,obj1, rtRemoteSetting<int>("rt.rpc.cache.max_object_lifetime")));
    EXPECT_EQ(RT_OK,rtObjectCache::erase(objectName));
  }
}

TEST(RemoteObjectCacheTest,FindObjectTest) 
{
  rtObjectRef obj(new testObj());
  rtObjectRef ref = rtObjectCache::findObject(objectName);
  if (!ref)
  {
    rtObjectCache::insert(objectName,obj, -1);
  }

}


TEST(RemoteObjectCacheTest,OnKeepAliveTest) 
{
  rtObjectRef obj(new testObj());
  rtObjectRef ref = rtObjectCache::findObject(objectName);
  if (!ref)
  {
    rtObjectCache::insert(objectName,obj, -1);
  }
 
 // EXPECT_EQ(RT_OK,ref->sendReturns<rtError>("sendKeepAlive"));

}

TEST(RemoteObjectCacheTest,GetFieldParamsTest)
{
  rtValue value = 15;
  
  rtRemoteSetRequest setReq(objectName,kFieldNamePropertyName);
  EXPECT_EQ(RT_OK,setReq.setValue(value));
 
  rtRemoteGetRequest getReq(objectName,kFieldNamePropertyName);
  rtCorrelationKey seq_id = rtMessage_GetNextCorrelationKey();
  
  printf("seq :: %d",seq_id);
}

TEST(RemoteObjectCacheTest,SetFieldParamsTest)
{
  rtValue value = 19;
  uint32_t timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");
  rtRemoteSetRequest setReq(objectName,kFieldNamePropertyName);
  setReq.setValue(value);
}

int main(int argc,char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
