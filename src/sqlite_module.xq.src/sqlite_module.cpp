/*
 * Copyright 2012 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <sstream>
#include <string>
#include <memory>

#include <sqlite3.h>

#include <zorba/diagnostic_list.h>
#include <zorba/empty_sequence.h>
#include <zorba/item_factory.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/user_exception.h>
#include <zorba/util/base64_stream.h>
#include <zorba/util/base64_util.h>
#include <zorba/util/transcode_stream.h>
#include <zorba/util/uuid.h>

#include "sqlite_module/config.h"
#include "sqlite_module.h"

namespace zorba { namespace sqlite {

// Allocating global variables
zorba::Item SqliteModule::globalNameKey;
zorba::Item SqliteModule::globalDatabaseKey;
zorba::Item SqliteModule::globalTableKey;
zorba::Item SqliteModule::globalTypeKey;
zorba::Item SqliteModule::globalCollationKey;
zorba::Item SqliteModule::globalNullableKey;
zorba::Item SqliteModule::globalPrimaryKey;
zorba::Item SqliteModule::globalAutoincKey;
zorba::Item SqliteModule::globalAffectedRowsKey;

/*******************************************************************************
 ******************************************************************************/
SqliteModule::SqliteModule()
{
  globalNameKey = Zorba::getInstance(0)->getItemFactory()->createString("name");
  globalDatabaseKey = Zorba::getInstance(0)->getItemFactory()->createString("database");
  globalTableKey = Zorba::getInstance(0)->getItemFactory()->createString("table");
  globalTypeKey = Zorba::getInstance(0)->getItemFactory()->createString("type");
  globalCollationKey = Zorba::getInstance(0)->getItemFactory()->createString("collation");
  globalNullableKey =  Zorba::getInstance(0)->getItemFactory()->createString("nullable");
  globalPrimaryKey = Zorba::getInstance(0)->getItemFactory()->createString("primary key");
  globalAutoincKey = Zorba::getInstance(0)->getItemFactory()->createString("autoincrement");
  globalAffectedRowsKey = Zorba::getInstance(0)->getItemFactory()->createString("Affected Rows");
}

zorba::ExternalFunction*
SqliteModule::getExternalFunction(const zorba::String& localName)
{
  FuncMap_t::iterator lIte = theFunctions.find(localName);

  ExternalFunction*& lFunc = theFunctions[localName];

  if (lIte == theFunctions.end())
  {
    if (localName == "connect")
    {
      lFunc = new ConnectFunction(this);
    }
    else if (localName == "is-connected")
    {
      lFunc = new IsConnectedFunction(this);
    }
    else if (localName == "commit")
    {
      lFunc = new CommitFunction(this);
    }
    else if (localName == "rollback")
    {
      lFunc = new RollbackFunction(this);
    }
    else if (localName == "execute-query")
    {
      lFunc = new ExecuteQueryFunction(this);
    }
    else if (localName == "execute-update")
    {
      lFunc = new ExecuteUpdateFunction(this);
    }
    else if (localName == "metadata")
      {
        lFunc = new MetadataFunction(this);
      }
      else if (localName == "prepare-statement")
      {
        lFunc = new PrepareStatementFunction(this);
      }
      else if (localName == "set-value")
      {
        lFunc = new SetValueFunction(this);
      }
      else if (localName == "set-boolean")
      {
        lFunc = new SetBooleanFunction(this);
      }
      else if (localName == "set-numeric")
      {
        lFunc = new SetNumericFunction(this);
      }
      else if (localName == "set-string")
      {
        lFunc = new SetStringFunction(this);
      }
      else if (localName == "set-null")
      {
        lFunc = new SetNullFunction(this);
      }
      else if (localName == "clear-params")
      {
        lFunc = new ClearParamsFunction(this);
      }
      else if (localName == "close-prepared")
      {
        lFunc = new ClosePreparedFunction(this);
      }
      else if (localName == "execute-query-prepared")
      {
        lFunc = new ExecuteQueryPreparedFunction(this);
      }
      else if (localName == "execute-update-prepared")
      {
        lFunc = new ExecuteUpdatePreparedFunction(this);
      }
    }

    return lFunc;
  }

  void SqliteModule::destroy()
  {
    delete this;
  }

  SqliteModule::~SqliteModule()
  {
    for (FuncMap_t::const_iterator lIter = theFunctions.begin();
      lIter != theFunctions.end(); ++lIter)
    {
      delete lIter->second;
    }
    theFunctions.clear();
  }

  zorba::Item&
  SqliteModule::getGlobalKey(GLOBAL_KEYS g)
  {
      switch(g)
      {
      case NAME: return globalNameKey;
      case DATABASE: return globalDatabaseKey;
      case TABLE: return globalTableKey;
      case TYPE: return globalTypeKey;
      case COLLATION: return globalCollationKey;
      case NULLABLE: return globalNullableKey;
      case PRIMARY_KEY: return globalPrimaryKey;
      case AUTOINC: return globalAutoincKey;
      case AFFECTED_ROWS: return globalAffectedRowsKey;
      // Should never touch this case but still ...
      default: return globalNameKey;
      }
  }

  /***********************
   *       ConnMap       *
   ***********************/

  ConnMap::ConnMap(StmtMap* stmtMap)
  {
    ConnMap::connMap = new ConnMap_t();
    sMap = stmtMap;
  }

  bool 
  ConnMap::storeConn(const std::string& aKeyName, sqlite3* sql)
  {
    std::pair<ConnMap_t::iterator,bool> ret;
    ret = connMap->insert(std::pair<std::string, sqlite3 *>(aKeyName, sql));
    return ret.second;
  }

  sqlite3*
  ConnMap::getConn(const std::string& aKeyName)
  {
    ConnMap_t::iterator lIter = connMap->find(aKeyName);

    if(lIter == connMap->end())
      return NULL;
    
    sqlite3 *lSql = lIter->second;

    return lSql;
  }

  bool
  ConnMap::deleteConn(const std::string& aKeyName)
  {
    ConnMap::ConnMap_t::iterator lIter = connMap->find(aKeyName);

    if(lIter == connMap->end())
      return false;
      
    if(sMap != NULL)
      sMap->deleteAllForConn(lIter->second);
    sqlite3_close(lIter->second);
    connMap->erase(lIter);
    return true;
  }

  void
  ConnMap::destroy() throw()
  {
    if(connMap)
    {
      if(sMap)
        sMap->deleteAllForConn(NULL); // delete all prep-statements
        
      for (ConnMap_t::iterator lIter = connMap->begin();
           lIter != connMap->end(); )
      {
        sqlite3_close(lIter->second);
        connMap->erase(lIter++);
      }
 //     connMap->clear();
      delete connMap;
    }
    delete this;
  }
  
  ConnMap::~ConnMap(){ }

