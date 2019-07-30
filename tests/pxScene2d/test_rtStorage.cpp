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

#include "rtStorage.h"

#include "rtPathUtils.h"
#include "pxTimer.h"

#include "test_includes.h" // Needs to be included last

namespace
{
  const char* testStorageLocation = "/tmp/sparkTestStorage";
#if defined(SQLITE_HAS_CODEC)
  const char* encryptedTestStorageLocation = "/tmp/sparkEncryptedTestStorage";
  const char* encryptedStorageKey = "PDTuvnsuTuDnQQyB";
#endif

#ifdef ENABLE_STORAGE_PERF_TEST
  const int perfItems = 50;
  const int perfOperations = 100;
  const double setTime = 0.05;
  const double getTime = 0.05;
  const double getAllTime = 0.05;
  const double removeTime = 0.05;
#endif
}

class rtStorageTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void setItem_test()
  {
    rtStorageRef s;
    rtValue item;
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1"));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string("value1"), str.cString());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void getItem_test()
  {
    rtStorageRef s;
    rtValue item;
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key2", "value2"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1_changed"));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string("value1_changed"), str.cString());
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key2", item));
    str = item.toString();
    EXPECT_EQ (std::string("value2"), str.cString());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void getItems_test()
  {
    rtStorageRef s;
    rtObjectRef items;
    rtValue item;
    rtObjectRef obj;
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key2", "value2"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key3", "value3"));
    // GET ALL
    EXPECT_EQ ((int)RT_OK, (int)s->getItems("", items));
    // VERIFY LENGTH
    EXPECT_EQ ((int)RT_OK, (int)items->Get("length", &item));
    EXPECT_EQ ((int)3, (int)item.toInt32());
    // VERIFY #0
    EXPECT_EQ ((int)RT_OK, (int)items->Get((uint32_t)0, &item));
    obj = item.toObject();
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("key", &item));
    str = item.toString();
    EXPECT_EQ (std::string("key1"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("value", &item));
    str = item.toString();
    EXPECT_EQ (std::string("value1"), str.cString());
    // VERIFY #1
    EXPECT_EQ ((int)RT_OK, (int)items->Get(1, &item));
    obj = item.toObject();
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("key", &item));
    str = item.toString();
    EXPECT_EQ (std::string("key2"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("value", &item));
    str = item.toString();
    EXPECT_EQ (std::string("value2"), str.cString());
    // VERIFY #2
    EXPECT_EQ ((int)RT_OK, (int)items->Get(2, &item));
    obj = item.toObject();
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("key", &item));
    str = item.toString();
    EXPECT_EQ (std::string("key3"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("value", &item));
    str = item.toString();
    EXPECT_EQ (std::string("value3"), str.cString());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void getItemsPrefix_test()
  {
    rtStorageRef s;
    rtObjectRef items;
    rtValue item;
    rtObjectRef obj;
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("prefix1_key1", "value1"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("prefix1_key2", "value2"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("prefix2_key1", "value3"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("prefix2_key2", "value4"));
    // GET PREFIX
    EXPECT_EQ ((int)RT_OK, (int)s->getItems("prefix1", items));
    // VERIFY LENGTH
    EXPECT_EQ ((int)RT_OK, (int)items->Get("length", &item));
    EXPECT_EQ ((int)2, (int)item.toInt32());
    // VERIFY #0
    EXPECT_EQ ((int)RT_OK, (int)items->Get((uint32_t)0, &item));
    obj = item.toObject();
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("key", &item));
    str = item.toString();
    EXPECT_EQ (std::string("prefix1_key1"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("value", &item));
    str = item.toString();
    EXPECT_EQ (std::string("value1"), str.cString());
    // VERIFY #1
    EXPECT_EQ ((int)RT_OK, (int)items->Get(1, &item));
    obj = item.toObject();
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("key", &item));
    str = item.toString();
    EXPECT_EQ (std::string("prefix1_key2"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)obj->Get("value", &item));
    str = item.toString();
    EXPECT_EQ (std::string("value2"), str.cString());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void removeItem_test()
  {
    rtStorageRef s;
    rtValue item;
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1"));
    EXPECT_EQ ((int)RT_OK, (int)s->removeItem("key1"));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string(""), str.cString());
    EXPECT_TRUE (item.isEmpty());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void clear_test()
  {
    rtStorageRef s;
    rtObjectRef items;
    rtValue item;
    rtObjectRef obj;
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key2", "value2"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key3", "value3"));
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string(""), str.cString());
    EXPECT_TRUE (item.isEmpty());
    // GET ALL
    EXPECT_EQ ((int)RT_OK, (int)s->getItems("", items));
    // VERIFY LENGTH
    EXPECT_EQ ((int)RT_OK, (int)items->Get("length", &item));
    EXPECT_EQ ((int)0, (int)item.toInt32());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void persistence_test()
  {
    rtStorageRef s;
    rtValue item;
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "persistent"));
    // REOPEN
    EXPECT_EQ ((int)RT_OK, (int)s->term());
    s = new rtStorage(testStorageLocation, 100);
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string("persistent"), str.cString());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void quota_test()
  {
    rtStorageRef s;

    s = new rtStorage(testStorageLocation, 20);

    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    // size 0
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1")); // +10
    // size 10
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("k2", "v2")); // +4
    // size 14
    EXPECT_EQ ((int)RT_ERROR, (int)s->setItem("key3", "value3")); // +10
    // size 14
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("k3", "v3")); // +4
    // size 18
    EXPECT_EQ ((int)RT_ERROR, (int)s->setItem("k4", "v4")); // +4
    // size 18
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("4", "4")); // +2
    // size 20
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("4", "5")); // +2 -2
    // size 20
    EXPECT_EQ ((int)RT_ERROR, (int)s->setItem("4", "66")); // +3 -2
    // size 20

    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

  void badInput_test()
  {
    rtStorageRef s;
    rtObjectRef items;
    rtValue item;
    rtObjectRef obj;
    rtString str;
    int32_t len;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->init(NULL, 0));
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->init("", 0));
    EXPECT_EQ ((int)RT_OK,                (int)s->clear());
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->setItem("", ""));
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->setItem("", "valueB"));
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("c", ""));
    item.term();
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->setItem(NULL, item));
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->setItem(NULL, "valueE"));
    item.term();
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("f", item));
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("g", (bool) true));
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("h", (bool) false));
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("i", (double) 3.14159265359));
    obj = new rtMapObject();
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("j", obj));
    obj = new rtArrayObject();
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("k", obj));
    EXPECT_EQ ((int)RT_OK,                (int)s->setItem("l", "valueL"));
    // REMOVE
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->removeItem(""));
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->removeItem(NULL));
    // GET
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->getItem("", item));
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->getItem(NULL, item));
    EXPECT_EQ ((int)RT_ERROR_INVALID_ARG, (int)s->getItems(NULL, items));
    EXPECT_EQ ((int)RT_OK,                (int)s->getItems("", items));
    // VERIFY LENGTH
    EXPECT_EQ ((int)RT_OK, (int)items->Get("length", &item));
    len = item.toInt32();
    EXPECT_EQ ((int)8, (int)len);
    // VERIFY
    const char* const keys[] = {
          "c",
          "f",
          "g",
          "h",
          "i",
          "j",
          "k",
          "l"
      };
    const char* const values[] = {
          "",
          "",
          "true",
          "false",
          "3.14159",
          "",
          "",
          "valueL"
      };
    for (int i = 0; i < len; i++)
    {
      EXPECT_EQ ((int)RT_OK, (int)items->Get((uint32_t)i, &item));
      obj = item.toObject();
      EXPECT_EQ ((int)RT_OK, (int)obj->Get("key", &item));
      str = item.toString();
      EXPECT_EQ (std::string(keys[i]), str.cString());
      EXPECT_EQ ((int)RT_OK, (int)obj->Get("value", &item));
      str = item.toString();
      EXPECT_EQ (std::string(values[i]), str.cString());
    }
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }

