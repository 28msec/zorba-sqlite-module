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

IF (SQLITE_INCLUDE_DIR)
  SET (SQLITE_FIND_QUIETLY TRUE)
ENDIF (SQLITE_INCLUDE_DIR)

FIND_PATH (
  SQLITE_INCLUDE_DIR
  sqlite3.h
  PATHS ${SQLITE_INCLUDE_DIR} /usr/include/ /usr/local/include /opt/local/include )
MARK_AS_ADVANCED (SQLITE_INCLUDE_DIR)

FIND_LIBRARY (
  SQLITE_LIBRARY
  NAMES sqlite3
  PATHS ${SQLITE_LIBRARY_DIR} /usr/lib /usr/local/lib /opt/local/lib)
MARK_AS_ADVANCED (SQLITE_LIBRARY)

IF (SQLITE_INCLUDE_DIR AND SQLITE_LIBRARY)
  SET (SQLITE_FOUND 1)
  SET (SQLITE_LIBRARIES ${SQLITE_LIBRARY})
  SET (SQLITE_INCLUDE_DIRS ${SQLITE_INCLUDE_DIR})
  IF (NOT SQLITE_FIND_QUIETLY)
    MESSAGE (STATUS "Found SQLite library: " ${SQLITE_LIBRARY})
    MESSAGE (STATUS "Found SQLite include path : " ${SQLITE_INCLUDE_DIR})
  ENDIF (NOT SQLITE_FIND_QUIETLY)

  SET(CMAKE_REQUIRED_INCLUDES "${SQLITE_INCLUDE_DIR}")
  SET(CMAKE_REQUIRED_LIBRARIES "${SQLITE_LIBRARY}")
  
  INCLUDE(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(sqlite3_column_database_name ZORBA_SQLITE_HAVE_METADATA)
ELSE (SQLITE_INCLUDE_DIR AND SQLITE_LIBRARY)
  SET (SQLITE_FOUND 0)
  SET (SQLITE_LIBRARIES)
  SET (SQLITE_INCLUDE_DIRS)
ENDIF (SQLITE_INCLUDE_DIR AND SQLITE_LIBRARY)  
