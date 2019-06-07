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

#include "rtPathUtils.h"
#include "rtLog.h"

#if defined(USE_PLABELS)
#include "pbnj_utils.hpp"
#endif

#ifndef SQLITE_FILE_HEADER
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif

#define SQLITE *(sqlite3**)&mPrivateData

rtStorage::rtStorage(const char* filename, const uint32_t storageQuota, const char* key)
  : mPrivateData(NULL)
{
  init(filename, storageQuota, key);
}

rtStorage::~rtStorage()
{
  term();
}

rtError rtStorage::init(const char* filename, uint32_t storageQuota, const char* key)
{
  if (!filename || *filename == 0)
    return RT_ERROR_INVALID_ARG;

  rtLogInfo("%s: %db @ '%s'", __FUNCTION__, storageQuota, filename);

  sqlite3* &db = SQLITE;

  term();

#if defined(SQLITE_HAS_CODEC)
  bool shouldReKey = false;
  if (key && *key)
    shouldReKey = rtFileExists(filename) && !isEncrypted(filename);
#endif

  int rc = sqlite3_open(filename, &db);
  if(rc)
  {
    term();
    return RT_FAIL;
  }

  if (key && *key)
  {
#if defined(SQLITE_HAS_CODEC)
    std::vector<uint8_t> pKey;

#if defined(USE_PLABELS)
    bool result = pbnj_utils::prepareBufferForOrigin(key, [&pKey](const std::vector<uint8_t>& buffer) {
      pKey = buffer;
    });
    if (!result)
      return RT_FAIL;
#else
    rtLogWarn("SQLite encryption key is not secure, path=%s", filename);
    pKey = std::vector<uint8_t>(key, key + strlen(key));
#endif

    if (!shouldReKey)
      rc = sqlite3_key_v2(db, nullptr, pKey.data(), pKey.size());
    else
    {
      rc = sqlite3_rekey_v2(db, nullptr, pKey.data(), pKey.size());
      if (rc == SQLITE_OK)
        runVacuumCommand();
    }

    if (rc != SQLITE_OK)
    {
      rtLogError("Failed to attach encryption key to SQLite database %s\nCause - %s",
        filename, sqlite3_errmsg(db));
      term();
      return RT_FAIL;
    }

    if (shouldReKey && !isEncrypted(filename))
      rtLogError("SQLite database file is clear after re-key, path=%s", filename);
#endif
  }

  sqlite3_stmt *stmt;

  char *errmsg;
  sqlite3_exec(db, "CREATE TABLE if not exists SizeTable (currentSize MEDIUMINT_UNSIGNED UNIQUE ON CONFLICT REPLACE);", 0, 0, &errmsg);
  sqlite3_exec(db, "CREATE TABLE if not exists ItemTable (key TEXT UNIQUE ON CONFLICT REPLACE, value TEXT);", 0, 0, &errmsg);

  // initial currentSize must be 0
  sqlite3_exec(db, "INSERT OR IGNORE INTO SizeTable (currentSize) VALUES(0);", 0, 0, &errmsg);

  mQuota = storageQuota;
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
  if (!key || *key == 0)
    return RT_ERROR_INVALID_ARG;

  if (RT_OK != verifyQuota(key, value))
    return RT_ERROR;

  rtLogInfo("%s: '%s'", __FUNCTION__, key);

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
      rtLogError("ERROR inserting data: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
  }

  updateSize();

  return RT_OK;
}

