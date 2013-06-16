xquery version "3.0";

(:
 : Copyright 2012 The FLWOR Foundation.
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
 :)

(:~
 : <p>This module provides functionality to extract, add and modify data 
 : from SQLite databases.</p>
 :
 : @author Luis Rodgriguez
 : @library <a href="http://www.sqlite.org/">SQLite</a>
 : @project DB Drivers/SQLite
 :)
 
module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
declare namespace an = "http://www.zorba-xquery.com/annotations";
declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "1.0";

(:~
 : Connect to a SQLite database.
 :
 : @param $db-name the SQLite database name to be opened as xs:string.
 :
 : @return the SQLite database object as xs:anyURI.
 :
 : @error s:SQLI0001 if the database name doesn't exist or it couldn't be opened.
 : @error s:SQLI0008 if a non-in-memory database is requested and the module
 :     is built without filesystem access
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:connect(
  $db-name as xs:string
  ) as xs:anyURI external;

(:~
 : Connect to a SQLite database with optional options.
 : All options are true/false values. Available options are: open-read-only,
 : open-create, open-no-mutex, and open-shared-cache.
 :
 : <p>The options are of the form: 
 : <pre>
 : {
 :   "open_read_only" : true,
 :   "open_create     : false,
 :   ... 
 : }
 : </pre>
 : </p>
 :
 : @param $db-name the SQLite database name to be opened as xs:string.
 : @param $options a JSON object containing SQLite connection options.
 :
 : @return the SQLite database object as xs:anyURI.
 :
 : @error s:SQLI0001 if the databse name doesn't exist or it couldn't be opened.
 : @error s:SQLI0007 if there is any unknown option specified.
 : @error s:SQLI0008 if a non-in-memory database is requested and the module
 :     is built without filesystem access
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:connect(
  $db-name as xs:string,
  $options as object()?
  ) as xs:anyURI external;
 
(:~
 : Returns whether on not the passed SQLite database object is connected.
 :
 : @param $conn the SQLite database object as xs:anyURI.
 :
 : @return true if the given SQLite database object is connected, false otherwise.
 :
 : @error s:SQLI0002 if $conn is not a valid SQLite database object.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:is-connected(
  $conn as xs:anyURI ) as xs:boolean external;
  
(:~
 : Commits all the pending update operations in this SQLite database.
 :
 : @param $conn the SQLite database object as xs:anyURI.
 :
 : @return the passed SQLite object.
 :
 : @error s:SQLI0002 if $conn is not a valid SQLite database object.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:commit(
  $conn as xs:anyURI ) as xs:anyURI external;
  
(:~
 : Rollbacks all the pending update operations in this SQLite database.
 :
 : @param $conn the SQLite database object as xs:anyURI.
 :
 : @return the passed SQLite object.
 :
 : @error s:SQLI0002 if $conn is not a valid SQLite database object.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:rollback(
  $conn as xs:anyURI ) as xs:anyURI external;
  
(:~
 : Executes a query (select command) over an already opened SQLite database
 : object.
 :
 : @param $conn an already opened SQLite database object as xs:anyURI.
 : @param $sqlstr the query to be executed as xs:string.
 :
 : @return a sequence of JSON objects describing each row returned.
 :
 : @error s:SQLI0002 if $conn is not a valid SQLite database object.
 : @error s:SQLI0003 if $stmnt is not a valid sql command.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:execute-query(
  $conn as xs:anyURI,
  $sqlstr as xs:string ) as object()* external;
  
(:~
 : Executes a update command over an already opened SQLite database object.
 :
 : @param $conn an already opened SQLite database object as xs:anyURI.
 : @param $sqlstr the update command to be executed as xs:string.
 :
 : @return the amount of rows modified by such update command.
 :
 : @error s:SQLI0002 if $conn is not a valid SQLite database object.
 : @error s:SQLI0003 if $stmnt is not a valid sql command.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:execute-update(
  $conn as xs:anyURI,
  $sqlstr as xs:string ) as xs:integer external;

(:~
 : Returns the metadata associated to a given prepared SQLite statement.
 :
 : <p>SQLite metadata is returned in the following form:
 : <pre>
 : {
 :   "columns" : 
 :       [{
 :           "name"          : &lt;column name>,
 :           "table"         : &lt;table name>,
 :           "database"      : &lt;database name>,
 :           "type"          : &lt;type name>,
 :           "collation"     : [BINARY|NOCASE|RTRIM],
 :           "nullable"      : [true|false],
 :           "primary key"   : [true|false],
 :           "autoincrement" : [true|false]
 :       }*]
 : }
 : </pre>
 : </p>
 :
 : @param $pstmnt the sql command as xs:anyURI from which metadata will be extracted.
 :
 : @return a object() with the associated the metadata.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI0009 if no metadata is available (SQLite library compiled without
 :     SQLITE_ENABLE_COLUMN_METADATA).
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:metadata(
  $pstmnt as xs:anyURI ) as object() external;
  
(:~
 : Compiles a prepared statement based on an already connected SQLite database
 : and a string that defines the sql command.
 :
 : <p>You can use '?' in the sql command as placeholders so you will
 : be able to bind variables later to such places.</p>
 :
 : @param $conn the SQLite database object as xs:anyURI.
 : @param $stmnt the sql command as xs:string.
 :
 : @return a xs:anyURI object representing the prepared statement.
 :
 : @error s:SQLI0002 if $conn is not a valid SQLite database object.
 : @error s:SQLI0003 if $stmnt is not a valid sql command.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:prepare-statement(
  $conn as xs:anyURI,
  $stmnt as xs:string ) as xs:anyURI external;
  
(:~
 : Binds a value to a placeholder inside a prepared statement using the
 : same type as the item given.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val the value to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position.
 : @error s:SQLI0007 if $val is not a valid value.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:set-value(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as item() ) as empty-sequence() external;
  
(:~
 : Binds a boolean to a placeholder inside a prepared statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val the boolean to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:set-boolean(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as xs:boolean ) as empty-sequence() external;
  
(:~
 : Binds a double variable to a placeholder inside a prepared statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val a xs:double, xs:integer or xs:decimal to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position.
 : @error s:SQLI0006 if $val is not a valid numeric type.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:set-numeric(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as xs:anyAtomicType ) as empty-sequence() external;
  
(:~
 : Binds a string variable to a placeholder inside a prepared statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val a string to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:set-string(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as xs:string ) as empty-sequence() external;
  
(:~
 : Set a null to a placeholder inside a prepared statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:set-null(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer ) as empty-sequence() external;
  
(:~
 : Set all parameters to null inside a prepared statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:clear-params(
  $pstmnt as xs:anyURI ) as empty-sequence() external;
  
(:~
 : Close and free resources associated to a prepared statement.
 :
 : @param $pstmnt the prepared statement to be closed.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 :)
declare %an:sequential function s:close-prepared(
  $pstmnt as xs:anyURI ) as empty-sequence() external;
  
(:~
 : Execute a query (select command) over an already connected SQLite
 : database object.
 :
 : @param $pstmnt the query command to be executed as xs:anyURI.
 :
 : @return a sequence of JSON objects representing the query results.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:execute-query-prepared(
  $pstmnt as xs:anyURI ) as object()* external;
  
(:~
 : Execute a update command over an already connected SQLite
 : database object.
 :
 : @param $pstmnt the update command to be executed as xs:anyURI.
 :
 : @return an integer that represents the amount of rows affected.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid SQLite prepared statement.
 : @error s:SQLI9999 if there was an internal error inside SQLite library.
 :)
declare %an:sequential function s:execute-update-prepared(
  $pstmnt as xs:anyURI ) as xs:integer external;
  