/***********************
 *       StmtMap       *
***********************/

  StmtMap::StmtMap()
  {
    StmtMap::stmtMap = new StmtMap_t();
  }

  bool 
  StmtMap::storeStmt(const std::string& aKeyName, sqlite3_stmt* stmt)
  {
    std::pair<StmtMap_t::iterator,bool> ret;
    ret = stmtMap->insert(std::pair<std::string, sqlite3_stmt *>(aKeyName, stmt));
    return ret.second;
  }

  sqlite3_stmt*
  StmtMap::getStmt(const std::string& aKeyName)
  {
    StmtMap_t::iterator lIter = stmtMap->find(aKeyName);

    if(lIter == stmtMap->end())
      return NULL;
    
    sqlite3_stmt *lStmt = lIter->second;

    return lStmt;
  }

  bool
  StmtMap::deleteStmt(const std::string& aKeyName)
  {
    StmtMap::StmtMap_t::iterator lIter = stmtMap->find(aKeyName);

    if(lIter == stmtMap->end())
      return false;

    sqlite3_finalize(lIter->second);
    stmtMap->erase(lIter);
    return true;
  }

  void
  StmtMap::destroy() throw()
  {
    if(stmtMap)
    {
      for (StmtMap_t::iterator lIter = stmtMap->begin();
           lIter != stmtMap->end(); )
      {
        sqlite3_finalize(lIter->second);
        stmtMap->erase(lIter++);
      }
      delete stmtMap;
    }
    delete this;
  }
  
  void
  StmtMap::deleteAllForConn(sqlite3* c)
  {
    StmtMap_t::iterator it;
    for(it = stmtMap->begin();
        it != stmtMap->end(); )
    {
      if((c == NULL) || (sqlite3_db_handle(it->second) == c))
      {
        sqlite3_finalize(it->second);
        stmtMap->erase(it++);
      } else
        it++;
    }
  }

  StmtMap::~StmtMap(){ }

