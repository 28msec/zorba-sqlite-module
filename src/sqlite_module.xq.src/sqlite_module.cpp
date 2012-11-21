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
  ConnMap::storeConn(const String& aKeyName, sqlite3* sql)
  {
    std::pair<ConnMap_t::iterator,bool> ret;
    ret = connMap->insert(std::pair<String, sqlite3 *>(aKeyName, sql));
    return ret.second;
  }

  sqlite3*
  ConnMap::getConn(const String& aKeyName)
  {
    ConnMap_t::iterator lIter = connMap->find(aKeyName);

    if(lIter == connMap->end())
      return NULL;
    
    sqlite3 *lSql = lIter->second;

    return lSql;
  }

  bool
  ConnMap::deleteConn(const String& aKeyName)
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
        deleteConn(lIter->first);
      }
      connMap->clear();
      delete connMap;
    }
    delete this;
  }

  ConnMap::~ConnMap(){ }

/*******************************************************************************
 *                              SqliteFunction                                 *
 *******************************************************************************/
  SqliteFunction::SqliteFunction(const SqliteModule* aModule)
    : theModule(aModule) {}

  SqliteFunction::~SqliteFunction(){}

  /********************
   *  Sqlite Options  *
   ********************/

  SqliteOptions::SqliteOptions()
    : theOpenReadOnly(false),
      theOpenCreate(false),
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
    Item lOption;

    Iterator_t lOptionIter = aOptions.getChildren();
    lOptionIter->open();

    while (lOptionIter->next(lOption))
    {
      Item lOptionName;
      lOption.getNodeName(lOptionName);

      if (lOptionName.getLocalName() == "open-read-only")
      {
        theOpenReadOnly = lOption.getStringValue() == "true" ? true : false;;
      }
      else if (lOptionName.getLocalName() == "open-create")
      {
        theOpenCreate = lOption.getStringValue() == "true" ? true : false;;
      }
      else if (lOptionName.getLocalName() == "open-no-mutex")
      {
        theOpenNoMutex = lOption.getStringValue() == "true" ? true : false;
      }
      else if(lOptionName.getLocalName() == "open-shared-cache")
      {
        theOpenSharedCache = lOption.getStringValue() == "true" ? true : false;
      }
    }
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

