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
 :
 : @library <a href="http://www.sqlite.org/">sqlite</a>
 :)
 
module namespace s = "http://www.zorba-xquery.com/modules/sqlite";
declare namespace an = "http://www.zorba-xquery.com/annotations";
declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "1.0";
  
(:~
 : connect to a sqlite database.
 :
 : @param $db-name the sqlite database name to be opened as xs:string.
 :
 : @return the sqlite database object as xs:anyURI.
 :
 : @error s:SQLI0001 if the databse name doesn't exist or it couldn't be opened.
 :)
declare %an:sequential function s:connect(
  $db-name as xs:string
  ) as xs:anyURI external;

(:~
 : connect to a sqlite database with options.
 :
 : <p>The options are of the form: 
 : <pre>
 : &lt;sqlite:options>
 :   &lt;sqlite:readwrite/>
 :   &lt;sqlite:create/>
 : &lt;/sqlite:options>
 : </pre>
 : </p>
 :
 : @param $db-name the sqlite database name to be opened as xs:string.
 : @param $options sqlite connection options.
 :
 : @return the sqlite database object as xs:anyURI.
 :
 : @error s:SQLI0001 if the databse name doesn't exist or it couldn't be opened.
 :)
declare %an:sequential function s:connect(
  $db-name as xs:string,
  $options as element(s:options)
  ) as xs:anyURI external;
 
(:~
 : Disconnects the passed sqlite database object.
 :
 : <p>The $conn argument provides the database encoded as xs:anyURI.
 : This parameter should define a sqlite database.
 :
 : @param $conn the database object encoded as xs:anyURI.
 :
 : @return true if everything went ok.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database.
 :)
declare %an:sequential function s:disconnect(
  $conn as xs:anyURI ) as xs:anyURI external; 
  
(:~
 : Returns whether on not the passed sqlite database object is connected.
 :
 : @param $conn the sqlite database object as xs:anyURI.
 :
 : @return true if the given sqlite database object is connected, false otherwise.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database object.
 :)
declare function s:is-connected(
  $conn as xs:anyURI ) as xs:boolean external;
  
(:~
 : Commits all the pending update operations in this sqlite database.
 :
 : @param $conn the sqlite database object as xs:anyURI.
 :
 : @return the passed sqlite object.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database object.
 :)
declare function s:commit(
  $conn as xs:anyURI ) as xs:anyURI external;
  
(:~
 : Rollbacks all the pending update operations in this sqlite database.
 :
 : @param $conn the sqlite database object as xs:anyURI.
 :
 : @return the passed sqlite object.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database object.
 :)
declare function s:rollback(
  $conn as xs:anyURI ) as xs:anyURI external;
  
(:~
 : Executes a sql command (select or update command) over an already opened
 : sqlite database object.
 :
 : @param $conn an already opened sqlite database object as xs:anyURI.
 : @param $sqlstr the sql statement to be executed as xs:string.
 :
 : @return an id that defines a dataset object.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database object.
 : @error s:SQLI0003 if $stmnt is not a valid sql command.
 :)
declare function s:execute(
  $conn as xs:anyURI,
  $sqlstr as xs:string ) as xs:anyURI external;
  
(:~
 : Executes a query (select command) over an already opened sqlite database
 : object.
 :
 : @param $conn an already opened sqlite database object as xs:anyURI.
 : @param $sqlstr the query to be executed as xs:string.
 :
 : @return a sequence of JSON objects describing each row returned.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database object.
 : @error s:SQLI0003 if $stmnt is not a valid sql command.
 :)
declare function s:execute-query(
  $conn as xs:anyURI,
  $sqlstr as xs:string ) as object()* external;
  
(:~
 : Executes a update command over an already opened sqlite database object.
 :
 : @param $conn an already opened sqlite database object as xs:anyURI.
 : @param $sqlstr the update command to be executed as xs:string.
 :
 : @return the amount of rows modified by such update command.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database object.
 : @error s:SQLI0003 if $stmnt is not a valid sql command.
 :)
