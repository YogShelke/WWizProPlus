# Install script for directory: E:/WardWiz_Developement/WardWizCrashHandler/x86

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/CrashRpt")
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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/demos/ConsoleDemo/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/demos/WTLDemo/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/demos/MFCDemo/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/reporting/crashrpt/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/reporting/crashsender/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/processing/crashrptprobe/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/processing/crprober/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/tests/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/thirdparty/tinyxml/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/thirdparty/jpeg/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/thirdparty/libpng/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/thirdparty/minizip/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/thirdparty/zlib/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/thirdparty/libogg/cmake_install.cmake")
  include("E:/WardWiz_Developement/WardWizCrashHandler/x86/thirdparty/libtheora/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

file(WRITE "E:/WardWiz_Developement/WardWizCrashHandler/x86/${CMAKE_INSTALL_MANIFEST}" "")
foreach(file ${CMAKE_INSTALL_MANIFEST_FILES})
  file(APPEND "E:/WardWiz_Developement/WardWizCrashHandler/x86/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
endforeach()
