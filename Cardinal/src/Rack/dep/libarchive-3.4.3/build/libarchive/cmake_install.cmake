# Install script for directory: /home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/build/libarchive/libarchive.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry_acl.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry_linkify.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry_misc.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry_paths.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry_perms.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry_stat.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry_time.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_add_passphrase.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_data.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_disk.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_extract.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_filter.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_format.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_free.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_header.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_new.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_open.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_read_set_options.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_util.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_blocksize.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_data.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_disk.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_filter.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_finish_entry.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_format.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_free.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_header.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_new.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_open.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_set_options.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_write_set_passphrase.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man5" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/cpio.5")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/libarchive.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/libarchive_changes.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/libarchive_internals.3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man5" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/libarchive-formats.5")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man5" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/mtree.5")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man5" TYPE FILE FILES "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/tar.5")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive.h"
    "/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/libarchive/archive_entry.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/thib/Documents/Polytech/S8/PROJET/docs/Cardinal/src/Rack/dep/libarchive-3.4.3/build/libarchive/test/cmake_install.cmake")

endif()