/*******************************************************************************
 *                              SqliteFunction                                 *
 *******************************************************************************/
  SqliteFunction::SqliteFunction(const SqliteModule* aModule)
    : theModule(aModule) {}

  SqliteFunction::~SqliteFunction(){}

  ConnMap*
  SqliteFunction::getConnectionMap(const zorba::DynamicContext* aDctx){
    DynamicContext* lDynCtx = const_cast<DynamicContext*>(aDctx);
    ConnMap* lConnMap;
    StmtMap* lStmtMap;
    if(!(lConnMap = dynamic_cast<ConnMap*>(lDynCtx->getExternalFunctionParameter("sqliteConnMap"))))
    {
      lStmtMap = getStatementMap(lDynCtx);
      lConnMap = new ConnMap(lStmtMap);
      lDynCtx->addExternalFunctionParameter("sqliteConnMap", lConnMap);     
    }
    return lConnMap;
  }

  StmtMap*
  SqliteFunction::getStatementMap(const zorba::DynamicContext* aDctx){
    DynamicContext* lDynCtx = const_cast<DynamicContext*>(aDctx);
    StmtMap* lStmtMap;
    if(!(lStmtMap = dynamic_cast<StmtMap*>(lDynCtx->getExternalFunctionParameter("sqliteStmtMap"))))
    {
      lStmtMap = new StmtMap();
      lDynCtx->addExternalFunctionParameter("sqliteStmtMap", lStmtMap);     
    }
    return lStmtMap;
  }

  std::string
  SqliteFunction::createUUID(){
    uuid lUUID;
    uuid::create(&lUUID);
    std::stringstream lStream;
    lStream << lUUID;
    return lStream.str();
  }

  sqlite3_stmt* 
  SqliteFunction::createPreparedStatement(const zorba::DynamicContext* aDctx,
                                          std::string aUUID, std::string aQry){
    sqlite3 *lDb;
    sqlite3_stmt *lPstmt;
    int lRc;
    const char *lTail;
    ConnMap* lConnMap = SqliteFunction::getConnectionMap(aDctx);

    lDb = lConnMap->getConn(aUUID);
    if(lDb == NULL){
      // throw error, ID not recognized
      throwError("INVALID-SQLITE-OBJECT", getErrorMessage("INVALID-SQLITE-OBJECT"));
    }

    lRc = sqlite3_prepare_v2(lDb, aQry.c_str(), aQry.size(), &lPstmt, &lTail);
    if(lRc != 0 && lPstmt != NULL){
      sqlite3_finalize(lPstmt);
    }
    if(lRc == SQLITE_ERROR) {
      std::string lErr = getErrorMessage("INVALID-SQL-STATEMENT");
      lErr += "; ";
      lErr += sqlite3_errmsg(lDb);
      throwError("INVALID-SQL-STATEMENT", lErr.c_str());
    } else
      checkForError(lRc, 0, lDb);

    return lPstmt;
  }

  void
  SqliteFunction::setValueToStatement(const zorba::DynamicContext* aDctx,
    std::string aUUID, 
    int aPos, bool aVal)
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    int lRc;
    
    // Get the prepared statement and then set the value
    lPstmt = stmtMap->getStmt(aUUID);
    if(lPstmt == NULL){
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));
    }
    lRc = sqlite3_bind_int(lPstmt, aPos, (aVal==true)?1:0);
    if(lRc == SQLITE_RANGE)
      throwError("INVALID-PLACEHOLDER-POSITION",
                 getErrorMessage("INVALID-PLACEHOLDER-POSITION"));
    else
      checkForError(lRc, 0, sqlite3_db_handle(lPstmt));
  }

  void
  SqliteFunction::setValueToStatement(const zorba::DynamicContext* aDctx,
    std::string aUUID, 
    int aPos, 
    int aVal)
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    int lRc;
    
    // Get the prepared statement and then set the value
    lPstmt = stmtMap->getStmt(aUUID);
    if(lPstmt == NULL){
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));
    }
    lRc = sqlite3_bind_int(lPstmt, aPos, aVal);
    if(lRc == SQLITE_RANGE)
      throwError("INVALID-PLACEHOLDER-POSITION",
                 getErrorMessage("INVALID-PLACEHOLDER-POSITION"));
    else
      checkForError(lRc, 0, sqlite3_db_handle(lPstmt));
  }

  void
  SqliteFunction::setValueToStatement(const zorba::DynamicContext* aDctx,
    std::string aUUID,
    int aPos,
    double aVal)
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);\
    int lRc;
    
    // Get the prepared statement and then set the value
    lPstmt = stmtMap->getStmt(aUUID);
    if(lPstmt == NULL){
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));
    }
    lRc = sqlite3_bind_double(lPstmt, aPos, aVal);
    if(lRc == SQLITE_RANGE)
      throwError("INVALID-PLACEHOLDER-POSITION",
                 getErrorMessage("INVALID-PLACEHOLDER-POSITION"));
    else
      checkForError(lRc, 0, sqlite3_db_handle(lPstmt));
  }

  void
  SqliteFunction::setValueToStatement(const zorba::DynamicContext* aDctx,
    std::string aUUID,
    int aPos,
    std::string aVal)
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    int lRc;
    
    // Get the prepared statement and then set the value
    lPstmt = stmtMap->getStmt(aUUID);
    if(lPstmt == NULL){
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));
    }
    lRc = sqlite3_bind_text(lPstmt, aPos, aVal.c_str(), aVal.size(), SQLITE_TRANSIENT);
    if(lRc == SQLITE_RANGE)
      throwError("INVALID-PLACEHOLDER-POSITION",
                 getErrorMessage("INVALID-PLACEHOLDER-POSITION"));
    else
      checkForError(lRc, 0, sqlite3_db_handle(lPstmt));
  }

  void
  SqliteFunction::setValueToStatement(const zorba::DynamicContext* aDctx,
    std::string aUUID,
    int aPos)
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    int lRc;
    
    // Get the prepared statement and then set the value
    lPstmt = stmtMap->getStmt(aUUID);
    if(lPstmt == NULL){
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));
    }
    lRc = sqlite3_bind_null(lPstmt, aPos);
    if(lRc == SQLITE_RANGE)
      throwError("INVALID-PLACEHOLDER-POSITION",
                 getErrorMessage("INVALID-PLACEHOLDER-POSITION"));
    else
      checkForError(lRc, 0, sqlite3_db_handle(lPstmt));
  }

  void
  SqliteFunction::clearValues(const zorba::DynamicContext* aDctx,
    std::string aUUID)
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    	
    // get the prepared statement if exists
    lPstmt = stmtMap->getStmt(aUUID);
    if(lPstmt == NULL){
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));
    }
    sqlite3_clear_bindings(lPstmt);
  }

  String 
  SqliteFunction::getURI() const
  {
    return theModule->getURI();
  }

  void
  SqliteFunction::throwError(
        const char* aLocalName,
        const char* aErrorMessage)
  {
    String errNS(SqliteModule::getModuleURI());
    Item errQName = SqliteModule::getItemFactory()->createQName(
        errNS, aLocalName);
    throw USER_EXCEPTION(errQName, aErrorMessage);
  }

  void
  SqliteFunction::checkForError(
        int aErrNo,
        const char* aLocalName,
        sqlite3 *sql)
  {
    if (aErrNo != SQLITE_OK)
    {
      if (!aLocalName)
      {
        throwError("INTERNAL-SQLITE-PROBLEM", sqlite3_errmsg(sql));
      }
      else
      {
        throwError(aLocalName, sqlite3_errmsg(sql));
      }
    }
  }

  zorba::Item
  SqliteFunction::getOneItem(const Arguments_t& aArgs, int aIndex)
  {
    Item lItem;
    Iterator_t args_iter = aArgs[aIndex]->getIterator();
    args_iter->open();
    args_iter->next(lItem);
    args_iter->close();

    return lItem;
  }

  int
  SqliteFunction::strToInt(std::string str){
    int iInt;
    sscanf(str.c_str(), "%d", &iInt);
    return iInt;
  }

  double
  SqliteFunction::strToDouble(std::string str){
    double dDbl;
    sscanf(str.c_str(), "%lf", &dDbl);
    return dDbl;
  }

  const char *
  SqliteFunction::getErrorMessage(std::string error){
    if(error == "CANT-OPEN-DB")
    {
      return "Database file does not exist or it is not possible to open it";
    } 
    else if(error == "INVALID-SQLITE-OBJECT")
    {
      return "Connection ID passed is not valid";
    }
    else if(error == "INVALID-SQL-STATEMENT")
    {
      return "Statement passed is not a valid SQL statement";
    }
    else if(error == "INVALID-PREPARED-STATEMENT")
    {
      return "Prepared statement passed is not valid";
    }
    else if(error == "INVALID-PLACEHOLDER-POSITION")
    {
      return "Parameter position passed is not valid";
    }
    else if(error == "INVALID-VALUE")
    {
      return "Parameter passed is not a valid number";
    }
    else if(error == "UNKNOWN-OPTION")
    {
      return "Parameter passed is not a valid value";
    }
#ifndef SQLITE_WITH_FILE_ACCESS
    else if(error == "COMPILED-WITHOUT-DISK-ACCESS")
    {
      return "Only in-memory databases are allowed (Module built without filesystem access)";
    }
#endif /* not SQLITE_WITH_FILE_ACCESS */
#ifndef ZORBA_SQLITE_HAVE_METADATA
    else if(error == "UNAVAILABLE-METADATA")
    {
      return "Metadata not found (SQLite built without SQLITE_ENABLE_COLUMN_METADATA)";
    }
#endif /* not ZORBA_SQLITE_HAVE_METADATA */
    else if(error == "INTERNAL-SQLITE-PROBLEM")
    {
      return "Internal error ocurred";
    }
    return "";
  }

  /********************
   *  Sqlite Options  *
   ********************/

  SqliteOptions::SqliteOptions()
    : theOpenReadOnly(false),
      theOpenCreate(true),
      theOpenNoMutex(false),
      theOpenSharedCache(false) {}

  void
  SqliteOptions::setValues(sqlite3* aSqlite)
  {
    // TODO: check if it is possible to get those values from the sqlite3 pointer
  }

  void
  SqliteOptions::setValues(Item& aOptions)
  {
    Item lItemJSONKey;

    Iterator_t lIterKeys = aOptions.getObjectKeys();
    lIterKeys->open();
    while (lIterKeys->next(lItemJSONKey))
    {
      Item lOptionValue;
      lOptionValue = aOptions.getObjectValue(lItemJSONKey.getStringValue());

      if (lItemJSONKey.getStringValue() == "open-read-only")
      {
        theOpenReadOnly = lOptionValue.getBooleanValue() == true;
      }
      else if (lItemJSONKey.getStringValue() == "open-create")
      {
        theOpenCreate = lOptionValue.getBooleanValue();
      }
      else if (lItemJSONKey.getStringValue() == "open-no-mutex")
      {
        theOpenNoMutex = lOptionValue.getBooleanValue();
      }
      else if(lItemJSONKey.getStringValue() == "open-shared-cache")
      {
        theOpenSharedCache = lOptionValue.getBooleanValue();
      } else
        // Not sure if I should stop here in case that any option
        // are not in the list
        SqliteFunction::throwError("UNKNOWN-OPTION",
                                   (std::string(SqliteFunction::getErrorMessage("UNKNOWN-OPTION")) + " - " +
                                    lItemJSONKey.getStringValue().str()).c_str());
    }
    lIterKeys->close();
  }

  int
  SqliteOptions::getOptionsAsInt(){
    int opts = 0;
    if(theOpenCreate)
      opts |= SQLITE_OPEN_CREATE;
    if(theOpenReadOnly)
      opts |= SQLITE_OPEN_READONLY;
    else
      opts |= SQLITE_OPEN_READWRITE;
    if(theOpenNoMutex)
      opts |= SQLITE_OPEN_NOMUTEX;
    if(theOpenSharedCache)
      opts |= SQLITE_OPEN_SHAREDCACHE;
    return opts;
  }
  
  std::string
  SqliteOptions::getOptionsAsString(){
    std::string res;
    bool somethingthere = false;
    if(theOpenCreate)
    {
      res += "SQLITE_OPEN_CREATE";
      somethingthere = true;
    }
    if(theOpenReadOnly)
    {
      if(somethingthere)
      {
        res += " | ";
      }
      res += "SQLITE_OPEN_READONLY";
      somethingthere = true;
    }
    else
    {
      if(somethingthere)
      {
        res += " | ";
      }
      res += "SQLITE_OPEN_READWRITE";
      somethingthere = true;
    }
    if(theOpenNoMutex)
    {
      if(somethingthere)
      {
        res += " | ";
      }
      res += "SQLITE_OPEN_NOMUTEX";
      somethingthere = true;
    }
    if(theOpenSharedCache)
    {
      if(somethingthere)
      {
        res += " | ";
      }
      res += "SQLITE_OPEN_SHAREDCACHE";
    }
    return res;
  }

