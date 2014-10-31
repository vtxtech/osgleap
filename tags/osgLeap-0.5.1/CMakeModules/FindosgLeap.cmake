# Locate osgLeap Library
#
# This module defines
# OSGLEAP_LIBRARY
# OSGLEAP_FOUND, if false, do not try to link to leap
# OSGLEAP_INCLUDE_DIR, where to find the headers
#
# Created by Johannes Scholz. 

FIND_PATH(OSGLEAP_INCLUDE_DIR osgLeap/LeapManipulator
	${CMAKE_INSTALL_PREFIX}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /opt/include
    /usr/freeware/include
)

FIND_LIBRARY(OSGLEAP_LIBRARY 
    NAMES osgLeap
    PATHS
	${CMAKE_INSTALL_PREFIX}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /opt/lib
    /usr/freeware/lib64
)

FIND_LIBRARY(OSGLEAP_LIBRARY_DEBUG 
    NAMES osgLeap${CMAKE_DEBUG_POSTFIX}
    PATHS
	${CMAKE_INSTALL_PREFIX}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /opt/lib
    /usr/freeware/lib64
)

SET(OSGLEAP_FOUND "NO")
IF(OSGLEAP_LIBRARY AND OSGLEAP_INCLUDE_DIR)
    SET(OSGLEAP_FOUND "YES")
ENDIF(OSGLEAP_LIBRARY AND OSGLEAP_INCLUDE_DIR)
