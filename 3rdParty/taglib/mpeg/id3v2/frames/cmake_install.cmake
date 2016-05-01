# Install script for directory: C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames

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
   "C:/Program Files (x86)/taglib/include/taglib/attachedpictureframe.h;C:/Program Files (x86)/taglib/include/taglib/commentsframe.h;C:/Program Files (x86)/taglib/include/taglib/generalencapsulatedobjectframe.h;C:/Program Files (x86)/taglib/include/taglib/popularimeterframe.h;C:/Program Files (x86)/taglib/include/taglib/privateframe.h;C:/Program Files (x86)/taglib/include/taglib/relativevolumeframe.h;C:/Program Files (x86)/taglib/include/taglib/textidentificationframe.h;C:/Program Files (x86)/taglib/include/taglib/uniquefileidentifierframe.h;C:/Program Files (x86)/taglib/include/taglib/unknownframe.h;C:/Program Files (x86)/taglib/include/taglib/unsynchronizedlyricsframe.h;C:/Program Files (x86)/taglib/include/taglib/urllinkframe.h")
FILE(INSTALL DESTINATION "C:/Program Files (x86)/taglib/include/taglib" TYPE FILE FILES
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/attachedpictureframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/commentsframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/popularimeterframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/privateframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/relativevolumeframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/textidentificationframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/uniquefileidentifierframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/unknownframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/id3v2/frames/urllinkframe.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