/*******************************************************************************
 *                         JSONItemSequence::JSONIterator                      *
 ******************************************************************************/
  void JSONItemSequence::JSONIterator::open(){
    zorba::Item lColumnName;
    char* lColumnNameChar;
    // Get data and create the column names
    if(theStmt != NULL){
      theRc = sqlite3_step(theStmt);
      SqliteFunction::checkForError((theRc==SQLITE_ROW || theRc==SQLITE_DONE)?0:-1, 0,
                                    sqlite3_db_handle(theStmt));
      if(theRc == SQLITE_DONE)
        isUpdateResult = true;
      theFactory = Zorba::getInstance(0)->getItemFactory();

      theColumnCount = sqlite3_column_count(theStmt);
      if(theColumnCount > 0)
      {
        for(int i=0; i<theColumnCount; i++)
        {
          const char* lpChar = sqlite3_column_name(theStmt, i);
          int lLen = strlen(lpChar);
          lColumnNameChar = new char[lLen+1];
          memcpy(lColumnNameChar, lpChar, lLen);
          lColumnNameChar[lLen] = '\0';
          lColumnName = theFactory->createString(lColumnNameChar);
          theColumnNamesZString.push_back(lColumnName);
          delete lColumnNameChar;
        }
      }
    }
  }

  bool JSONItemSequence::JSONIterator::next(zorba::Item& aItem){
    int aType, aSize;
    zorba::Item aValue;
    std::vector<std::pair<zorba::Item, zorba::Item> > elements;
    const char *aBlobPtr;

    if(theRc == SQLITE_ROW){
      // get the resulting data from the statement
      // in a key = value fashion
      for(int i=0; i<theColumnCount; i++){
        aType = sqlite3_column_type(theStmt, i);
        switch(aType){
        case SQLITE_NULL:
          aValue = theFactory->createJSONNull();
          break;
        case SQLITE_INTEGER: 
          aValue = theFactory->createInt(sqlite3_column_int(theStmt, i));
          break;
        case SQLITE_FLOAT:
          aValue = theFactory->createDouble(sqlite3_column_double(theStmt, i));
          break;
        case SQLITE_BLOB:
          aSize = sqlite3_column_bytes(theStmt, i);
          aBlobPtr = (const char *)sqlite3_column_blob(theStmt, i);
          aValue = theFactory->createBase64Binary(aBlobPtr, aSize, true);
          break;
        default:
          std::string str = std::string((const char *)sqlite3_column_text(theStmt, i));
          aValue = theFactory->createString(
            zorba::String(str)
          );
        }
        elements.push_back(std::pair<zorba::Item, zorba::Item>(theColumnNamesZString.at(i), aValue));
      }
      aItem = theFactory->createJSONObject(elements);
      elements.clear();
      // Get more data if available
      theRc = sqlite3_step(theStmt);
      return true;
    } else if(isUpdateResult && theRc == SQLITE_DONE){
      // we have a prepared statement that represents a UPDATE and it's already executed
      aValue = theFactory->createInt(sqlite3_changes(sqlite3_db_handle(theStmt)));
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::AFFECTED_ROWS), aValue));
      aItem = theFactory->createJSONObject(elements);
      elements.clear();
      // be sure it won't be back in here
      theRc = SQLITE_ERROR;
      return true;
    } else
      // There is no data to return anymore
      return false;
  }

  void JSONItemSequence::JSONIterator::close(){
    // Set the Rc to "no more data" and clear the variables
    theRc = SQLITE_ERROR;
    if(theColumnCount > 0)
    {
      theColumnNamesZString.clear();
    }
    theColumnCount = 0;
    if(theStmt != NULL)
      sqlite3_reset(theStmt);
  }

