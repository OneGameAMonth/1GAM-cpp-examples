#######################################################################
## NOTE: CMAKE_PREFIX_PATH MUST BE SET ON WINDOWS BUILDS!            ##
## If not set to the directory that holds the /bin /include and /lib ##
## for your compiler, then you have two build options:               ##
##                                                                   ##
## 1.) The env variables VCINSTALLDIR and WindowsSdkDir must be set  ##
##     for MSVC builds in the MSVC env command prompt, or:           ##
##                                                                   ##
## 2) MSYS and MinGW must be used, with /mingw mounted under the MSYS##
##    root.                                                          ##
##                                                                   ##
## If SDL_INCLUDE_DIR and SDL_LIBRARY are not set when doing Windows ##
## builds, then the SDL include path and library path are assumed to ##
## be equal to CMAKE_PREFIX_PATH. Additional searching is done, but  ##
## it might not work on Windows.                                     ##
#######################################################################
CMAKE_MINIMUM_REQUIRED( VERSION 2.6)
SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

PROJECT(1GAM-examples)
SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 2 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

IF( ${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR} )
  MESSAGE( FATAL_ERROR
  "Stopping you from doing an in-source build. Bad, BAD :-)!\n"
  "Rather do:\n"
  "mkdir MyBuild\n"
  "cd MyBuild\n"
  "cmake <params> ..\n"
  "make\n")
  ELSE()
    MESSAGE( STATUS "Good, you're doing an out-source build. Keep it that way." )
ENDIF()

IF( NOT CMAKE_PREFIX_PATH )
  MESSAGE( WARNING "CMAKE_PREFIX_PATH not set. Attempting to guess.." )
  IF( WIN32 )
    IF( MSVC )
      SET( CMAKE_PREFIX_PATH "$ENV{VCINSTALLDIR};$ENV{WindowsSdkDir}" )
      IF( NOT CMAKE_PREFIX_PATH )
        MESSAGE(FATAL_ERROR "Couldn't find the VCINSTALLDIR and WindowsSdkDir environment variables." )
      ENDIF()
    ELSEIF( MINGW AND MSYS )
      SET(CMAKE_PREFIX_PATH "/mingw")
    ENDIF()
  ELSEIF( APPLE )
  ELSE()
    SET(CMAKE_PREFIX_PATH "/usr")  
  ENDIF()
  MESSAGE( STATUS "CMAKE_PREFIX_PATH set to ${CMAKE_PREFIX_PATH}" )
ENDIF()


FIND_PACKAGE( SDL REQUIRED )
FIND_PACKAGE( Freetype REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( PNG REQUIRED )
FIND_PACKAGE( FontConfig REQUIRED )

# Force variables into the cache
SET( CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE PATH
  "The prefix path to the toolchain's /bin /include and /lib" FORCE )


include_directories( AFTER "${SDL_INCLUDE_DIR}" )
include_directories( AFTER "${FREETYPE_INCLUDE_DIRS}" )
include_directories( AFTER "${OPENGL_INCLUDE_DIR}" )

IF( UBUNTU )
  include_directories( AFTER "/usr/include/freetype2/freetype" )
  include_directories( AFTER "/usr/include/freetype2/" )
ENDIF()

IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
  SET( CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
ENDIF()

IF( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  SET( CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
ENDIF()

SET( PROJPATH
  font-rendering
  QuadTree
)
FOREACH(proj ${PROJPATH})
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/${proj} ${CMAKE_BINARY_DIR}/${proj}/bin )
ENDFOREACH()