declare function s:execute-update(
  $conn as xs:anyURI,
  $sqlstr as xs:string ) as xs:integer external;

(:~
 : Returns the metadata associated to a given prepared sqlite statement.
 :
 : @param $pstmnt the update command to be executed as xs:anyURI.
 :
 : @return a sequence with the associated the metadata.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 :)
declare function s:metadata(
  $pstmnt as xs:anyURI ) as object()* external;
  
(:~
 : Compiles a prepared statement based on an already connected sqlite database
 : and a string that defines the sql command.
 :
 : <p>You can use '?' in the sql command as placeholders so you will
 : be able to bind variables later to such places.
 :
 : @param $conn the sqlite database object as xs:anyURI.
 : @param $stmnt the sql command as xs:string.
 :
 : @return a xs:anyURI object representing the prepared statement.
 :
 : @error s:SQLI0002 if $conn is not a valid sqlite database object.
 : @error s:SQLI0003 if $stmnt is not a valid sql command.
 :)
declare function s:prepare-statement(
  $conn as xs:anyURI,
  $stmnt as xs:string ) as xs:anyURI external;
  
(:~
 : Binds a value to a placeholder inside a prepated statement using the
 : same type as the item given.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val the value to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position.
 : @error s:SQLI0007 if $val is not a valid value.
 :)
declare function s:set-value(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as item() ) as empty-sequence() external;
  
(:~
 : Binds a boolean to a placeholder inside a prepated statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val the boolean to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position
 :)
declare function s:set-boolean(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as xs:boolean ) as empty-sequence() external;
  
(:~
 : Binds a double variable to a placeholder inside a prepated statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val a xs:double, xs:integer or xs:decimal to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position
 : @error s:SQLI0006 if $val is not a valid numeric type.
 :)
declare function s:set-numeric(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as xs:anyAtomicType ) as empty-sequence() external;
  
(:~
 : Binds a string variable to a placeholder inside a prepated statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 : @param $val a string to be bind in such placeholder.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position
 :)
declare function s:set-string(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer,
  $val as xs:string ) as empty-sequence() external;
  
(:~
 : Set a null to a placeholder inside a prepated statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 : @param $param-num the placeholder position to be set.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 : @error s:SQLI0005 if $param-num is not a valid position
 :)
declare function s:set-null(
  $pstmnt as xs:anyURI, 
  $param-num as xs:integer ) as empty-sequence() external;
  
(:~
 : Set all parameters to null inside a prepated statement.
 :
 : @param $pstmnt the prepared statement already compiled as xs:anyURI.
 :
 : @return nothing.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 :)
declare function s:clear-params(
  $pstmnt as xs:anyURI ) as empty-sequence() external;
  
(:~
 : Execute a sql command (select or update command) over an already connected
 : sqlite database object.
 :
 : @param $pstmnt the query command to be executed as xs:anyURI.
 :
 : @return an id that defines a dataset object.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 :)
declare function s:execute-prepared(
  $pstmnt as xs:anyURI ) as xs:anyURI external;
  
(:~
 : Execute a query (select command) over an already connected sqlite
 : database object.
 :
 : @param $pstmnt the query command to be executed as xs:anyURI.
 :
 : @return a sequence of JSON objects representing the query results.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 :)
declare function s:execute-query-prepared(
  $pstmnt as xs:anyURI ) as object()* external;
  
(:~
 : Execute a update command over an already connected sqlite
 : database object.
 :
 : @param $pstmnt the update command to be executed as xs:anyURI.
 :
 : @return an integer that represents the amount of rows affected.
 :
 : @error s:SQLI0004 if $pstmnt is not a valid sqlite prepared statement.
 :)
declare function s:execute-update-prepared(
  $pstmnt as xs:anyURI ) as xs:integer external;
  