/*******************************************************************************
 *              JSONMetadataItemSequence::JSONMetadataIterator                 *
 ******************************************************************************/
#ifdef ZORBA_SQLITE_HAVE_METADATA
  void JSONMetadataItemSequence::JSONMetadataIterator::open(){
    // Get data and create the column names
    if(theStmt != NULL){
      theRc = sqlite3_step(theStmt);
      SqliteFunction::checkForError((theRc==SQLITE_ROW || theRc==SQLITE_DONE)?0:-1, 0,
                                    sqlite3_db_handle(theStmt));
      theFactory = Zorba::getInstance(0)->getItemFactory();

      theColumnCount = sqlite3_column_count(theStmt);
      if(theColumnCount > 0)
      {
        theActualColumn = 0;
        theRc = SQLITE_ROW;
      } else
        theRc = SQLITE_ERROR;
    } else
      theRc = SQLITE_ERROR;
  }

  bool JSONMetadataItemSequence::JSONMetadataIterator::next(zorba::Item& aItem){
    //zorba::Item aKey;
    zorba::Item aValue;
    std::vector<std::pair<zorba::Item, zorba::Item> > elements;
    sqlite3 *lDbHandle;
    const char *lDbName;
    const char *lTableName;
    const char *lDataType;
    const char *lCollSequence;
    const char *lOriginName;
    int lNotNull, lPrimaryKey, lAutoinc, lRc;

    if(theRc == SQLITE_ROW){
      // Get the metadata for 'theActualColumn' column
      // in a key = value fashion
      lDbHandle = sqlite3_db_handle(theStmt);
      lDbName = sqlite3_column_database_name(theStmt, theActualColumn);
      lTableName = sqlite3_column_table_name(theStmt, theActualColumn);
      lOriginName = sqlite3_column_origin_name(theStmt, theActualColumn);
      lRc = sqlite3_table_column_metadata(lDbHandle,
                                    lDbName,
                                    lTableName,
                                    lOriginName,
                                    &lDataType,
                                    &lCollSequence,
                                    &lNotNull,
                                    &lPrimaryKey,
                                    &lAutoinc);
      if(lRc != 0)
        SqliteFunction::throwError(0, sqlite3_errmsg(lDbHandle));
      aValue = theFactory->createString(lOriginName);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::NAME), aValue));
      aValue = theFactory->createString(lDbName);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::DATABASE), aValue));
      aValue = theFactory->createString(lTableName);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::TABLE), aValue));
      aValue = theFactory->createString(lDataType);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::TYPE), aValue));
      aValue = theFactory->createString(lCollSequence);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::COLLATION), aValue));
      aValue = theFactory->createBoolean((lNotNull==0)?false:true);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::NULLABLE), aValue));
      aValue = theFactory->createBoolean((lPrimaryKey==0)?false:true);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::PRIMARY_KEY), aValue));
      aValue = theFactory->createBoolean((lAutoinc==0)?false:true);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(SqliteModule::getGlobalKey(SqliteModule::AUTOINC), aValue));
      aItem = theFactory->createJSONObject(elements);
      elements.clear();
      // Get more data if available
      theActualColumn++;
      if(theActualColumn >= theColumnCount)
        theRc = SQLITE_ERROR;
      else
        theRc = SQLITE_ROW;
      return true;
    } else
      // There is no data to return anymore
      return false;
  }

  void JSONMetadataItemSequence::JSONMetadataIterator::close(){
    // Set the Rc to "no more data" and clear the variables
    theRc = SQLITE_ERROR;
    theColumnCount = 0;
    if(theStmt != NULL)
      sqlite3_reset(theStmt);
  }
