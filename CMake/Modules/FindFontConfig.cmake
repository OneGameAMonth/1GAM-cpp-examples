# - find FontConfig
# FONTCONFIG_INCLUDE_DIR - Where to find FontConfig header files (directory)
# FONTCONFIG_LIBRARIES - FontConfig libraries
# FONTCONFIG_LIBRARY_RELEASE - Where the release library is
# FONTCONFIG_LIBRARY_DEBUG - Where the debug library is
# FONTCONFIG_FOUND - Set to TRUE if we found everything (library, includes and executable)

# Copyright (c) 2013 Mads Andreas Elvheim, <mads@mechcore.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Generated by CModuler, a CMake Module Generator - http://gitorious.org/cmoduler

IF( FONTCONFIG_INCLUDE_DIR AND FONTCONFIG_LIBRARY_RELEASE AND FONTCONFIG_LIBRARY_DEBUG )
    SET(FONTCONFIG_FIND_QUIETLY TRUE)
ENDIF( FONTCONFIG_INCLUDE_DIR AND FONTCONFIG_LIBRARY_RELEASE AND FONTCONFIG_LIBRARY_DEBUG )

FIND_PATH( FONTCONFIG_INCLUDE_DIR fontconfig.h PATH_SUFFIXES fontconfig )

FIND_LIBRARY(FONTCONFIG_LIBRARY_RELEASE NAMES fontconfig )

FIND_LIBRARY(FONTCONFIG_LIBRARY_DEBUG NAMES fontconfig  HINTS /usr/lib/debug/usr/lib/ )

IF( FONTCONFIG_LIBRARY_RELEASE OR FONTCONFIG_LIBRARY_DEBUG AND FONTCONFIG_INCLUDE_DIR )
	SET( FONTCONFIG_FOUND TRUE )
ENDIF( FONTCONFIG_LIBRARY_RELEASE OR FONTCONFIG_LIBRARY_DEBUG AND FONTCONFIG_INCLUDE_DIR )

IF( FONTCONFIG_LIBRARY_DEBUG AND FONTCONFIG_LIBRARY_RELEASE )
	# if the generator supports configuration types then set
	# optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
	IF( CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE )
		SET( FONTCONFIG_LIBRARIES optimized ${FONTCONFIG_LIBRARY_RELEASE} debug ${FONTCONFIG_LIBRARY_DEBUG} )
	ELSE( CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE )
    # if there are no configuration types and CMAKE_BUILD_TYPE has no value
    # then just use the release libraries
		SET( FONTCONFIG_LIBRARIES ${FONTCONFIG_LIBRARY_RELEASE} )
	ENDIF( CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE )
ELSEIF( FONTCONFIG_LIBRARY_RELEASE )
	SET( FONTCONFIG_LIBRARIES ${FONTCONFIG_LIBRARY_RELEASE} )
ELSE( FONTCONFIG_LIBRARY_DEBUG AND FONTCONFIG_LIBRARY_RELEASE )
	SET( FONTCONFIG_LIBRARIES ${FONTCONFIG_LIBRARY_DEBUG} )
ENDIF( FONTCONFIG_LIBRARY_DEBUG AND FONTCONFIG_LIBRARY_RELEASE )

IF( FONTCONFIG_FOUND )
	IF( NOT FONTCONFIG_FIND_QUIETLY )
		MESSAGE( STATUS "Found FontConfig header file in ${FONTCONFIG_INCLUDE_DIR}")
		MESSAGE( STATUS "Found FontConfig libraries: ${FONTCONFIG_LIBRARIES}")
	ENDIF( NOT FONTCONFIG_FIND_QUIETLY )
ELSE(FONTCONFIG_FOUND)
	IF( FONTCONFIG_FIND_REQUIRED)
		MESSAGE( FATAL_ERROR "Could not find FontConfig" )
	ELSE( FONTCONFIG_FIND_REQUIRED)
		MESSAGE( STATUS "Optional package FontConfig was not found" )
	ENDIF( FONTCONFIG_FIND_REQUIRED)
ENDIF(FONTCONFIG_FOUND)
