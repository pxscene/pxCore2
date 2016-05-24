/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include<gtest/gtest.h>
#include "../rtRemote.h"
#include "rtTestCommon.h"
#include <limits.h>

static char const* objectName = "com.xfinity.xsmart.SimpleServer/Comcast";
class RemoteSettingsTest : public ::testing::Test {
   protected:
     virtual void SetUp() {
}
    virtual void TearDown() {
}
    
};

TEST(RemoteSettingsTest,RTRemoteRegisterObjectTest) 
{
  
  EXPECT_EQ(RT_OK,rtRemoteInit());
  rtObjectRef serverObj(new rtThermostat());  
  rtObjectRef obj(new rtLcd());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",150);
  EXPECT_EQ(RT_OK,rtRemoteRegisterObject(objectName, serverObj));
  
  serverObj.set("lcd",obj);
}

TEST(RemoteSettingsTest,UINTTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  uint64_t i = 123;
 
  rtObjectRef objectRef;
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",150);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName,serverObj));

  serverObj.set("lcd",obj);
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));
 
  rtObjectRef lcd;
  objectRef.get("lcd",lcd);
  EXPECT_EQ(150,lcd.get<int>("width")); 
  lcd.set("height",i);
  //UINT64
  EXPECT_EQ(123,lcd.get<int>("height"));
    
  printf("height:%lu\n",lcd.get<uint64_t>("height"));
  EXPECT_EQ(static_cast<uint64_t>(i),lcd.get<uint64_t>("height"));
  
  //UINT32
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  
  printf("height:%u\n",lcd.get<uint32_t>("height"));
  EXPECT_EQ(static_cast<uint32_t>(i), lcd.get<uint32_t>("height"));

  //UINT16
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  //printf("height:%d\n", objectRef.get<uint16_t>("height"));
  //EXPECT_EQ(static_cast<uint16_t>("height"), objectRef.get<uint16_t>("height"));
  
  //UINT8
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
 
  printf("height:%d\n", objectRef.get<uint8_t>("height"));
  //EXPECT_EQ(static_cast<uint8_t>(i), objectRef.get<uint8_t>("height"));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,UINTMaxValueTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  uint64_t i = UINT64_MAX;
  rtObjectRef objectRef;

  rtObjectRef lcd;  
  rtObjectRef obj(new rtLcd());    
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("height",150);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));
  
  serverObj.set("lcd",obj);
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));
 
  objectRef.get("lcd",lcd);
  //UINT64
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    

  rtString str = lcd.get<rtString>("text");
  printf("text:%s\n", str.cString());
  printf("height:%lu\n", lcd.get<uint64_t>("height"));
  //EXPECT_EQ(static_cast<uint64_t>(i),lcd.get<uint64_t>("height"));
  
  
  //UINT32
  i = UINT32_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  printf("height:%u\n",lcd.get<uint32_t>("height"));
  EXPECT_EQ(static_cast<uint32_t>(i),lcd.get<uint32_t>("height"));

  //UINT16
  i = UINT16_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  
  //UINT8
  i = UINT8_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,UINTMinValueTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  int i = 0;
  rtObjectRef objectRef;
  
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",150);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));

  //UINT64
  EXPECT_EQ(RT_OK,objectRef.set("height", i));
    
  
  printf("height:%lu\n", objectRef.get<uint64_t>("height"));
  //EXPECT_EQ(static_cast<uint64_t>(i), objectRef.get<uint64_t>("height"));
  
  //UINT32
  EXPECT_EQ(RT_OK,objectRef.set("height", i));
    
  
  printf("height:%d\n", objectRef.get<uint32_t>("height"));
  //EXPECT_EQ(static_cast<uint32_t>(i), objectRef.get<uint32_t>("height"));

  //UINT16
  EXPECT_EQ(RT_OK,objectRef.set("height", i));
    
  
  //printf("height:%d\n", objectRef.get<uint16_t>("height"));
  //EXPECT_EQ(static_cast<uint16_t>(i), objectRef.get<uint16_t>("height"));
  
  //UINT8
  EXPECT_EQ(RT_OK,objectRef.set("height", i));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,UINTInvalidDataTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  int i = -4;
  rtObjectRef objectRef;
  
  rtObjectRef lcd;
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",150);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));
  
  serverObj.set("lcd",obj);
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));

  objectRef.get("lcd",lcd);
  //UINT64
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  printf("height:%lu\n", lcd.get<uint64_t>("height"));
  //EXPECT_EQ(static_cast<uint64_t>(i),lcd.get<uint64_t>("height"));
  
  //UINT32
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  
  printf("height:%d\n",lcd.get<uint32_t>("height"));
  //EXPECT_EQ(static_cast<uint32_t>(i),lcd.get<uint32_t>("height"));

  //UINT16
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  
  //printf("height:%d\n",lcd.get<uint16_t>("height"));
  //EXPECT_EQ(static_cast<uint16_t>(i),lcd.get<uint16_t>("height"));
  
  //UINT8
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  //EXPECT_EQ(static_cast<uint8_t>(i),lcd.get<uint8_t>("height"));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,INTTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  int i = 1043;
  rtObjectRef objectRef;
  
  rtObjectRef lcd;
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",150);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));

  serverObj.set("lcd",obj); 
   EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));

  objectRef.get("lcd",lcd);
  //INT64
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  printf("height:%ld\n",lcd.get<int64_t>("height"));
  EXPECT_EQ(static_cast<int64_t>(i),lcd.get<int64_t>("height"));
  
  //INT32
  EXPECT_EQ(RT_OK,objectRef.set("height", i));
    
  
  printf("height:%d\n",lcd.get<int32_t>("height"));
  EXPECT_EQ(static_cast<int32_t>(i),lcd.get<int32_t>("height"));

  //INT16
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  
  //EXPECT_EQ(static_cast<int16_t>("height"),lcd.get<int16_t>("height"));
  
  //INT8
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  //EXPECT_EQ(static_cast<int8_t>(i),lcd.get<int8_t>("height"));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,INTMaxValueTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  int64_t i = INT64_MAX;
  rtObjectRef objectRef;
 
  rtObjectRef lcd; 
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",50);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));
  
  serverObj.set("lcd",obj);
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));

  objectRef.get("lcd",lcd);  
