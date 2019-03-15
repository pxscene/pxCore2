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
#include "sqlite3.h"

#define SQLITE *(sqlite3**)&mPrivateData

rtStorage::rtStorage(const char* fileName, const char* key): mPrivateData(NULL)
{
  init(fileName, key);
}

rtStorage::~rtStorage()
{
  term();
}


rtError rtStorage::init(const char* fileName, const char* key)
{
  sqlite3* &db = SQLITE;

  term();

  int rc = sqlite3_open(fileName, &db);
  if(rc)
  {
    term();
    return RT_FAIL;
  }

  sqlite3_stmt *stmt;

  char *errmsg;
  sqlite3_exec(db, "CREATE TABLE ItemTable (key TEXT UNIQUE ON CONFLICT REPLACE, value TEXT);", 0, 0, &errmsg);

  return RT_OK;
}

// closes file
rtError rtStorage::term()
{
    sqlite3* &db = SQLITE;

    if (db)
      sqlite3_close(db);

    db = NULL;

    return RT_OK;
}

rtError rtStorage::setItem(const char* key, const rtValue& value)
{
  sqlite3* &db = SQLITE;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO ItemTable (key,value) values (?,?);", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.toString(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
      printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtError rtStorage::getItem(const char* key, rtValue& retValue) const
{
  sqlite3* &db = SQLITE;

  retValue = "";

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM ItemTable where key = ?;", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) 
    {
      // JRJR TODO consider adding (const unsigned char*) accessors for rtValue
      retValue = (const char*)sqlite3_column_text(stmt, 1);
    }
    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtError rtStorage::getItems(const char* key, rtObjectRef& retValue) const
{
  sqlite3* &db = SQLITE;

  rtRef<rtArrayObject> a = new rtArrayObject();

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM ItemTable WHERE key LIKE ? || \'%\' ORDER BY key;", -1, &stmt, NULL);

    rtLogError("getItems query %s",  "SELECT * FROM ItemTable WHERE key LIKE ? || \'%\';");

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    while(rc == SQLITE_ROW) 
    {
      rtLogError("In step loop %d", rc);
      rtObjectRef row = new rtMapObject();
      
      for (int i = 0; i < sqlite3_column_count(stmt); i++)
      {
        // JRJR TODO consider adding (const unsigned char*) accessors for rtValue
        row.set((const char*)sqlite3_column_name(stmt, i), (const char*)sqlite3_column_text(stmt,i));
      }

      a->pushBack(row);
      rc = sqlite3_step(stmt);
    }

    retValue = a;

    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtError rtStorage::removeItem(const char* key)
{
  sqlite3* &db = SQLITE;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM ItemTable where key = ?;", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
      printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

// Clear all data
rtError rtStorage::clear()
{
  sqlite3* &db = SQLITE;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM ItemTable;", -1, &stmt, NULL);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
      printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtDefineObject(rtStorage,rtObject);
rtDefineMethod(rtStorage,setItem);
rtDefineMethod(rtStorage,getItem);
rtDefineMethod(rtStorage,getItems);
rtDefineMethod(rtStorage,removeItem);
rtDefineMethod(rtStorage,clear);