#endif

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ConnectFunction::evaluate(
      const Arguments_t& aArgs,
      const zorba::StaticContext* aSctx,
      const zorba::DynamicContext* aDctx) const 
  {
    sqlite3 *lSqldb = NULL;
    int lRc;
    ConnMap* lConnMap = getConnectionMap(aDctx);
    Item lItemName = getOneItem(aArgs, 0);
    Item lItemOpts;
    std::string lStrUUID;
    std::string lDbName;
    SqliteOptions lOptions;

    if(aArgs.size() == 2){
      // add code to include options
      lItemOpts = getOneItem(aArgs, 1);
      lOptions.setValues(lItemOpts);
    }

    // Connect to the specified location with the specified options
    lDbName = lItemName.getStringValue().str();
    if(lDbName == "")
      lDbName = std::string(":memory:");

#ifndef SQLITE_WITH_FILE_ACCESS
    if (lDbName != ":memory:") {
      throwError("COMPILED-WITHOUT-DISK-ACCESS",
                 getErrorMessage("COMPILED-WITHOUT-DISK-ACCESS"));
    }
#endif /* not SQLITE_WITH_FILE_ACCESS */
    lRc = sqlite3_open_v2(lDbName.c_str(), &lSqldb, lOptions.getOptionsAsInt(), NULL);
    // Store the UUID for this connection and return it
    lStrUUID = createUUID();
    lConnMap->storeConn(lStrUUID, lSqldb);
    if(lRc == SQLITE_CANTOPEN)
      throwError("CANT-OPEN-DB", getErrorMessage("CANT-OPEN-DB"));
    else
      checkForError(lRc, 0, lSqldb);

    return ItemSequence_t(new SingletonItemSequence(SqliteModule::getItemFactory()->createAnyURI(lStrUUID)));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    IsConnectedFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3 *lSqldb;
    ConnMap* lConnMap = getConnectionMap(aDctx);
    Item lItemRes;
    Item lItemUUID = getOneItem(aArgs, 0);

    // Look for the UUID into the Connection Map
    // if it is in there means that you have it connected
    lSqldb = lConnMap->getConn(lItemUUID.getStringValue().str());
    if(lSqldb == NULL){
      lItemRes = SqliteModule::getItemFactory()->createBoolean(false);
    } else {
      lItemRes = SqliteModule::getItemFactory()->createBoolean(true);
    }

    return ItemSequence_t(new SingletonItemSequence(lItemRes));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    CommitFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3 *lDb;
    Item lItemUUID = getOneItem(aArgs, 0);
    ConnMap* lConnMap = getConnectionMap(aDctx);

    lDb = lConnMap->getConn(lItemUUID.getStringValue().str());
    if(lDb == NULL)
      throwError("INVALID-SQLITE-OBJECT", getErrorMessage("INVALID-SQLITE-OBJECT"));

    return ItemSequence_t(new SingletonItemSequence(lItemUUID));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    RollbackFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3 *lDb;
    Item lItemUUID = getOneItem(aArgs, 0);
    ConnMap* lConnMap = getConnectionMap(aDctx);

    lDb = lConnMap->getConn(lItemUUID.getStringValue().str());
    if(lDb == NULL)
      throwError("INVALID-SQLITE-OBJECT", getErrorMessage("INVALID-SQLITE-OBJECT"));

    return ItemSequence_t(new SingletonItemSequence(lItemUUID));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteQueryFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  { 
    sqlite3_stmt *lPstmt;
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemQry = getOneItem(aArgs, 1);
    StmtMap *stmtMap = getStatementMap(aDctx);
    std::string lStrUUID;

    // Create the prepared statement with the UUID and Query passed
    lPstmt = createPreparedStatement(aDctx, lItemUUID.getStringValue().str(),
      lItemQry.getStringValue().str());
    lStrUUID = createUUID();
    stmtMap->storeStmt(lStrUUID, lPstmt);

    // Once we got the SQL Query executed just pass it to the JSON Sequence
    // so it will return what we need to the user
    std::auto_ptr<JSONItemSequence> lSeq(new JSONItemSequence(lPstmt));
    return ItemSequence_t(lSeq.release());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteUpdateFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3_stmt *lPstmt;
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemQry = getOneItem(aArgs, 1);
    Item lItemRes;
    Item lItemJSONKey;
    StmtMap *stmtMap = getStatementMap(aDctx);
    std::string lStrUUID;

    // Create the prepared statement with the UUID and Query passed
    lPstmt = createPreparedStatement(aDctx, lItemUUID.getStringValue().str(),
      lItemQry.getStringValue().str());
    lStrUUID = createUUID();
    stmtMap->storeStmt(lStrUUID, lPstmt);

    // Once we got the SQL Query executed just pass it to the JSON Sequence
    // after we get the result we convert it to a integer Item
    std::auto_ptr<JSONItemSequence> lSeq(new JSONItemSequence(lPstmt));
    Iterator_t lIter = lSeq->getIterator();
    lIter->open();
    lIter->next(lItemRes);
    lIter->close();
    Iterator_t lIterKeys = lItemRes.getObjectKeys();
    lIterKeys->open();
    lIterKeys->next(lItemJSONKey);
    lIterKeys->close();
    Item lItemValue = lItemRes.getObjectValue(lItemJSONKey.getStringValue());
    return ItemSequence_t(new SingletonItemSequence(lItemValue));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    MetadataFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
#ifdef ZORBA_SQLITE_HAVE_METADATA
    sqlite3_stmt *lPstmt;
    zorba::Item lItemPstmt = getOneItem(aArgs, 0);
    zorba::Item lVecItem, lJSONKey, lJSONArray, lJSONRes;
    StmtMap *lStmtMap = getStatementMap(aDctx);
    std::vector<zorba::Item> lItems;
    std::vector<std::pair<zorba::Item, zorba::Item> > lVectorRes;
    ItemFactory* lFactory;
    Iterator_t lIter;

    lFactory = SqliteModule::getItemFactory();
    // Get the prepared statement
    lPstmt = lStmtMap->getStmt(lItemPstmt.getStringValue().str());
    if(lPstmt == NULL){
      // No valid prepared statement id passed
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));
    }

    // So now create a JSONMetadataItemSequence and let it
    // get us what we need
    std::auto_ptr<JSONMetadataItemSequence> lSeq(new JSONMetadataItemSequence(lPstmt));
    lIter = lSeq->getIterator();
    lIter->open();
    while(lIter->next(lVecItem))
    {
      lItems.push_back(lVecItem);
    }
    lIter->close();
    lJSONArray = lFactory->createJSONArray(lItems);
    lJSONKey = lFactory->createString(std::string("columns"));
    lVectorRes.push_back(std::pair<Item, Item>(lJSONKey, lJSONArray));
    lJSONRes = lFactory->createJSONObject(lVectorRes);
    
    return ItemSequence_t(new SingletonItemSequence(lJSONRes));