//INT64
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  printf("height:%ld\n", objectRef.get<int64_t>("height"));
  //EXPECT_EQ(static_cast<int64_t>(i), objectRef.get<int64_t>("height"));
  
  //INT32
  i = INT32_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  //printf("height:%d\n", objectRef.get<int32_t>("height"));
 // EXPECT_EQ(static_cast<int32_t>(i), objectRef.get<uint32_t>("height"));

  //UINT16
  i = INT16_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
 
  //printf("height:%d\n",lcd.get<int16_t>("height"));
  //EXPECT_EQ(static_cast<int16_t>(i),lcd.get<int16_t>("height"));
  
  
  //INT8
  i = INT8_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", INT8_MAX));
    
  //EXPECT_EQ(static_cast<int8_t>(i),lcd.get<int8_t>("height"));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,INTMinValueTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  int64_t i = INT64_MIN;
  rtObjectRef objectRef;
  
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",150);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));
  
  serverObj.set("lcd",obj);
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));
 
  rtObjectRef lcd;
  objectRef.get("lcd",lcd);
  //INT64
  EXPECT_EQ(RT_OK,objectRef.set("height", i));
  
  printf("height:%ld\n",lcd.get<int64_t>("height"));
  //EXPECT_EQ(static_cast<int64_t>(i),lcd.get<int64_t>("height"));
  
  //INT32
  i = INT32_MIN;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  //printf("height:%d\n",lcd.get<int32_t>("height"));
  //EXPECT_EQ(static_cast<int32_t>(i),lcd.get<int32_t>("height"));

  //UINT16
  i = INT16_MIN;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
  
  //printf("height:%d\n",lcd.get<int16_t>("height"));
  //EXPECT_EQ(static_cast<int16_t>(i),lcd.get<int16_t>("height"));
  
  //INT8
  i = INT8_MIN;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  printf("height:%d\n",lcd.get<int8_t>("height"));
  //EXPECT_EQ(static_cast<int8_t>(i),lcd.get<int8_t>("height"));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,INTInvalidDataTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  uint32_t i = UINT32_MAX;
  rtObjectRef objectRef;
 
  rtObjectRef lcd; 
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("height",10);
  obj.set("width",150);
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));
  
  serverObj.set("lcd",obj);
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));

  rtValue bVal = true;
  objectRef.get("lcd",lcd);
  //INT64
  EXPECT_EQ(RT_OK,lcd.set("height",bVal));
    
  printf("height:%d\n", lcd.get<bool>("height"));
  //EXPECT_EQ(i,lcd.get<uint32_t>("height"));
 
  //INT32
  //i = INT32_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  printf("height:%d\n", lcd.get<int32_t>("height"));
  EXPECT_EQ(static_cast<int32_t>(i), lcd.get<int32_t>("height"));

  //INT16
  //i = INT16_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  //int16_t n = lcd.get<int16_t>("height");
  //printf("height:%d\n",lcd.get<int16_t>("height"));
  //EXPECT_EQ(static_cast<int16_t>(i), lcd.get<int16_t>("height"));
  
  //INT8
  //i = INT8_MAX;
  EXPECT_EQ(RT_OK,lcd.set("height", i));
    
  
  //printf("height:%d\n",lcd.get<int8_t>("height"));
  //EXPECT_EQ(static_cast<int8_t>(i),lcd.get<int8_t>("height"));
  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,rtLCDStringTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  rtObjectRef objectRef;
  
  rtObjectRef serverObj(new rtLcd()); // allocate an rt object
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));

  rtString s = "Hello";
  serverObj.set("text","Hello");
  
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));
  //EXPECT_EQ(s,objectRef.get<rtString>("text"));
  
  s = objectRef.get<rtString>("text");
  printf("text:: %s\n",s.cString());  
  rtRemoteShutdown();
}

TEST(RemoteSettingsTest,rtLCDPropertyTest) 
{
  EXPECT_EQ(RT_OK,rtRemoteInit());
  
  rtObjectRef obj(new rtLcd());    // allocate an rt object
  rtObjectRef serverObj(new rtThermostat());   
  obj.set("text","Remote LCD - Thermostat");
  obj.set("width",10);
  obj.set("height",150);
  rtObjectRef objectRef;
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj));
  serverObj.set("lcd",obj); 
  
  EXPECT_EQ(RT_OK,rtRemoteLocateObject(objectName, objectRef));
  rtObjectRef lcd;
  objectRef.get("lcd",lcd);

  lcd.set("width",123);
  lcd.set("height",10);
  EXPECT_EQ(123,lcd.get<int>("width"));
  lcd.set("height",11);
  EXPECT_EQ(11,lcd.get<int>("height"));
    
  rtRemoteShutdown();
}

int main(int argc,char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


