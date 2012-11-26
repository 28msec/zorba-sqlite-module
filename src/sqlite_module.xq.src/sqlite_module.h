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


//#ifndef ORG_EXPATH_NS_ARCHIVE_H_
//#define ORG_EXPATH_NS_ARCHIVE_H_

#include <map>
#include <set>

#include <zorba/zorba.h>
#include <zorba/item_factory.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <vector>
#include <sqlite3.h>

namespace zorba { namespace sqlite {

/*******************************************************************************
 ******************************************************************************/
  class ConnMap : public ExternalFunctionParameter
  {
    private:
      typedef std::map<std::string, sqlite3 *> ConnMap_t;
      ConnMap_t* connMap;

    public:
      ConnMap();
      virtual ~ConnMap();
      bool 
        storeConn(const std::string&, sqlite3 *sql);
      sqlite3*
        getConn(const std::string&);
      bool 
        deleteConn(const std::string&);
      virtual void 
        destroy() throw();
  };

  class StmtMap : public ExternalFunctionParameter
  {
    private:
      typedef std::map<std::string, sqlite3_stmt *> StmtMap_t;
      StmtMap_t* stmtMap;

    public:
      StmtMap();
      virtual ~StmtMap();
      bool 
        storeStmt(const std::string&, sqlite3_stmt *sql);
      sqlite3_stmt*
        getStmt(const std::string&);
      bool 
        deleteStmt(const std::string&);
      virtual void 
        destroy() throw();
  };

  class SqliteModule : public ExternalModule {
    protected:
      class ltstr
      {
      public:
        bool operator()(const String& s1, const String& s2) const
        {
          return s1.compare(s2) < 0;
        }
      };

      typedef std::map<String, ExternalFunction*, ltstr> FuncMap_t;
      FuncMap_t theFunctions;

    public:

      virtual ~SqliteModule();

      virtual zorba::String
      getURI() const { return getModuleURI(); }

      virtual zorba::ExternalFunction*
      getExternalFunction(const String& localName);

      virtual void destroy();

      static ItemFactory*
      getItemFactory()
      {
        return Zorba::getInstance(0)->getItemFactory();
      }

      static zorba::String
      getModuleURI() { return "http://www.zorba-xquery.com/modules/sqlite"; }

  };


/*******************************************************************************
 ******************************************************************************/
  class JSONItemSequence : public ItemSequence
  {
    public:
      class JSONIterator : public Iterator
      {
        protected:
          sqlite3_stmt* theStmt;
          std::string* theColumnNames;
          int theColumnCount;
          int theRc;
          bool isUpdateResult;
          // needed if theArchiveItem is not streamable an needs to be decoded
          // zorba::String   theDecodedData;

          zorba::ItemFactory* theFactory;

        public:
          JSONIterator(sqlite3_stmt* aPrepStmt):
              theStmt(aPrepStmt),theColumnNames(NULL),theColumnCount(0),
              theRc(0),isUpdateResult(false) {}

          virtual ~JSONIterator() {
          }

          void
          open();

          bool
          next(zorba::Item& aItem);

          void
          close();

          bool
          isOpen() const { return theRc == SQLITE_ROW; }
      };

    protected:
      sqlite3_stmt* thePrepStmt;

    public:
      JSONItemSequence(sqlite3_stmt* aPrepStmt)
        : thePrepStmt(aPrepStmt)
      {}

      virtual ~JSONItemSequence() {}

      zorba::Iterator_t 
        getIterator() { return new JSONIterator(thePrepStmt); }
  };

/*******************************************************************************
 ******************************************************************************/
  class JSONMetadataItemSequence : public ItemSequence
  {
    public:
      class JSONMetadataIterator : public Iterator
      {
        protected:
          sqlite3_stmt* theStmt;
          std::string* theColumnNames;
          int theColumnCount;
          int theRc;
          int theActualColumn;
          // needed if theArchiveItem is not streamable an needs to be decoded
          // zorba::String   theDecodedData;

          zorba::ItemFactory* theFactory;

        public:
          JSONMetadataIterator(sqlite3_stmt* aPrepStmt):
              theStmt(aPrepStmt),theColumnNames(NULL),theColumnCount(0),
              theRc(0), theActualColumn(0) {}

          virtual ~JSONMetadataIterator() {
          }

          void
          open();

          bool
          next(zorba::Item& aItem);

          void
          close();

          bool
          isOpen() const { return theRc == SQLITE_ROW; }
      };

    protected:
      sqlite3_stmt* thePrepStmt;

    public:
      JSONMetadataItemSequence(sqlite3_stmt* aPrepStmt)
        : thePrepStmt(aPrepStmt)
      {}

      virtual ~JSONMetadataItemSequence() {}

      zorba::Iterator_t 
        getIterator() { return new JSONMetadataIterator(thePrepStmt); }
  };

/*******************************************************************************
 ******************************************************************************/
  class SqliteOptions {
  protected:
    bool theOpenReadOnly;
    bool theOpenCreate;
    bool theOpenNoMutex;
    bool theOpenSharedCache;

  public:

    SqliteOptions();

    const bool
      getOpenReadOnly() const { return theOpenReadOnly; }

    const bool
      getOpenCreate() const { return theOpenCreate; }

    const bool
      getOpenNoMutex() const { return theOpenNoMutex; }

    const bool
      getOpenSharedCache() const { return theOpenSharedCache; }

    void
      setValues(Item&);

    void
      setValues(struct sqlite3* aSqlite);

  protected:
    static std::string
      getAttributeValue(
      const Item& aNode,
      const String& aAttrName = "value");
  };

/*******************************************************************************
 ******************************************************************************/
  class SqliteFunction : public ContextualExternalFunction {
    protected:
      const SqliteModule* theModule;