#ifdef ENABLE_STORAGE_PERF_TEST
  void performance_test()
  {
    rtStorageRef s;
    rtObjectRef items;
    rtValue item;
    double secs;

    rtString keys[perfItems];
    rtString values[perfItems];
    for (int i = 0; i < perfItems; i++)
    {
      rtString i_str = rtValue(i).toString();
      keys[i] = "key" + i_str;
      values[i] = "value" + i_str;
    }

    s = new rtStorage(testStorageLocation, 1000000);

    // SET or REPLACE, ~0.023262
    secs = pxSeconds();
    for (int i = 0; i < perfOperations; i++)
      s->setItem(keys[i % perfItems], values[i % perfItems]);
    secs = (pxSeconds() - secs) / perfOperations;
    EXPECT_LE (secs, setTime);

    // GET, ~0.000016
    secs = pxSeconds();
    for (int i = 0; i < perfOperations; i++)
      s->getItem(keys[i % perfItems], item);
    secs = (pxSeconds() - secs) / perfOperations;
    EXPECT_LE (secs, getTime);

    // GET ALL, ~0.000357
    secs = pxSeconds();
    s->getItems("", items);
    secs = pxSeconds() - secs;
    EXPECT_LE (secs, getAllTime);

    // REMOVE, ~0.023571
    secs = pxSeconds();
    for (int i = 0; i < perfItems; i++)
      s->removeItem(keys[i]);
    secs = (pxSeconds() - secs) / perfItems;
    EXPECT_LE (secs, removeTime);

    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }
