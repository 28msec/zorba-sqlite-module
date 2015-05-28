# Copyright 2012 The FLWOR Foundation.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

MESSAGE(STATUS "")
MESSAGE(STATUS "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
MESSAGE(STATUS "Configuring external module <sqlite>")
MESSAGE(STATUS "-------------------------------------------------------------")
MESSAGE(STATUS "")

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT (zorba_sqlite_module)
ENABLE_TESTING ()
INCLUDE (CTest)

IF (WIN32)
  # On Windows we use proxy modules that try to guess first the location
  # of the required third party libraries. This will search in order in:
  # 1. the path pointed by ZORBA_THIRD_PARTY_REQUIREMENTS
  # 2. the Program Files directory available on the users computer
  # 3. the PATH environment variable
  # The logic is implemented by the macros in the ProxyFindModule.cmake module.
  LIST (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules/Windows")
ENDIF (WIN32)
LIST (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")

FIND_PACKAGE (Zorba REQUIRED HINTS "${ZORBA_BUILD_DIR}")
MESSAGE(STATUS "Zorba_USE_FILE ${Zorba_USE_FILE}")
INCLUDE ("${Zorba_USE_FILE}")

IF (ZORBA_SUPPRESS_SQLITE)
  MESSAGE (STATUS "ZORBA_SUPRESS_SQLITE is true - not searching for SQLite.")
  
  MESSAGE(STATUS "")
  MESSAGE(STATUS "-------------------------------------------------------------")  
  MESSAGE(STATUS "Configuration SUPPRESSED for external module <sqlite>")
  
ELSE (ZORBA_SUPRESS_SQLITE)
  
  MESSAGE (STATUS "Looking for SQLite")
  FIND_PACKAGE (SQLite3)

  IF (SQLITE_FOUND)
    MESSAGE (STATUS "Found SQLite --" ${SQLITE_LIBRARIES})

    # Set SQLITE_WITH_FILE_ACCESS - by default will be the same value as
    # ZORBA_WITH_FILE ACCESS when built inside Zorba, or "ON" if built
    # stand-alone.
    IF (DEFINED ZORBA_WITH_FILE_ACCESS)
      SET (_file_access ${ZORBA_WITH_FILE_ACCESS})
    ELSE (DEFINED ZORBA_WITH_FILE_ACCESS)
      SET (_file_access ON)
    ENDIF (DEFINED ZORBA_WITH_FILE_ACCESS)
    SET(SQLITE_WITH_FILE_ACCESS ${_file_access} CACHE BOOL
      "Allow filesystem-based SQLite databases")

    INCLUDE_DIRECTORIES (${SQLITE_INCLUDE_DIR})  

    ADD_SUBDIRECTORY("src")
    ADD_TEST_DIRECTORY("${PROJECT_SOURCE_DIR}/test")
    
    MESSAGE(STATUS "")
    MESSAGE(STATUS "-------------------------------------------------------------")
    MESSAGE(STATUS "Configuration SUCCEEDED for external module <sqlite>")
    
  ELSE (SQLITE_FOUND)
    MESSAGE (STATUS "SQLite library not found == if you want to use the sqlite functionality please set SQLITE_INCLUDE_DIR and SQLITE_LIBRARIES cmake parameters accordingly.")
    
    MESSAGE(STATUS "")
    MESSAGE(STATUS "-------------------------------------------------------------")  
    MESSAGE(STATUS "Configuration FAILED for external module <sqlite>")
    
  ENDIF (SQLITE_FOUND)
ENDIF (ZORBA_SUPPRESS_SQLITE)

DONE_DECLARING_ZORBA_URIS()

MESSAGE(STATUS "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")