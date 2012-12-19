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

#include <zorba/item_factory.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/diagnostic_list.h>
#include <zorba/empty_sequence.h>
#include <zorba/user_exception.h>
#include <zorba/transcode_stream.h>
#include <zorba/base64_stream.h>
#include <zorba/base64.h>
#include <zorba/util/uuid.h>
#include <string>
#include <cassert>

#include <sqlite3.h>
#include <iostream>
#include <stdio.h>

#include "sqlite_module.h"

namespace zorba { namespace sqlite {


/*******************************************************************************
 ******************************************************************************/
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
      else if (localName == "disconnect")
      {
        lFunc = new DisconnectFunction(this);
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
      else if (localName == "execute")
      {
        lFunc = new ExecuteFunction(this);
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
      else if (localName == "execute-prepared")
      {
        lFunc = new ExecutePreparedFunction(this);
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

  /***********************
   *       ConnMap       *
   ***********************/

  ConnMap::ConnMap()
  {
    ConnMap::connMap = new ConnMap_t();
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

    connMap->erase(lIter);
    return true;
  }

  void
  ConnMap::destroy() throw()
  {
    if(connMap)
    {
      for (ConnMap_t::const_iterator lIter = connMap->begin();
           lIter != connMap->end(); ++lIter)
      {
        if(lIter->second != NULL)
          sqlite3_close(lIter->second);
        //deleteConn(lIter->first);
      }
      connMap->clear();
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

    stmtMap->erase(lIter);
    return true;
  }

  void
  StmtMap::destroy() throw()
  {
    if(stmtMap)
    {
      for (StmtMap_t::const_iterator lIter = stmtMap->begin();
           lIter != stmtMap->end(); ++lIter)
      {
        if(lIter->second != NULL)
          sqlite3_finalize(lIter->second);
      }
      stmtMap->clear();
      delete stmtMap;
    }
    delete this;
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
    if(!(lConnMap = dynamic_cast<ConnMap*>(lDynCtx->getExternalFunctionParameter("sqliteConnMap"))))
    {
      lConnMap = new ConnMap();
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
  SqliteFunction::createPreparedStatement(const zorba::DynamicContext* aDctx, std::string aUUID, std::string aQry){
    sqlite3 *lDb;
    sqlite3_stmt *lPstmt;
    int lRc;
    const char *lTail;
    ConnMap* lConnMap = SqliteFunction::getConnectionMap(aDctx);

    //std::cout << "Query or Update: " << aQry << std::endl;
    lDb = lConnMap->getConn(aUUID);
    if(lDb == NULL){
      // throw error, ID not recognized
      throwError("SQLI0002", "DB ID not recognized");
    }

    lRc = sqlite3_prepare_v2(lDb, aQry.c_str(), aQry.size(), &lPstmt, &lTail);
    if(lRc != 0 && lPstmt != NULL){
      sqlite3_finalize(lPstmt);
    }
    if(lRc == SQLITE_ERROR) {
      std::string lErr = "SQL Statement is not valid; ";
      lErr += sqlite3_errmsg(lDb);
      throwError("SQLI0003", lErr.c_str());
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
      throwError("SQLI0004", "prepared statement not valid");
    }
    lRc = sqlite3_bind_int(lPstmt, aPos, (aVal==true)?1:0);
    if(lRc == SQLITE_RANGE)
      throwError("SQLI0005", "parameter position out of range");
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
      throwError("SQLI0004", "prepared statement not valid");
    }
    lRc = sqlite3_bind_int(lPstmt, aPos, aVal);
    if(lRc == SQLITE_RANGE)
      throwError("SQLI0005", "parameter position out of range");
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
      throwError("SQLI0004", "prepared statement not valid");
    }
    lRc = sqlite3_bind_double(lPstmt, aPos, aVal);
    if(lRc == SQLITE_RANGE)
      throwError("SQLI0005", "parameter position out of range");
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
      throwError("SQLI0004", "prepared statement not valid");
    }
    lRc = sqlite3_bind_text(lPstmt, aPos, aVal.c_str(), aVal.size(), SQLITE_STATIC);
    if(lRc == SQLITE_RANGE)
      throwError("SQLI0005", "parameter position out of range");
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
      throwError("SQLI0004", "prepared statement not valid");
    }
    lRc = sqlite3_bind_null(lPstmt, aPos);
    if(lRc == SQLITE_RANGE)
      throwError("SQLI0005", "parameter position out of range");
    else
      checkForError(lRc, 0, sqlite3_db_handle(lPstmt));
  }

  void
  SqliteFunction::clearValues(const zorba::DynamicContext* aDctx,
    std::string aUUID)
  {
    sqlite3_stmt *lPstmt;
    StmtMap *stmtMap = getStatementMap(aDctx);
    int lRc, aPos = 1;
    // go over all the parameters in the prepared statement
    // and set them to null
    do {
      lRc = sqlite3_bind_null(lPstmt, aPos);
      aPos++;
    } while(lRc != SQLITE_RANGE);
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
        throwError("SQLI9999", sqlite3_errmsg(sql));
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
    store::StoreConsts::JSONItemKind lJSONK = aOptions.getJSONItemKind();
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
        SqliteFunction::throwError("SQLI0007", ("Unknown option specified - "+lItemJSONKey.getStringValue().str()).c_str());
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

/*******************************************************************************
 *                         JSONItemSequence::JSONIterator                      *
 ******************************************************************************/
  void JSONItemSequence::JSONIterator::open(){
    // Get data and create the column names
    if(theStmt != NULL){
      theRc = sqlite3_step(theStmt);
      SqliteFunction::checkForError((theRc==SQLITE_ROW || theRc==SQLITE_DONE)?0:-1, 0, sqlite3_db_handle(theStmt));
      if(theRc == SQLITE_DONE)
        isUpdateResult = true;
      theFactory = Zorba::getInstance(0)->getItemFactory();

      theColumnCount = sqlite3_column_count(theStmt);
      theColumnNames = new std::string[theColumnCount];
      for(int i=0; i<theColumnCount; i++){
        theColumnNames[i] = sqlite3_column_name(theStmt, i);
      }
    }
  }

  bool JSONItemSequence::JSONIterator::next(zorba::Item& aItem){
    int aType;
    zorba::Item aKey;
    zorba::Item aValue;
    std::vector<std::pair<zorba::Item, zorba::Item> > elements;

    if(theRc == SQLITE_ROW){
      // get the resulting data from the statement
      // in a key = value fashion
      for(int i=0; i<theColumnCount; i++){
        aKey = theFactory->createString(theColumnNames[i]);
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
        default:
          std::string str = std::string((const char *)sqlite3_column_text(theStmt, i));
          aValue = theFactory->createString(
            zorba::String(str)
          );
        }
        elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      }
      aItem = theFactory->createJSONObject(elements);
      elements.clear();
      // Get more data if available
      theRc = sqlite3_step(theStmt);
      return true;
    } else if(isUpdateResult && theRc == SQLITE_DONE){
      // we have a prepared statement that represents a UPDATE and it's already executed
      aKey = theFactory->createString("Affected Rows");
      aValue = theFactory->createInt(sqlite3_changes(sqlite3_db_handle(theStmt)));
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
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
    theColumnCount = 0;
    delete[] theColumnNames;
    if(theStmt != NULL)
      sqlite3_reset(theStmt);
  }

/*******************************************************************************
 *              JSONMetadataItemSequence::JSONMetadataIterator                 *
 ******************************************************************************/
  void JSONMetadataItemSequence::JSONMetadataIterator::open(){
    // Get data and create the column names
    if(theStmt != NULL){
      theRc = sqlite3_step(theStmt);
      SqliteFunction::checkForError((theRc==SQLITE_ROW || theRc==SQLITE_DONE)?0:-1, 0, sqlite3_db_handle(theStmt));
      theFactory = Zorba::getInstance(0)->getItemFactory();

      theColumnCount = sqlite3_column_count(theStmt);
      theColumnNames = new std::string[theColumnCount];
      theActualColumn = 0;
      for(int i=0; i<theColumnCount; i++){
        theColumnNames[i] = sqlite3_column_name(theStmt, i);
      }
      if(theColumnCount > 0)
        theRc = SQLITE_ROW;
      else
        theRc = SQLITE_ERROR;
    } else
      theRc = SQLITE_ERROR;
  }

  bool JSONMetadataItemSequence::JSONMetadataIterator::next(zorba::Item& aItem){
    zorba::Item aKey;
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
      aKey = theFactory->createString("Database");
      aValue = theFactory->createString(lDbName);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      aKey = theFactory->createString("Table");
      aValue = theFactory->createString(lTableName);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      aKey = theFactory->createString("Column Name");
      aValue = theFactory->createString(lOriginName);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      aKey = theFactory->createString("Declared Type");
      aValue = theFactory->createString(lDataType);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      aKey = theFactory->createString("Collation");
      aValue = theFactory->createString(lCollSequence);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      aKey = theFactory->createString("Not Null");
      aValue = theFactory->createBoolean((lNotNull==0)?false:true);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      aKey = theFactory->createString("Primary Key");
      aValue = theFactory->createBoolean((lPrimaryKey==0)?false:true);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
      aKey = theFactory->createString("Autoinc");
      aValue = theFactory->createBoolean((lAutoinc==0)?false:true);
      elements.push_back(std::pair<zorba::Item, zorba::Item>(aKey, aValue));
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
    delete[] theColumnNames;
    if(theStmt != NULL)
      sqlite3_reset(theStmt);
  }

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
      lDbName = ":memory:";
    lRc = sqlite3_open_v2(lDbName.c_str(), &lSqldb, lOptions.getOptionsAsInt(), NULL);
    if(lRc == SQLITE_CANTOPEN)
      throwError("SQLI0001", "DB file does not exist");
    else
      checkForError(lRc, 0, lSqldb);

    // Store the UUID for this connection and return it
    lStrUUID = createUUID();
    lConnMap->storeConn(lStrUUID, lSqldb);

    return ItemSequence_t(new SingletonItemSequence(SqliteModule::getItemFactory()->createAnyURI(lStrUUID)));
  }


/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    DisconnectFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  { 
    sqlite3 *lSqldb;
    Item lItem = getOneItem(aArgs, 0);
    ConnMap* lConnMap = getConnectionMap(aDctx);
    
    lSqldb = lConnMap->getConn(lItem.getStringValue().str());
    if(lSqldb != NULL){
      // In case we have it connected, disconnect it
      sqlite3_close(lSqldb);
    } else {
      // throw error, UUID not recognized
      throwError("SQLI0002", "DB ID not recognized");
    }

    return ItemSequence_t(new SingletonItemSequence(lItem));
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
      throwError("SQLI0002", "DB ID not recognized");

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
      throwError("SQLI0002", "DB ID not recognized");

    return ItemSequence_t(new SingletonItemSequence(lItemUUID));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    sqlite3_stmt *lPstmt;
    Item lItemUUID = getOneItem(aArgs, 0);
    Item lItemQry = getOneItem(aArgs, 1);

    // Create the prepared statement with the UUID and Query passed
    lPstmt = createPreparedStatement(aDctx, lItemUUID.getStringValue().str(),
      lItemQry.getStringValue().str());

    // Once we got the SQL Query executed just pass it to the JSON Sequence
    // so it will return what we need to the user
    std::auto_ptr<JSONItemSequence> lSeq(new JSONItemSequence(lPstmt));
    return ItemSequence_t(lSeq.release());
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

    // Create the prepared statement with the UUID and Query passed
    lPstmt = createPreparedStatement(aDctx, lItemUUID.getStringValue().str(),
      lItemQry.getStringValue().str());

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

    // Create the prepared statement with the UUID and Query passed
    lPstmt = createPreparedStatement(aDctx, lItemUUID.getStringValue().str(),
      lItemQry.getStringValue().str());

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
    sqlite3_stmt *lPstmt;
    Item lItemPstmt = getOneItem(aArgs, 0);
    StmtMap *stmtMap = getStatementMap(aDctx);

    // Get the prepared statement
    lPstmt = stmtMap->getStmt(lItemPstmt.getStringValue().str());
    if(lPstmt == NULL){
      // No valid prepared statement id passed
      throwError("SQLI0004", "The Prepared Statement ID passed is not valid");
    }

    // So now create a JSONMetadataItemSequence and let it
    // get us what we need
    std::auto_ptr<JSONMetadataItemSequence> lSeq(new JSONMetadataItemSequence(lPstmt));
    return ItemSequence_t(lSeq.release());
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
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, lItem.getBooleanValue());
      break;
    case store::XS_BYTE:
    case store::XS_INT:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, lItem.getIntValue());
      break;
    case store::XS_INTEGER:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, strToInt(lItem.getStringValue().str()));
      break;
    case store::XS_FLOAT:
    case store::XS_DOUBLE:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, lItem.getDoubleValue());
      break;
    case store::XS_DECIMAL:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, strToDouble(lItem.getStringValue().str()));
      break;
    case store::XS_STRING:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, lItem.getStringValue().str());
      break;
    default:
      throwError("SQLI0007", "value is not of a valid type");
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

    setValueToStatement(aDctx, lItemUUID.getStringValue().str(), strToInt(lItemPos.getStringValue().str()), lItemBool.getBooleanValue());
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
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, lItemNumeric.getIntValue());
      break;
    case store::XS_INTEGER:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, strToInt(lItemNumeric.getStringValue().str()));
      break;
    case store::XS_FLOAT:
    case store::XS_DOUBLE:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, lItemNumeric.getDoubleValue());
      break;
    case store::XS_DECIMAL:
      setValueToStatement(aDctx, lItemUUID.getStringValue().str(), lPos, strToDouble(lItemNumeric.getStringValue().str()));
      break;
    default:
      throwError("SQLI0006", "value is not a valid numeric type");
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

    setValueToStatement(aDctx, lItemUUID.getStringValue().str(), strToInt(lItemPos.getStringValue().str()), lItemString.getStringValue().str());
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

    setValueToStatement(aDctx, lItemUUID.getStringValue().str(), strToInt(lItemPos.getStringValue().str()));
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
    ExecutePreparedFunction::evaluate(
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
      throwError("SQLI0004", "prepared statement not valid");

    // And let the JSONItemSequence execute it
    std::auto_ptr<JSONItemSequence> lSeq(new JSONItemSequence(lPstmt));
    return ItemSequence_t(lSeq.release());
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
      throwError("SQLI0004", "prepared statement not valid");

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
      throwError("SQLI0004", "prepared statement not valid");

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
