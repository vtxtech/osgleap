#
# CMake Build System for osgLeap
# Copyright (C) 2013 Johannes Kroeger/vtxtech. All rights reserved.
#
# This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
# but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

cmake_minimum_required(VERSION 2.8)
 
PROJECT(osgLeap)

SET(CMAKE_DEBUG_POSTFIX "d")
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
SET(OSG_DIR "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Path where to find the osg installation")

INCLUDE(OsgMacroUtils)

#######################################
# Versions
#######################################
SET(OSGLEAP_MAJOR_VERSION 0)
SET(OSGLEAP_MINOR_VERSION 5)
SET(OSGLEAP_PATCH_VERSION 0)

SET(OSGLEAP_VERSION ${OSGLEAP_MAJOR_VERSION}.${OSGLEAP_MINOR_VERSION}.${OSGLEAP_PATCH_VERSION})
SET(OSGLEAP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#######################################
# Library files
#######################################
#FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( osg REQUIRED )

SET(INSTALL_BINDIR "bin")
SET(INSTALL_LIBDIR "lib")
SET(INSTALL_ARCHIVEDIR "lib")
SET(INSTALL_INCDIR "include")

OPTION(OSGLEAP_BUILD_SHARED_LIBS "Set to ON to build osgLeap for dynamic linking.  Use OFF for static." ON)
IF (OSGLEAP_BUILD_SHARED_LIBS)
    SET(OSGLEAP_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE (OSGLEAP_BUILD_SHARED_LIBS)
    SET(OSGLEAP_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
ENDIF(OSGLEAP_BUILD_SHARED_LIBS)

OPTION(OSGLEAP_BUILD_EXAMPLES "Set to ON to build osgLeap examples." ON)
#OPTION(OSGLEAP_INSTALL_DATA "Set to ON to install osgLeap data." ON)
#SET(OSGLEAP_DATA_INSTALLDIR $ENV{OSG_FILE_PATH} CACHE PATH "Path where to install osgLeap data")
#ADD_SUBDIRECTORY(data)

OPTION(LEAPSDK_080_COMPATIBILITYMODE "Set to ON to enable LeapSDK v0.8.0 compatibility mode. Disable for LeapSDK v0.8.1 or higher" OFF)
IF(LEAPSDK_080_COMPATIBILITYMODE)
	ADD_DEFINITIONS(-DLEAPSDK_080_COMPATIBILITYMODE)
ENDIF(LEAPSDK_080_COMPATIBILITYMODE)
SET(OSGLEAP_EXAMPLES_INSTALLDIR "${CMAKE_INSTALL_PREFIX}/share/osgLeap/bin")

INCLUDE_DIRECTORIES(BEFORE
    ${OPENGL_INCLUDE_DIR}
    ${OSG_INCLUDE_DIRS}
    ${OSGLEAP_SOURCE_DIR}/src
	${OSGLEAP_SOURCE_DIR}/include
)

ADD_SUBDIRECTORY(src)

IF(OSGLEAP_BUILD_EXAMPLES)
	OPTION(OSGLEAP_INSTALL_EXAMPLES "Set to ON to install osgLeap examples." ON)
	ADD_SUBDIRECTORY(examples)
ENDIF(OSGLEAP_BUILD_EXAMPLES)

################################################################################
### uninstall target
################################################################################
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
