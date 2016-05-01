# Install script for directory: C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib

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
   "C:/Program Files (x86)/taglib/lib/libtag.dll.a")
FILE(INSTALL DESTINATION "C:/Program Files (x86)/taglib/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/libtag.dll.a")
  IF(EXISTS "$ENV{DESTDIR}/C:/Program Files (x86)/taglib/lib/libtag.dll.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/C:/Program Files (x86)/taglib/lib/libtag.dll.a")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "C:/QtSDK/mingw/bin/strip.exe" "$ENV{DESTDIR}/C:/Program Files (x86)/taglib/lib/libtag.dll.a")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/taglib/bin/libtag.dll")
FILE(INSTALL DESTINATION "C:/Program Files (x86)/taglib/bin" TYPE SHARED_LIBRARY FILES "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/libtag.dll")
  IF(EXISTS "$ENV{DESTDIR}/C:/Program Files (x86)/taglib/bin/libtag.dll" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/C:/Program Files (x86)/taglib/bin/libtag.dll")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "C:/QtSDK/mingw/bin/strip.exe" "$ENV{DESTDIR}/C:/Program Files (x86)/taglib/bin/libtag.dll")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/taglib/include/taglib/tag.h;C:/Program Files (x86)/taglib/include/taglib/fileref.h;C:/Program Files (x86)/taglib/include/taglib/audioproperties.h;C:/Program Files (x86)/taglib/include/taglib/taglib_export.h")
FILE(INSTALL DESTINATION "C:/Program Files (x86)/taglib/include/taglib" TYPE FILE FILES
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/tag.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/fileref.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/audioproperties.h"
    "C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/taglib_export.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/toolkit/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/asf/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpeg/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/ogg/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/flac/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/ape/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mpc/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/mp4/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/wavpack/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/trueaudio/cmake_install.cmake")
  INCLUDE("C:/Users/Bridges/Documents/LCDHost/taglib-1.7/taglib/riff/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