/*******************************************************************************
 ******************************************************************************/
  void JSONItemSequence::JSONIterator::open(){
    if(theStmt != NULL){
      theRc = sqlite3_step(theStmt);
      SqliteFunction::checkForError((theRc==SQLITE_ROW)?0:-1, 0, sqlite3_db_handle(theStmt));
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

    if(theRc == SQLITE_ROW){
      std::vector<std::pair<zorba::Item, zorba::Item>> elements;
      // get the resulting data from the statement
      // in a key = value fashion
      for(int i=0; i<theColumnCount; i++){
        zorba::Item aKey = theFactory->createString(theColumnNames[i]);
        zorba::Item aValue;
        aType = sqlite3_column_type(theStmt, i);
        switch(aType){
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
      theRc = sqlite3_step(theStmt);
      return true;
    } else 
      return false;
  }

  void JSONItemSequence::JSONIterator::close(){
    theRc = SQLITE_ERROR;
    theColumnCount = 0;
    delete[] theColumnNames;
    if(theStmt != NULL)
      sqlite3_finalize(theStmt);
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ConnectFunction::evaluate(
      const Arguments_t& aArgs,
      const zorba::StaticContext* aSctx,
      const zorba::DynamicContext* aDctx) const 
  {
    sqlite3 *sqldb;
    int rc;
    ConnMap* lConnMap;
    DynamicContext* lDynCtx = const_cast<DynamicContext*>(aDctx);
    Item lItem = getOneItem(aArgs, 0);

    if(!(lConnMap = dynamic_cast<ConnMap*>(lDynCtx->getExternalFunctionParameter("sqliteConnMap"))))
    {
      lConnMap = new ConnMap();
      lDynCtx->addExternalFunctionParameter("sqliteConnMap", lConnMap);     
    }

    rc = sqlite3_open_v2(lItem.getStringValue().c_str(), &sqldb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    SqliteFunction::checkForError(rc, 0, sqldb);

    uuid lUUID;
    uuid::create(&lUUID);
    std::stringstream lStream;
    lStream << lUUID;

    lConnMap->storeConn(lStream.str(), sqldb);

    return ItemSequence_t(new SingletonItemSequence(SqliteModule::getItemFactory()->createAnyURI(lStream.str())));
  }


/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    DisconnectFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  { 
    sqlite3 *sqldb;
    DynamicContext* lDynCtx = const_cast<DynamicContext*>(aDctx);

    Item lItem = getOneItem(aArgs, 0);
    //String lStrUUID = item.getStringValue();

    ConnMap* lConnMap;
    if(!(lConnMap = dynamic_cast<ConnMap*>(lDynCtx->getExternalFunctionParameter("sqliteConnMap"))))
    {
      lConnMap = new ConnMap();
      lDynCtx->addExternalFunctionParameter("sqliteConnMap", lConnMap);     
    }

    sqldb = lConnMap->getConn(lItem.getStringValue());

    if(sqldb != NULL){
      sqlite3_close(sqldb);
    } else {
      // throw error, ID not recognized
      SqliteFunction::throwError("SQLI0001", "DB ID not recognized");
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
    sqlite3 *sqldb;
    DynamicContext* lDynCtx = const_cast<DynamicContext*>(aDctx);
    Item res;

    Item item = getOneItem(aArgs, 0);
    String lStrUUID = item.getStringValue();

    ConnMap* lConnMap;
    if(!(lConnMap = dynamic_cast<ConnMap*>(lDynCtx->getExternalFunctionParameter("sqliteConnMap"))))
    {
      lConnMap = new ConnMap();
      lDynCtx->addExternalFunctionParameter("sqliteConnMap", lConnMap);     
    }

    sqldb = lConnMap->getConn(lStrUUID);

    if(sqldb != NULL){
      res = SqliteModule::getItemFactory()->createBoolean(false);
    } else {
      res = SqliteModule::getItemFactory()->createBoolean(true);
    }

    return ItemSequence_t(new SingletonItemSequence(res));
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    CommitFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    RollbackFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteQueryFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  { 
    sqlite3 *lDb;
    sqlite3_stmt *lPstmt;
    int lRc;
    //const char *lSql;
    const char *lTail;
    DynamicContext* lDynCtx = const_cast<DynamicContext*>(aDctx);

    Item lItemID = getOneItem(aArgs, 0);
    Item lItemQry = getOneItem(aArgs, 1);

    std::cout << "Query: " << lItemQry.getStringValue().str().c_str() << std::endl;

    ConnMap* lConnMap;
    if(!(lConnMap = dynamic_cast<ConnMap*>(lDynCtx->getExternalFunctionParameter("sqliteConnMap"))))
    {
      lConnMap = new ConnMap();
      lDynCtx->addExternalFunctionParameter("sqliteConnMap", lConnMap);     
    }

    lDb = lConnMap->getConn(lItemID.getStringValue());

    if(lDb == NULL){
      // throw error, ID not recognized
      SqliteFunction::throwError("SQLI0001", "DB ID not recognized");
    }

    lRc = sqlite3_prepare_v2(lDb, lItemQry.getStringValue().str().c_str(), 
                             lItemQry.getStringValue().str().size(), &lPstmt, &lTail);
    if(lRc != 0 && lPstmt != NULL){
      sqlite3_finalize(lPstmt);
    }
    SqliteFunction::checkForError(lRc, 0, lDb);

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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    MetadataFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    PrepareStatementFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    SetValueFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
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
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
  }

/*******************************************************************************
 ******************************************************************************/
  zorba::ItemSequence_t
    ExecuteUpdatePreparedFunction::evaluate(
    const Arguments_t& aArgs,
    const zorba::StaticContext* aSctx,
    const zorba::DynamicContext* aDctx) const 
  {
    SqliteFunction::throwError("ERR9999", "Function not yet implemented");
    return ItemSequence_t(new EmptySequence());
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