rtError rtStorage::getItem(const char* key, rtValue& retValue) const
{
  if (!key || *key == 0)
    return RT_ERROR_INVALID_ARG;

  rtLogInfo("%s: '%s'", __FUNCTION__, key);

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
  if (!key)
    return RT_ERROR_INVALID_ARG;

  rtLogInfo("%s: '%s'", __FUNCTION__, key);

  sqlite3* &db = SQLITE;

  rtRef<rtArrayObject> a = new rtArrayObject();

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM ItemTable WHERE key LIKE ? || \'%\' ORDER BY key;", -1, &stmt, NULL);

    rtLogDebug("getItems query %s",  "SELECT * FROM ItemTable WHERE key LIKE ? || \'%\';");

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    while(rc == SQLITE_ROW) 
    {
      rtLogDebug("In step loop %d", rc);
      rtObjectRef row = new rtMapObject();
      
      for (int i = 0; i < sqlite3_column_count(stmt); i++)
      {
        // JRJR TODO consider adding (const unsigned char*) accessors for rtValue
        row.set((const char*)sqlite3_column_name(stmt, i), (const char*)sqlite3_column_text(stmt,i));
      }

      a->pushBack(row);
      rc = sqlite3_step(stmt);

      rtLogInfo("%s: key '%s'", __FUNCTION__, row.get<rtString>("key").cString());
    }

    retValue = a;

    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtError rtStorage::removeItem(const char* key)
{
  if (!key || *key == 0)
    return RT_ERROR_INVALID_ARG;

  rtLogInfo("%s: '%s'", __FUNCTION__, key);

  sqlite3* &db = SQLITE;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM ItemTable where key = ?;", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
      rtLogError("ERROR removing data: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
  }

  updateSize();

  return RT_OK;
}

// Methods for managing current size and quota
rtError rtStorage::calculateCurrentSize(uint32_t &size) const
{
  sqlite3* &db = SQLITE;

  size = 0;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT sum(length(key)+length(value)) FROM ItemTable;", -1, &stmt, NULL);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
      size = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtError rtStorage::setCurrentSize(const uint32_t size)
{
  sqlite3* &db = SQLITE;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE SizeTable SET currentSize = ?;", -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, size);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
      rtLogError("ERROR inserting size: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtError rtStorage::getCurrentSize(const char* key, uint32_t& sizeForKey, uint32_t& sizeTotal) const
{
  if (!key || *key == 0)
    return RT_ERROR_INVALID_ARG;

  sqlite3* &db = SQLITE;

  sizeForKey = 0;
  sizeTotal = 0;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM SizeTable;", -1, &stmt, NULL);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
      sizeTotal = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT length(value) FROM ItemTable WHERE key = ?;", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
      sizeForKey = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return RT_OK;
}

rtError rtStorage::updateSize()
{
  uint32_t size = 0;
  rtError retStat = calculateCurrentSize(size);

  if (RT_OK == retStat)
  {
    retStat = setCurrentSize(size);
    if (RT_OK == retStat)
    {
      rtLogDebug("storage size is %d", size);
    }
    else
    {
      rtLogError("failed to update size");
    }
  }
  else
  {
    rtLogError("failed to calculate size");
  }

  return retStat;
}

rtError rtStorage::verifyQuota(const char* key, const rtValue& value) const
{
  if (!key || *key == 0)
    return RT_ERROR_INVALID_ARG;

  uint32_t sizeForKey = 0;
  uint32_t sizeTotal = 0;
  rtError retStat = getCurrentSize(key, sizeForKey, sizeTotal);

  if (RT_OK == retStat)
  {
    rtString keyStr = key;
    rtString valueStr = value.toString();
    uint32_t newSize = sizeTotal - sizeForKey + valueStr.byteLength();
    if (sizeForKey == 0)
      newSize += keyStr.byteLength();

    if (newSize > mQuota)
    {
      rtLogWarn("quota exceeded: %d", newSize);
      retStat = RT_ERROR_NOT_ALLOWED;
    }
    else
    {
      rtLogDebug("quota is not exceeded: %d", newSize);
    }
  }
  else
  {
    rtLogError("failed to verify size");
  }

  return retStat;
}

// Clear all data
rtError rtStorage::clear()
{
  rtLogInfo("%s", __FUNCTION__);

  sqlite3* &db = SQLITE;

  if (db)
  {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM ItemTable;", -1, &stmt, NULL);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
      rtLogError("ERROR inserting data: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
  }

  updateSize();

  return RT_OK;
}

rtError rtStorage::runVacuumCommand()
{
  sqlite3* &db = SQLITE;

  if (db)
  {
    sqlite3_exec(db, "VACUUM", 0, 0, 0);
  }

  return RT_OK;
}

bool rtStorage::isEncrypted(const char* fileName)
{
  FILE* fd = fopen(fileName, "rb");
  if (NULL == fd)
    return false;

  int magicSize = strlen(SQLITE_FILE_HEADER);
  char* fileHeader = (char*)malloc(magicSize + 1/*for Coverity*/);

  int readSize = fread(fileHeader, 1, magicSize, fd);
  fclose(fd);

  bool eq = magicSize == readSize && ::memcmp(fileHeader, SQLITE_FILE_HEADER, magicSize) == 0;
  free(fileHeader);

  return !eq;
}

rtDefineObject(rtStorage,rtObject);
rtDefineMethod(rtStorage,setItem);
rtDefineMethod(rtStorage,getItem);
rtDefineMethod(rtStorage,getItems);
rtDefineMethod(rtStorage,removeItem);
rtDefineMethod(rtStorage,clear);
