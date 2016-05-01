# Install script for directory: C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/taglib")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/taglib/include/taglib/taglib.h;C:/Program Files (x86)/taglib/include/taglib/tstring.h;C:/Program Files (x86)/taglib/include/taglib/tlist.h;C:/Program Files (x86)/taglib/include/taglib/tlist.tcc;C:/Program Files (x86)/taglib/include/taglib/tstringlist.h;C:/Program Files (x86)/taglib/include/taglib/tbytevector.h;C:/Program Files (x86)/taglib/include/taglib/tbytevectorlist.h;C:/Program Files (x86)/taglib/include/taglib/tfile.h;C:/Program Files (x86)/taglib/include/taglib/tmap.h;C:/Program Files (x86)/taglib/include/taglib/tmap.tcc")
FILE(INSTALL DESTINATION "C:/Program Files (x86)/taglib/include/taglib" TYPE FILE FILES
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/taglib.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tstring.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tlist.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tlist.tcc"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tstringlist.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tbytevector.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tbytevectorlist.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tfile.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tmap.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/tmap.tcc"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

