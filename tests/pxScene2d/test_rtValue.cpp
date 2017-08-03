#include <sstream>

#define private public
#define protected public

#include "rtValue.h"
#include <string.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

class rtValueTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
          boolVal = ( true                         );
          int8Val = ( int8_t(8)                    );
         uint8Val = ( uint8_t(88)                  );
         int32Val = ( int32_t(32)                  );
        uint32Val = ( uint32_t(3232)               );
         int64Val = ( int64_t(64)                  );
        uint64Val = ( uint64_t(6464)               );
         floatVal = ( float(3.14)                  );
        doubleVal = ( double(3.142)                );
          charVal = ( (const char*) "cString here" );
        stringVal = ( rtString("rtString here")    );
    }

    virtual void TearDown()
    {
    }
    
    virtual void SetEmpty()
    {
          boolVal.setEmpty();
          int8Val.setEmpty();
         uint8Val.setEmpty();
         int32Val.setEmpty();
        uint32Val.setEmpty();
         int64Val.setEmpty();
        uint64Val.setEmpty();
         floatVal.setEmpty();
        doubleVal.setEmpty();
          charVal.setEmpty();
        stringVal.setEmpty();
        //  objectVal.setEmpty();
        //  objRefVal.setEmpty();
        //    funcVal.setEmpty();
        // funcRefVal.setEmpty();
        //   valueVal.setEmpty();
        voidPtrVal.setEmpty();
    }
    
    virtual void CheckEmpty()
    {
        EXPECT_TRUE(    boolVal.isEmpty() );
        EXPECT_TRUE(    int8Val.isEmpty() );
        EXPECT_TRUE(   uint8Val.isEmpty() );
        EXPECT_TRUE(   int32Val.isEmpty() );
        EXPECT_TRUE(  uint32Val.isEmpty() );
        EXPECT_TRUE(   int64Val.isEmpty() );
        EXPECT_TRUE(  uint64Val.isEmpty() );
        EXPECT_TRUE(   floatVal.isEmpty() );
        EXPECT_TRUE(  doubleVal.isEmpty() );
        EXPECT_TRUE(    charVal.isEmpty() );
        EXPECT_TRUE(  stringVal.isEmpty() );
        // EXPECT_TRUE(  objectVal.isEmpty() );
        // EXPECT_TRUE(  objRefVal.isEmpty() );
        // EXPECT_TRUE(    funcVal.isEmpty() );
        // EXPECT_TRUE( funcRefVal.isEmpty() );
        // EXPECT_TRUE(   valueVal.isEmpty() );
        // EXPECT_TRUE( voidPtrVal.isEmpty() );
    }
 
    virtual void CheckNonEmpty()
    {
        EXPECT_FALSE(   boolVal.isEmpty() );
        EXPECT_FALSE(   int8Val.isEmpty() );
        EXPECT_FALSE(  uint8Val.isEmpty() );
        EXPECT_FALSE(  int32Val.isEmpty() );
        EXPECT_FALSE( uint32Val.isEmpty() );
        EXPECT_FALSE(  int64Val.isEmpty() );
        EXPECT_FALSE( uint64Val.isEmpty() );
        EXPECT_FALSE(  floatVal.isEmpty() );
        EXPECT_FALSE( doubleVal.isEmpty() );
        EXPECT_FALSE(   charVal.isEmpty() );
        EXPECT_FALSE( stringVal.isEmpty() );
        // EXPECT_FALSE(  objectVal.isEmpty() );
        // EXPECT_FALSE(  objRefVal.isEmpty() );
        // EXPECT_FALSE(    funcVal.isEmpty() );
        // EXPECT_FALSE( funcRefVal.isEmpty() );
        // EXPECT_FALSE(   valueVal.isEmpty() );
        // EXPECT_FALSE( voidPtrVal.isEmpty() );
    }
    
    virtual void SetValues()
    {
          boolVal.setBool(   true                         );
          int8Val.setInt8(   int8_t(8)                    );
         uint8Val.setUInt8(  uint8_t(88)                  );
         int32Val.setInt32(  int32_t(32)                  );
        uint32Val.setUInt32( uint32_t(3232)               );
         int64Val.setInt64(  int64_t(64)                  );
        uint64Val.setUInt64( uint64_t(6464)               );
         floatVal.setFloat(  float(3.14)                  );
        doubleVal.setDouble( double(3.142)                );
          charVal.setString( (const char*) "cString here" );
        stringVal.setString( rtString("rtString here")    );
        //  objectVal.setVALUE(const rtIObject* v);
        //  objRefVal.setVALUE(const rtObjectRef& v);
        //    funcVal.setVALUE(const rtIFunction* v);
        // funcRefVal.setVALUE(const rtFunctionRef& v);
        //   valueVal.setVALUE(const rtValue& v);
        voidPtrVal.setVoidPtr( (void *) NULL);
    }
        
    void ctorToTests()
    {
        EXPECT_TRUE(   boolVal.toBool()   == true);
        EXPECT_TRUE(   int8Val.toInt8()   == 8);
        EXPECT_TRUE(  uint8Val.toUInt8()  == 88);
        EXPECT_TRUE(  int32Val.toInt32()  == 32);
        EXPECT_TRUE( uint32Val.toUInt32() == 3232);
        EXPECT_TRUE(  int64Val.toInt64()  == 64);
        EXPECT_TRUE( uint64Val.toUInt64() == 6464);
        EXPECT_TRUE(  floatVal.toFloat()  == 3.14f);
        EXPECT_TRUE( doubleVal.toDouble() == 3.142);
        // EXPECT_TRUE(   charVal.toString() == "cString here")
        // EXPECT_TRUE( stringVal.toString() == "rtString here")
        // EXPECT_TRUE(boolVal.toObject() == true)
        // EXPECT_TRUE(boolVal.toFunction() == true)
        // EXPECT_TRUE(boolVal.toVoidPtr() == true)
    }
    
    void isEmptyTest()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(boolVal.isEmpty() == false);
        boolVal.setEmpty();
        EXPECT_TRUE(boolVal.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(int8Val.isEmpty() == false);
        int8Val.setEmpty();
        EXPECT_TRUE(int8Val.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(uint8Val.isEmpty() == false);
        uint8Val.setEmpty();
        EXPECT_TRUE(uint8Val.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(int32Val.isEmpty() == false);
        int32Val.setEmpty();
        EXPECT_TRUE(int32Val.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(uint32Val.isEmpty() == false);
        uint32Val.setEmpty();
        EXPECT_TRUE(uint32Val.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(int64Val.isEmpty() == false);
        int64Val.setEmpty();
        EXPECT_TRUE(int64Val.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(uint64Val.isEmpty() == false);
        uint64Val.setEmpty();
        EXPECT_TRUE(uint64Val.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(floatVal.isEmpty() == false);
        floatVal.setEmpty();
        EXPECT_TRUE(floatVal.isEmpty() == true);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        EXPECT_TRUE(doubleVal.isEmpty() == false);
        doubleVal.setEmpty();
        EXPECT_TRUE(doubleVal.isEmpty() == true);

        // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // EXPECT_TRUE(charVal.isEmpty() == false);
        // charVal.setEmpty();
        // EXPECT_TRUE(charVal.isEmpty() == true);

        // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // EXPECT_TRUE(stringVal.isEmpty() == false);
        // stringVal.setEmpty();
        // EXPECT_TRUE(stringVal.isEmpty() == true);
        // // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    }

    void getTest()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...
        rtError err = RT_OK;

            bool   bv;     boolVal.getBool(   bv   );   EXPECT_TRUE(   bv == true);
          int8_t  i8v;     int8Val.getInt8(   i8v  );   EXPECT_TRUE(  i8v == 8);
         uint8_t  u8v;    uint8Val.getUInt8(  u8v  );   EXPECT_TRUE(  u8v == 88);
         int32_t i32v;    int32Val.getInt32(  i32v );   EXPECT_TRUE( i32v == 32);
        uint32_t u32v;   uint32Val.getUInt32( u32v );   EXPECT_TRUE( u32v == 3232);
         int64_t i64v;    int64Val.getInt64(  i64v );   EXPECT_TRUE( i64v == 64);
        uint64_t u64v;   uint64Val.getUInt64( u64v );   EXPECT_TRUE( u64v == 6464);
           float   fv;    floatVal.getFloat(  fv   );   EXPECT_TRUE(   fv == 3.14f);
          double   dv;   doubleVal.getDouble( dv   );   EXPECT_TRUE(   dv == 3.142);
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        //  rtString  sv;   stringVal.getString( sv   );   EXPECT_TRUE(   sv == "rtString here");
        // rtValue  objectVal.getVALUE(const rtIObject* v);
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }

    void getBoolTest()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        bool   bv;

          boolVal.getBool(   bv   );   EXPECT_TRUE( bv == true );  // ##

          int8Val.getBool(   bv   );   EXPECT_TRUE( bv == true );
         uint8Val.getBool(   bv   );   EXPECT_TRUE( bv == true );
         int32Val.getBool(   bv   );   EXPECT_TRUE( bv == true );
        uint32Val.getBool(   bv   );   EXPECT_TRUE( bv == true );
         int64Val.getBool(   bv   );   EXPECT_TRUE( bv == true );
        uint64Val.getBool(   bv   );   EXPECT_TRUE( bv == true );
         floatVal.getBool(   bv   );   EXPECT_TRUE( bv == true );
        doubleVal.getBool(   bv   );   EXPECT_TRUE( bv == true );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        stringVal.getBool(   bv   );   EXPECT_TRUE( bv == true );
        // objectVal.getBool(   bv   );   EXPECT_TRUE( bv == bv );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);

        int8Val.setBool( false );
        int8Val.getBool( bv );   EXPECT_TRUE( bv == false );  // ## 
    }

    void getInt8Test()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        int8_t  i8v;

          boolVal.getInt8( i8v );   EXPECT_TRUE( i8v == 1    );
          int8Val.getInt8( i8v );   EXPECT_TRUE( i8v == 8    );  // ##
         uint8Val.getInt8( i8v );   EXPECT_TRUE( i8v == 88   );
         int32Val.getInt8( i8v );   EXPECT_TRUE( i8v == 32   );
        uint32Val.getInt8( i8v );   EXPECT_TRUE( i8v == 0xFFFFFFA0 );  // 3232 truncated to  (int8_t)
         int64Val.getInt8( i8v );   EXPECT_TRUE( i8v == 64   );
        uint64Val.getInt8( i8v );   EXPECT_TRUE( i8v == 0x40 ); // 6464 truncated to  (int8_t)
         floatVal.getInt8( i8v );   EXPECT_TRUE( i8v == 3    );
        doubleVal.getInt8( i8v );   EXPECT_TRUE( i8v == 3    );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        stringVal.getInt8( i8v   ); EXPECT_TRUE( i8v == i8v );
        // objectVal.getInt8( i8v );   EXPECT_TRUE( i8v == i8v  );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);

        int8Val.setInt8( 0 );
        int8Val.getInt8( i8v );   EXPECT_TRUE( i8v == 0    );  // ## 
    }

    void getUInt8Test()
    {
         // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...
        rtError err = RT_OK;

        uint8_t  u8v;
         
          boolVal.getUInt8( u8v );   EXPECT_TRUE( u8v == 1    );
          int8Val.getUInt8( u8v );   EXPECT_TRUE( u8v == 8    );
         uint8Val.getUInt8( u8v );   EXPECT_TRUE( u8v == 88   );  // ##
         int32Val.getUInt8( u8v );   EXPECT_TRUE( u8v == 32   );
        uint32Val.getUInt8( u8v );   EXPECT_TRUE( u8v == 0xA0 );  // 3232 truncated to  (uint8_t)
         int64Val.getUInt8( u8v );   EXPECT_TRUE( u8v == 64   );
        uint64Val.getUInt8( u8v );   EXPECT_TRUE( u8v == 0x40 );  // 6464 truncated to  (uint8_t)
         floatVal.getUInt8( u8v );   EXPECT_TRUE( u8v == 3    );
        doubleVal.getUInt8( u8v );   EXPECT_TRUE( u8v == 3    );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        stringVal.getUInt8( u8v );   EXPECT_TRUE( u8v == u8v  );
        // objectVal.getUInt8( u8v );   EXPECT_TRUE( u8v == u8v  );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }

    void getInt32Test()
    {
         // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        int32_t i32v;

          boolVal.getInt32( i32v );   EXPECT_TRUE( i32v == 1    );
          int8Val.getInt32( i32v );   EXPECT_TRUE( i32v == 8    );
         uint8Val.getInt32( i32v );   EXPECT_TRUE( i32v == 88   );
         int32Val.getInt32( i32v );   EXPECT_TRUE( i32v == 32   );   // ##
        uint32Val.getInt32( i32v );   EXPECT_TRUE( i32v == 3232 );
         int64Val.getInt32( i32v );   EXPECT_TRUE( i32v == 64   );
        uint64Val.getInt32( i32v );   EXPECT_TRUE( i32v == 6464 );
         floatVal.getInt32( i32v );   EXPECT_TRUE( i32v == 3    );
        doubleVal.getInt32( i32v );   EXPECT_TRUE( i32v == 3    );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        stringVal.getInt32( i32v );   EXPECT_TRUE( i32v == i32v );
        // objectVal.getInt32( i32v );   EXPECT_TRUE( i32v == i32v );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }

    void getUInt32Test()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        uint32_t u32v;

          boolVal.getUInt32( u32v );   EXPECT_TRUE( u32v == 1    );
          int8Val.getUInt32( u32v );   EXPECT_TRUE( u32v == 8    );
         uint8Val.getUInt32( u32v );   EXPECT_TRUE( u32v == 88   );
         int32Val.getUInt32( u32v );   EXPECT_TRUE( u32v == 32   );
        uint32Val.getUInt32( u32v );   EXPECT_TRUE( u32v == 3232 );
         int64Val.getUInt32( u32v );   EXPECT_TRUE( u32v == 64   );
        uint64Val.getUInt32( u32v );   EXPECT_TRUE( u32v == 6464 );
         floatVal.getUInt32( u32v );   EXPECT_TRUE( u32v == 3    );
        doubleVal.getUInt32( u32v );   EXPECT_TRUE( u32v == 3    );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        stringVal.getUInt32( u32v );   EXPECT_TRUE( u32v == u32v );
        // objectVal.getUInt32( u32v);   EXPECT_TRUE( u32v == u32v    );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }


    void getInt64Test()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        int64_t i64v;

          boolVal.getInt64( i64v );   EXPECT_TRUE( i64v == 1    );
          int8Val.getInt64( i64v );   EXPECT_TRUE( i64v == 8    );
         uint8Val.getInt64( i64v );   EXPECT_TRUE( i64v == 88   );
         int32Val.getInt64( i64v );   EXPECT_TRUE( i64v == 32   );
        uint32Val.getInt64( i64v );   EXPECT_TRUE( i64v == 3232 );
         int64Val.getInt64( i64v );   EXPECT_TRUE( i64v == 64   );
        uint64Val.getInt64( i64v );   EXPECT_TRUE( i64v == 6464 );
         floatVal.getInt64( i64v );   EXPECT_TRUE( i64v == 3    );
        doubleVal.getInt64( i64v );   EXPECT_TRUE( i64v == 3    );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        stringVal.getInt64( i64v );   EXPECT_TRUE( i64v == i64v );
        // objectVal.getInt64( i64v );   EXPECT_TRUE( i64v == i64v    );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }

    void getUInt64Test()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        uint64_t u64v;

          boolVal.getUInt64( u64v );   EXPECT_TRUE( u64v == 1    );
          int8Val.getUInt64( u64v );   EXPECT_TRUE( u64v == 8    );
         uint8Val.getUInt64( u64v );   EXPECT_TRUE( u64v == 88   );
         int32Val.getUInt64( u64v );   EXPECT_TRUE( u64v == 32   );
        uint32Val.getUInt64( u64v );   EXPECT_TRUE( u64v == 3232 );
         int64Val.getUInt64( u64v );   EXPECT_TRUE( u64v == 64   );
        uint64Val.getUInt64( u64v );   EXPECT_TRUE( u64v == 6464 );
         floatVal.getUInt64( u64v );   EXPECT_TRUE( u64v == 3    );
        doubleVal.getUInt64( u64v );   EXPECT_TRUE( u64v == 3    );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        // stringVal.getString( u64v );   EXPECT_TRUE( u64v == u64v );
        // objectVal.getUInt64( u64v );   EXPECT_TRUE( u64v == u64v );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }


    void getFloatTest()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        float fv;

          boolVal.getFloat( fv );   EXPECT_TRUE( fv == 1.0f    );
          int8Val.getFloat( fv );   EXPECT_TRUE( fv == 8.0f    );
         uint8Val.getFloat( fv );   EXPECT_TRUE( fv == 88.0f   );
         int32Val.getFloat( fv );   EXPECT_TRUE( fv == 32.0f   );
        uint32Val.getFloat( fv );   EXPECT_TRUE( fv == 3232.0f );
         int64Val.getFloat( fv );   EXPECT_TRUE( fv == 64.0f   );
        uint64Val.getFloat( fv );   EXPECT_TRUE( fv == 6464.0f );
         floatVal.getFloat( fv );   EXPECT_TRUE( fv == 3.14f   );
        doubleVal.getFloat( fv );   EXPECT_TRUE( fv == 3.142f  );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        //  rtString  sv;   stringVal.getString( sv   );   EXPECT_TRUE(   sv == "rtString here");
        // objectVal.getFloat( fv );   EXPECT_TRUE( fv == fv      );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }

    void getDoubleTest()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        double dv;

          boolVal.getDouble( dv );   EXPECT_TRUE( dv == 1.0    );
          int8Val.getDouble( dv );   EXPECT_TRUE( dv == 8.0    );
         uint8Val.getDouble( dv );   EXPECT_TRUE( dv == 88.0   );
         int32Val.getDouble( dv );   EXPECT_TRUE( dv == 32.0   );
        uint32Val.getDouble( dv );   EXPECT_TRUE( dv == 3232.0 );
         int64Val.getDouble( dv );   EXPECT_TRUE( dv == 64.0   );
        uint64Val.getDouble( dv );   EXPECT_TRUE( dv == 6464.0 );
         floatVal.getDouble( dv );   EXPECT_TRUE( (dv - 3.140) < 0.002 );
        doubleVal.getDouble( dv );   EXPECT_TRUE( (dv - 3.142) < 0.002 );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        //  rtString  sv;   stringVal.getString( sv   );   EXPECT_TRUE(   sv == "rtString here");
        // objectVal.getDouble( dv );   EXPECT_TRUE( dv == dv );
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // voidPtrVal.getDouble( dv );  EXPECT_TRUE( dv == dv );
    }

    void getStringTest()
    {
        // SET - Not Empty
        //
        SetValues();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // These should ALL be Non-EMPTY now...

        rtString sv;

          boolVal.getString( sv );   EXPECT_TRUE( sv == "true"  );
          int8Val.getString( sv );   EXPECT_TRUE( sv == "8"     );
         uint8Val.getString( sv );   EXPECT_TRUE( sv == "88"    );
         int32Val.getString( sv );   EXPECT_TRUE( sv == "32"    );
        uint32Val.getString( sv );   EXPECT_TRUE( sv == "3232"  );
         int64Val.getString( sv );   EXPECT_TRUE( sv == "64"    );
        uint64Val.getString( sv );   EXPECT_TRUE( sv == "6464"  );
         floatVal.getString( sv );   EXPECT_TRUE( sv == "3.14"  );
        doubleVal.getString( sv );   EXPECT_TRUE( sv == "3.142" );
        //    char*   cv;     charVal.getString( cv   );   EXPECT_TRUE(   cv == "cString here");
        //  rtString  sv;   stringVal.getString( sv   );   EXPECT_TRUE(   sv == "rtString here");
        // rtValue  objectVal.getVALUE(const rtIObject* v);
        // rtValue  objRefVal.getVALUE(const rtObjectRef& v);
        // rtValue    funcVal.getVALUE(const rtIFunction* v);
        // rtValue funcRefVal.getVALUE(const rtFunctionRef& v);
        // rtValue   valueVal.getVALUE(const rtValue& v);
        // rtValue voidPtrVal.getVALUE(voidPtr v);
    }

    void testStringType()
    {
      // Some exceptions from pattern...
      //
      EXPECT_TRUE( strcmp( rtStrType( RT_voidType          ), RT_TYPE_NAME( RT_voidType         )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_valueType         ), "UNKNOWN" ) == 0 );
//      EXPECT_TRUE( strcmp( rtStrType( RT_valueType         ), RT_TYPE_NAME( RT_valueType        )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_rtValueType       ), "UNKNOWN" ) == 0 );
//      EXPECT_TRUE( strcmp( rtStrType( RT_rtValueType       ), RT_TYPE_NAME( RT_rtValueType      )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_boolType          ), RT_TYPE_NAME( RT_boolType         )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_int8_tType        ), RT_TYPE_NAME( RT_int8_tType       )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_uint8_tType       ), RT_TYPE_NAME( RT_uint8_tType      )) == 0 );
//      EXPECT_TRUE( strcmp( rtStrType( RT_intType           ), RT_TYPE_NAME( RT_intType          )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_intType           ), RT_TYPE_NAME( RT_int32_tType      )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_int32_tType       ), RT_TYPE_NAME( RT_int32_tType      )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_uint32_tType      ), RT_TYPE_NAME( RT_uint32_tType     )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_int64_tType       ), RT_TYPE_NAME( RT_int64_tType      )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_uint64_tType      ), RT_TYPE_NAME( RT_uint64_tType     )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_floatType         ), RT_TYPE_NAME( RT_floatType        )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_doubleType        ), RT_TYPE_NAME( RT_doubleType       )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_rtStringType      ), RT_TYPE_NAME( RT_stringType       )) == 0 );
//      EXPECT_TRUE( strcmp( rtStrType( RT_rtStringType      ), RT_TYPE_NAME( RT_rtStringType     )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_rtObjectRefType   ), RT_TYPE_NAME( RT_objectType  )) == 0 );
//      EXPECT_TRUE( strcmp( rtStrType( RT_rtObjectRefType   ), RT_TYPE_NAME( RT_rtObjectRefType  )) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_rtFunctionRefType ), RT_TYPE_NAME( RT_functionType)) == 0 );
//      EXPECT_TRUE( strcmp( rtStrType( RT_rtFunctionRefType ), RT_TYPE_NAME( RT_rtFunctionRefType)) == 0 );
      EXPECT_TRUE( strcmp( rtStrType( RT_voidPtrType       ), RT_TYPE_NAME( RT_voidPtrType      )) == 0 );
    }


    void setTest()
    {
        // SET - Not Empty
        //
        SetEmpty();

        // CHECK - Empty
        //
        CheckEmpty();

        // SET - Not Empty
        //
        SetValues();

        // CHECK - Not Empty
        //
        CheckNonEmpty();

        // CHECK - values set as directed.
        //
        rtError err = RT_OK;

            bool   bv;   err =   boolVal.getBool(bv);       EXPECT_TRUE(err == RT_OK);
          int8_t  i8v;   err =   int8Val.getInt8(i8v);      EXPECT_TRUE(err == RT_OK);
         uint8_t  u8v;   err =  uint8Val.getUInt8(u8v);     EXPECT_TRUE(err == RT_OK);
         int32_t i32v;   err =  int32Val.getInt32(i32v);    EXPECT_TRUE(err == RT_OK);
        uint32_t u32v;   err = uint32Val.getUInt32(u32v);   EXPECT_TRUE(err == RT_OK);
         int64_t i64v;   err =  int64Val.getInt64(i64v);    EXPECT_TRUE(err == RT_OK);
        uint64_t u64v;   err = uint64Val.getUInt64(u64v);   EXPECT_TRUE(err == RT_OK);
           float   fv;   err =  floatVal.getFloat(fv);      EXPECT_TRUE(err == RT_OK);
          double   dv;   err = doubleVal.getDouble(dv);     EXPECT_TRUE(err == RT_OK);
        //    char*   cv;   err =   charVal.getString(&cv);     EXPECT_TRUE(err == RT_OK);
        //  rtString  sv;   err = stringVal.getString(&sv);     EXPECT_TRUE(err == RT_OK);
        //  objectVal.setVALUE(const rtIObject* v);      EXPECT_TRUE(   fv == 3.14f);
        //  objRefVal.setVALUE(const rtObjectRef& v);    EXPECT_TRUE(   fv == 3.14f);
        //    funcVal.setVALUE(const rtIFunction* v);    EXPECT_TRUE(   fv == 3.14f);
        // funcRefVal.setVALUE(const rtFunctionRef& v);  EXPECT_TRUE(   fv == 3.14f);
        //   valueVal.setVALUE(const rtValue& v);        EXPECT_TRUE(   fv == 3.14f);
        // voidPtrVal.setVALUE(voidPtr v);               EXPECT_TRUE(   fv == 3.14f);
    }

    void compareTest()
    {
        rtValue    myVal_1(true);
        rtValue    myVal_2(true);

        EXPECT_TRUE( myVal_1 == myVal_2);

        myVal_2.setBool(false);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        myVal_1.setInt8( int8_t(8) );
        myVal_2.setInt8( int8_t(8) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setInt8(99);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        myVal_1.setUInt8( uint8_t(8) );
        myVal_2.setUInt8( uint8_t(8) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setUInt8(99);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        myVal_1.setInt32( int32_t(32) );
        myVal_2.setInt32( int32_t(32) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setInt32(99);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        myVal_1.setUInt32( uint32_t(3232) );
        myVal_2.setUInt32( uint32_t(3232) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setUInt32(99);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        myVal_1.setInt64( int64_t(64) );
        myVal_2.setInt64( int64_t(64) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setInt64(99);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        myVal_1.setUInt64( uint64_t(6464) );
        myVal_2.setUInt64( uint64_t(6464) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setUInt64(99);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        myVal_1.setFloat( float(3.14f) );
        myVal_2.setFloat( float(3.14f) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setFloat(2.14f);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        myVal_1.setDouble( double(3.142) );
        myVal_2.setDouble( double(3.142) );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setDouble(2.14);
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        myVal_1.setString( (const char*) "cString here" );
        myVal_2.setString( (const char*) "cString here" );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setString("Wrong here");
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        myVal_1.setString( rtString("rtString here") );
        myVal_2.setString( rtString("rtString here") );

        EXPECT_TRUE(  myVal_1 == myVal_2);

        myVal_2.setString( rtString("Wrong here") );
        EXPECT_TRUE(  myVal_1 != myVal_2);
        EXPECT_FALSE( myVal_1 == myVal_2);
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        // rtValue  objectVal(const rtIObject* v);
        // rtValue  objRefVal(const rtObjectRef& v);
        // rtValue    funcVal(const rtIFunction* v);
        // rtValue funcRefVal(const rtFunctionRef& v);
        // rtValue   valueVal(const rtValue& v);
        // rtValue voidPtrVal(voidPtr v);
      }

    private:
      rtValue    boolVal;
      rtValue    int8Val;
      rtValue   uint8Val;
      rtValue   int32Val;
      rtValue  uint32Val;
      rtValue   int64Val;
      rtValue  uint64Val;
      rtValue   floatVal;
      rtValue  doubleVal;
      rtValue    charVal;
      rtValue  stringVal;

      // rtValue  objectVal;
      // rtValue  objRefVal;
      // rtValue    funcVal;
      // rtValue funcRefVal;
      // rtValue   valueVal;

      rtValue voidPtrVal;

}; // CLASS


TEST_F(rtValueTest, rtValueTests)
{
  ctorToTests();
  
  isEmptyTest();
  
  setTest();
  getTest();

  getBoolTest();
    
  getInt8Test();
  getUInt8Test();
  
  getInt32Test();
  getUInt32Test();

  getInt64Test();
  getUInt64Test();

  getFloatTest();
  getDoubleTest();

  getStringTest();
  testStringType();
  
  compareTest();
}