    public:
      SqliteFunction(const SqliteModule* module);
      virtual ~SqliteFunction();

      static zorba::Item
      getOneItem(const Arguments_t& aArgs, int aIndex);

      static ConnMap*
      getConnectionMap(const zorba::DynamicContext* aDctx);

      static StmtMap*
      getStatementMap(const zorba::DynamicContext* aDctx);

      static std::string
      createUUID();

      static sqlite3_stmt* 
      createPreparedStatement(const zorba::DynamicContext* aDctx,
        std::string aUUID,
        std::string aQry);

      static void
      setValueToStatement(const zorba::DynamicContext* aDctx,
        std::string aUUID,
        int aPos,
        bool aVal);

      static void
      setValueToStatement(const zorba::DynamicContext* aDctx,
        std::string aUUID,
        int aPos,
        int aVal);

      static void
      setValueToStatement(const zorba::DynamicContext* aDctx, 
        std::string aUUID,
        int aPos,
        double aVal);

      static void
      setValueToStatement(const zorba::DynamicContext* aDctx,
        std::string aUUID,
        int aPos,
        std::string aVal);

      static void
      setValueToStatement(const zorba::DynamicContext* aDctx,
        std::string aUUID,
        int aPos);

      virtual String
      getURI() const;

      static void
      throwError(const char*, const char*);

      static void
      checkForError(int aErrNo, const char* aLocalName, sqlite3 *sql);

  };

  class ConnectFunction : public SqliteFunction {
  public:
    ConnectFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ConnectFunction() {}

    virtual zorba::String
      getLocalName() const { return "connect"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class DisconnectFunction : public SqliteFunction {
  public:
    DisconnectFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~DisconnectFunction() {}

    virtual zorba::String
      getLocalName() const { return "disconnect"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class IsConnectedFunction : public SqliteFunction {
  public:
    IsConnectedFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~IsConnectedFunction() {}

    virtual zorba::String
      getLocalName() const { return "is-connected"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class CommitFunction : public SqliteFunction {
  public:
    CommitFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~CommitFunction() {}

    virtual zorba::String
      getLocalName() const { return "commit"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class RollbackFunction : public SqliteFunction {
  public:
    RollbackFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~RollbackFunction() {}

    virtual zorba::String
      getLocalName() const { return "rollback"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class ExecuteFunction : public SqliteFunction {
  public:
    ExecuteFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ExecuteFunction() {}

    virtual zorba::String
      getLocalName() const { return "execute"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class ExecuteQueryFunction : public SqliteFunction {
  public:
    ExecuteQueryFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ExecuteQueryFunction() {}

    virtual zorba::String
      getLocalName() const { return "execute-query"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class ExecuteUpdateFunction : public SqliteFunction {
  public:
    ExecuteUpdateFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ExecuteUpdateFunction() {}

    virtual zorba::String
      getLocalName() const { return "execute-update"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class MetadataFunction : public SqliteFunction {
  public:
    MetadataFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~MetadataFunction() {}

    virtual zorba::String
      getLocalName() const { return "metadata"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class PrepareStatementFunction : public SqliteFunction {
  public:
    PrepareStatementFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~PrepareStatementFunction() {}

    virtual zorba::String
      getLocalName() const { return "prepare-statement"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class SetValueFunction : public SqliteFunction {
  public:
    SetValueFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~SetValueFunction() {}

    virtual zorba::String
      getLocalName() const { return "set-value"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class SetBooleanFunction : public SqliteFunction {
  public:
    SetBooleanFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~SetBooleanFunction() {}

    virtual zorba::String
      getLocalName() const { return "set-boolean"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class SetNumericFunction : public SqliteFunction {
  public:
    SetNumericFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~SetNumericFunction() {}

    virtual zorba::String
      getLocalName() const { return "set-numeric"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class SetStringFunction : public SqliteFunction {
  public:
    SetStringFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~SetStringFunction() {}

    virtual zorba::String
      getLocalName() const { return "set-string"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class SetNullFunction : public SqliteFunction {
  public:
    SetNullFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~SetNullFunction() {}

    virtual zorba::String
      getLocalName() const { return "set-null"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class ClearParamsFunction : public SqliteFunction {
  public:
    ClearParamsFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ClearParamsFunction() {}

    virtual zorba::String
      getLocalName() const { return "clear-params"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class ExecutePreparedFunction : public SqliteFunction {
  public:
    ExecutePreparedFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ExecutePreparedFunction() {}

    virtual zorba::String
      getLocalName() const { return "execute-prepared"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class ExecuteQueryPreparedFunction : public SqliteFunction {
  public:
    ExecuteQueryPreparedFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ExecuteQueryPreparedFunction() {}

    virtual zorba::String
      getLocalName() const { return "execute-query-prepared"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

  class ExecuteUpdatePreparedFunction : public SqliteFunction {
  public:
    ExecuteUpdatePreparedFunction(const SqliteModule* aModule) : SqliteFunction(aModule) {}

    virtual ~ExecuteUpdatePreparedFunction() {}

    virtual zorba::String
      getLocalName() const { return "execute-update-prepared"; }

    virtual zorba::ItemSequence_t
      evaluate(const Arguments_t&,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
    
  };

} /* namespace sqlite  */ } /* namespace zorba */

//namespace std {
//
//ostream&
//operator<<(
//    ostream& out,
//    const zorba::sqlite::SqliteFunction::JSONEntry& e);
//
//}

//#endif // _ORG_EXPATH_WWW_NS_ARCHIVE_H_