#else
    throwError("UNAVAILABLE-METADATA", getErrorMessage("UNAVAILABLE-METADATA"));
#endif
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    PrepareStatementFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3_stmt *lPstmt;
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemQry = getOneItem(aArgs, 1);
    std::string lStrUUID;
    StmtMap *stmtMap = getStatementMap(aDctx);

    // Create the prepared statement
    lPstmt = createPreparedStatement(aDctx, lItemUUID.getStringValue().str(),
      lItemQry.getStringValue().str());

    // Create a new UUID for the prepared statement
    lStrUUID = SqliteFunction::createUUID();
    stmtMap->storeStmt(lStrUUID, lPstmt);

    return ItemSequence_t(new SingletonItemSequence(SqliteModule::getItemFactory()->createAnyURI(lStrUUID)));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    SetValueFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemPos = getOneItem(aArgs, 1);
    Item lItem = getOneItem(aArgs, 2);
    int lPos;

    lPos = strToInt(lItemPos.getStringValue().str());
    switch(lItem.getTypeCode()){
    case store::XS_BOOLEAN:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, lItem.getBooleanValue());
      break;
    case store::XS_BYTE:
    case store::XS_INT:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, lItem.getIntValue());
      break;
    case store::XS_INTEGER:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, strToInt(lItem.getStringValue().str()));
      break;
    case store::XS_FLOAT:
    case store::XS_DOUBLE:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, lItem.getDoubleValue());
      break;
    case store::XS_DECIMAL:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, strToDouble(lItem.getStringValue().str()));
      break;
    case store::XS_STRING:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, lItem.getStringValue().str());
      break;
    default:
      throwError("INVALID-VALUE", getErrorMessage("INVALID-VALUE"));
    }
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    SetBooleanFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemPos = getOneItem(aArgs, 1);
    Item lItemBool = getOneItem(aArgs, 2);

    setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                        strToInt(lItemPos.getStringValue().str()), lItemBool.getBooleanValue());
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    SetNumericFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemPos = getOneItem(aArgs, 1);
    Item lItemNumeric = getOneItem(aArgs, 2);
    int lPos = strToInt(lItemPos.getStringValue().str());

    switch(lItemNumeric.getTypeCode()){
    case store::XS_BYTE:
    case store::XS_INT:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, lItemNumeric.getIntValue());
      break;
    case store::XS_INTEGER:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, strToInt(lItemNumeric.getStringValue().str()));
      break;
    case store::XS_FLOAT:
    case store::XS_DOUBLE:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, lItemNumeric.getDoubleValue());
      break;
    case store::XS_DECIMAL:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                          lPos, strToDouble(lItemNumeric.getStringValue().str()));
      break;
    default:
      throwError("INVALID-VALUE", getErrorMessage("INVALID-VALUE"));
    }
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    SetStringFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemPos = getOneItem(aArgs, 1);
    Item lItemString = getOneItem(aArgs, 2);
    
    setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                        strToInt(lItemPos.getStringValue().str()), lItemString.getStringValue().str());
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    SetNullFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemPos = getOneItem(aArgs, 1);

    setValueToStatement(aDctx, lItemUUID.getStringValue().str(),
                        strToInt(lItemPos.getStringValue().str()));
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ClearParamsFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    Item lItemUUID = getOneItem(aArgs, 0);

    clearValues(aDctx, lItemUUID.getStringValue().str());
    return ItemSequence_t(new EmptySequence());
  }
  