#endif

#if defined(SQLITE_HAS_CODEC)
  void encryption_test()
  {
    rtStorageRef s;
    rtValue item;
    rtString str;

    auto cmd = rtString("rm -f ").append(encryptedTestStorageLocation);
    EXPECT_EQ ((int)0, (int)system(cmd.cString()));
    EXPECT_FALSE (rtFileExists(encryptedTestStorageLocation));

    // NO KEY
    s = new rtStorage(encryptedTestStorageLocation, 100);
    EXPECT_TRUE (rtFileExists(encryptedTestStorageLocation));
    EXPECT_FALSE (rtStorage::isEncrypted(encryptedTestStorageLocation));
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("k1", "v1"));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("k1", item));
    str = item.toString();
    EXPECT_EQ (std::string("v1"), str.cString());

    // REKEY
    s = new rtStorage(encryptedTestStorageLocation, 100, encryptedStorageKey);
    EXPECT_TRUE (rtStorage::isEncrypted(encryptedTestStorageLocation));
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("encrypted_k1", "encrypted_v1"));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("k1", item));
    str = item.toString();
    EXPECT_EQ (std::string("v1"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("encrypted_k1", item));
    str = item.toString();
    EXPECT_EQ (std::string("encrypted_v1"), str.cString());

    // WRONG KEY
    EXPECT_EQ ((int)RT_OK, (int)s->init(encryptedTestStorageLocation, 100, "LrAmyMguFVJbQMLJ"));
    EXPECT_TRUE (rtStorage::isEncrypted(encryptedTestStorageLocation));
    // GET FAILS, DB ENCRYPTED
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string(""), str.cString());
    EXPECT_TRUE (item.isEmpty());

    // NO KEY
    EXPECT_EQ ((int)RT_OK, (int)s->init(encryptedTestStorageLocation, 100));
    EXPECT_TRUE (rtStorage::isEncrypted(encryptedTestStorageLocation));
    // GET FAILS, DB ENCRYPTED
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string(""), str.cString());
    EXPECT_TRUE (item.isEmpty());

    // KEY
    s = new rtStorage(encryptedTestStorageLocation, 100, encryptedStorageKey);
    EXPECT_TRUE (rtStorage::isEncrypted(encryptedTestStorageLocation));
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("encrypted_k2", "encrypted_v2"));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("k1", item));
    str = item.toString();
    EXPECT_EQ (std::string("v1"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("encrypted_k1", item));
    str = item.toString();
    EXPECT_EQ (std::string("encrypted_v1"), str.cString());
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("encrypted_k2", item));
    str = item.toString();
    EXPECT_EQ (std::string("encrypted_v2"), str.cString());

    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }
#endif

  void dotBracketNotation_test()
  {
    rtStorageRef s;
    rtValue item;
    rtValue value("value1");
    rtString str;

    s = new rtStorage(testStorageLocation, 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->Set("key1", &value));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->Get("key1", &item));
    EXPECT_TRUE (item == value);
    EXPECT_EQ ((int)RT_OK, (int)s->Get("setItem", &item));
    EXPECT_EQ ((int)RT_functionType, (int)item.getType());
    EXPECT_EQ ((int)RT_PROP_NOT_FOUND, (int)s->Get("keyNotExist", &item));
    EXPECT_TRUE (item.isEmpty());
    // CLOSE
    EXPECT_EQ ((int)RT_OK, (int)s->term());
  }
};

TEST_F(rtStorageTest, rtStorageTests)
{
  setItem_test();
  getItem_test();
  getItems_test();
  getItemsPrefix_test();
  removeItem_test();
  clear_test();
  persistence_test();
  quota_test();
  badInput_test();
#ifdef ENABLE_STORAGE_PERF_TEST
  performance_test();
#endif
#if defined(SQLITE_HAS_CODEC)
  encryption_test();
#endif
  dotBracketNotation_test();
}
