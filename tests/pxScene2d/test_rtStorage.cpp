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

#include "test_includes.h" // Needs to be included last

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

    s = new rtStorage("/tmp/sparkTestStorage", 100);
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

    s = new rtStorage("/tmp/sparkTestStorage", 100);
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

    s = new rtStorage("/tmp/sparkTestStorage", 100);
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

    s = new rtStorage("/tmp/sparkTestStorage", 100);
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

    s = new rtStorage("/tmp/sparkTestStorage", 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1"));
    EXPECT_EQ ((int)RT_OK, (int)s->removeItem("key1"));
    // GET
    EXPECT_EQ ((int)RT_OK, (int)s->getItem("key1", item));
    str = item.toString();
    EXPECT_EQ (std::string(""), str.cString());
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

    s = new rtStorage("/tmp/sparkTestStorage", 100);
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

    s = new rtStorage("/tmp/sparkTestStorage", 100);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "persistent"));
    // REOPEN
    EXPECT_EQ ((int)RT_OK, (int)s->term());
    s = new rtStorage("/tmp/sparkTestStorage", 100);
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

    s = new rtStorage("/tmp/sparkTestStorage", 20);
    // SET
    EXPECT_EQ ((int)RT_OK, (int)s->clear());
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key1", "value1"));
    EXPECT_EQ ((int)RT_OK, (int)s->setItem("key2", "value2"));
    EXPECT_EQ ((int)RT_ERROR, (int)s->setItem("key3", "value3"));
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
}