/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ClosePreparedFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  { 
    sqlite3_stmt *lPstmt;
    Item lItemUUID = getOneItem(aArgs, 0);
    StmtMap* stmtMap;

    // get the prepared statement
    stmtMap = getStatementMap(aDctx);
    lPstmt = stmtMap->getStmt(lItemUUID.getStringValue().str());
    if(lPstmt == NULL)
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));

    // Once we got the prepared statement just get rid of it
    stmtMap->deleteStmt(lItemUUID.getStringValue().str());
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteQueryPreparedFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    Item lItemUUID = getOneItem(aArgs, 0);

    // Get the prepared statement
    lPstmt = stmtMap->getStmt(lItemUUID.getStringValue().str());
    if(lPstmt == NULL)
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));

    // And let the JSONItemSequence execute it
    std::auto_ptr<JSONItemSequence> lSeq(new JSONItemSequence(lPstmt));
    return ItemSequence_t(lSeq.release());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteUpdatePreparedFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemRes, lItemJSONKey;

    // Get the prepared statement
    lPstmt = stmtMap->getStmt(lItemUUID.getStringValue().str());
    if(lPstmt == NULL)
      throwError("INVALID-PREPARED-STATEMENT",
                 getErrorMessage("INVALID-PREPARED-STATEMENT"));

    // And let the JSONItemSequence execute it
    std::auto_ptr<JSONItemSequence> lSeq(new JSONItemSequence(lPstmt));
    Iterator_t lIter = lSeq->getIterator();
    lIter->open();
    lIter->next(lItemRes);
    lIter->close();
    Iterator_t lIterKeys = lItemRes.getObjectKeys();
    lIterKeys->open();
    lIterKeys->next(lItemJSONKey);
    lIterKeys->close();
    Item lItemValue = lItemRes.getObjectValue(lItemJSONKey.getStringValue());
    return ItemSequence_t(new SingletonItemSequence(lItemValue));
  }

} /* namespace zorba */ } /* namespace archive*/

#ifdef WIN32
#  define DLL_EXPORT __declspec(dllexport)
#else
#  define DLL_EXPORT __attribute__ ((visibility("default")))
#endif

extern "C" DLL_EXPORT zorba::ExternalModule* createModule() {
  return new zorba::sqlite::SqliteModule();
}
