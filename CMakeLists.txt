##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3+
### *   This program is free software; you can redistribute it and/or modify  *
## *   it under the terms of the GNU General Public License as published by  *
## *   the Free Software Foundation; either version 2 of the License, or     *
## *   (at your option) any later version.                                   *
## *                                                                         *
## *   This program is distributed in the hope that it will be useful,       *
## *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
## *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
## *   GNU General Public License for more details.                          *
## *                                                                         *
## *   You should have received a copy of the GNU General Public License     *
## *   along with this program; if not, write to the                         *
## *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx,                                       *
## *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                       *
## ***
#-----------------------------------------------------------------------------
# Frontend2 Author: Jon Gough  
# Frontend2: v1.0.137.0
# ---------------------------------------------------------------------------
## ----- When changing this file do NOT change the order in which sections occur        ----- ##
## ----- Changes should only be made between the section blocks that identify where     ----- ##
## ----- these changes should be. The whole configuration process relies on this        ----- ##
## ----- sequence to be successful                                                      ----- ##
##                                                                                      ----- ##
##----- Modify section below to include all the details for your plugin                 ----- ##

set(CMLOC "CMakeLists: ")

# define minimum cmake version
cmake_minimum_required(VERSION 3.1.1)
if(COMMAND cmake_policy)
    if(POLICY CMP0043)
        cmake_policy(SET CMP0043 NEW)
    endif(POLICY CMP0043)
    cmake_policy(SET CMP0048 NEW)
    if(POLICY CMP0077)
        cmake_policy(SET CMP0077 NEW)
    endif(POLICY CMP0077)
endif(COMMAND cmake_policy)

# define plugin name, owner and versions
set(VERBOSE_NAME "Autopilot_Route")
set(COMMON_NAME "Autopilot Route")  # Search "getcommonname"in <plugin_pi>.cpp
set(TITLE_NAME "AUTOPILOT_ROUTE")
set(PACKAGE_CONTACT "Sean D'Epagnier")
set(PACKAGE "autopilot_route_pi")
set(SHORT_DESCRIPTION "Autopilot Route PlugIn for OpenCPN")
set(LONG_DESCRIPTION "Configurable Autopilot Route following abilities.")

set(VERSION_MAJOR "0")
set(VERSION_MINOR "4")
set(VERSION_PATCH "0")
set(VERSION_TWEAK "9")
set(VERSION_DATE "09/21/2020")
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")

set(PARENT "opencpn")

# The next line allows setup of a local webserver with git for testing purposes.
# The default is shown below, but it is done in cmake/pluginconfigure.cmake now.
# This default is needed to build locally and to build flatpak.
#set(GIT_REPOSITORY_SERVER "github.com")

#No longer needed, queried.
#set(GIT_USER "rgleason")
#set(GIT_USER "seandepagnier")							  
#set(GIT_REPOSITORY_NAME "autopilot_plugin_pi")

# Specifies Cloudsmith upload repository suffix for each catalog
set(PROD "prod")    #Standard Repos
set(BETA "beta")    #Standard Repos
set(ALPHA "alpha")  #Standard Repos

#set(CLOUDSMITH_USER "jon-gough")
#set(CLOUDSMITH_USER "rick-gleason")
set(CLOUDSMITH_USER "opencpn")

# Set if yourCloudsmith Base Repository name does not match your Git Repository name.
#set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins") #Rick's customization
set (CLOUDSMITH_BASE_REPOSITORY "autopilot-route")  #without "pi" and "-" instead of "_"

set(XML_INFO_URL "https://opencpn.org/wiki/dokuwiki/doku.php?id=opencpn:opencpn_user_manual:plugins:safety:odraw1.6_pi")
set(XML_SUMMARY ${SHORT_DESCRIPTION})
set(XML_DESCRIPTION ${LONG_DESCRIPTION})

##
## ----- Modify section above to include all the details for your plugin ----- ##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

# SET(CMAKE_MACOSX_RPATH "ON")

option(PLUGIN_USE_SVG "Use SVG graphics" OFF)

set(CMAKE_CXX_STANDARD 11)

# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")
# Don't use local version of GLU library
set(USE_LOCAL_GLU FALSE)

option(USE_GL "Enable OpenGL support" ON)
message(STATUS "${CMLOC}USE_GL: ${USE_GL}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "${CMLOC}Build type: ${CMAKE_BUILD_TYPE}")


##
## ----- The statements below are used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
##

project(${PACKAGE})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;")

##=======================================
include("PluginSetup")
##=======================================
##
## ----- The above is used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

set(CMAKE_VERBOSE_MAKEFILE ON)

option(Plugin_CXX11 "Use c++11" OFF)

#--- For Weatherfax insert Portaudio Section here  

##
## ----- Modify section above if there are special requirements for the plugin ----- ##
## ----- do not change next section - needed to configure build process ----- ##
##
##=======================================
include("PluginConfigure")
##=======================================
##
## ----- do not change section above - needed to configure build process ----- ##
## ----- Change below to match project requirements for source, headers, etc. ----- ##
#

add_definitions(-DUSE_S57)

IF(PLUGIN_USE_SVG)
    MESSAGE(STATUS "${CMLOC}SVG support enabled...")
    IF(NOT QT_ANDROID)
        ADD_DEFINITIONS(-DPLUGIN_USE_SVG)
    ENDIF(NOT QT_ANDROID)
ENDIF(PLUGIN_USE_SVG)

SET(SRCS
            src/autopilot_route_pi.cpp
            src/autopilot_route_pi.cpp
            src/PreferencesDialog.cpp
            src/AutopilotRouteUI.cpp
            src/concanv.cpp
            src/computation.cpp
            src/georef.c
            src/icons.cpp
            src/ODAPI.h
			api-16/ocpn_plugin.h
	)

#added for android  from pypilot
set(LIBHDRS
    libs/GL/gl.h
    libs/GL/gl_private.h
    libs/GL/glext.h
    libs/GL/glu.h
)

add_definitions(-DPLUGIN_USE_SVG)
##
# Modify/configure 'Include' Directories below for the Plugin
##
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/extinclude)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wx/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/api-16)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/tinyxml)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/nmea0183)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/jsoncpp)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/jsoncpp/json)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/plugingl)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include/GL)


##
## ----- Change above to match project requirements ----- ##


##
## Statement below is required to collect all the set( headers and SRCS  - Adjust as required
## This add_library  can be used in conditionals below.
##
## SET(EXTSRC  ${EXTSRC}   etc   Can be in an if statement as below
##
##==========================================================================
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRCS})
##==========================================================================
#  ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_AUTOPILOT_ROUTE})

#needed only if glshim project is included.
if(QT_ANDROID)
  target_link_libraries(${PACKAGE_NAME}  GL_static)
endif(QT_ANDROID)  


add_definitions(-DTIXML_USE_STL)

#IF(UNIX)
# INCLUDE("cmake/FindTinyXML.cmake")
# FIND_PACKAGE(TinyXML QUIET)
# ENDIF(UNIX)

#IF(TINYXML_FOUND)
# message (STATUS "Building with system tinyxml")
# INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
# ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRCS} ${SRC_JSON} ${SRC_NMEA0183})
# TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES})
#ELSE(TINYXML_FOUND)
# message (STATUS "Building with embedded tinyxml")
# INCLUDE_DIRECTORIES(src/tinyxml/)
# SET(SRC_LTINYXML
#            src/tinyxml/tinyxml.cpp
#            src/tinyxml/tinyxmlparser.cpp
#            src/tinyxml/tinyxmlerror.cpp
# )
# ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRCS} ${SRC_JSON} ${SRC_NMEA0183} ${SRC_LTINYXML})
# ENDIF(TINYXML_FOUND)





add_definitions(-DPLUGIN_USE_SVG)


## These are NOT used in Testplugin, used for other plugins
##=======================================
INCLUDE("PluginNMEA")
INCLUDE("PluginJSON")
INCLUDE("PluginGL")
##=======================================

## ----- do not change next section - needed to configure build process ----- ##
##
##=======================================
INCLUDE("PluginInstall")
INCLUDE("PluginLocalization")
INCLUDE("PluginPackage")
##=======================================

#from wxroute
#get_target_property(OUT ${PACKAGE_NAME} LINK_LIBRARIES)
#message(STATUS "${CMLOC}Libraries: ${OUT}")

##
## ----- do not change section above - needed to configure build process ----- ##
##


## These are NOT used in Testplugin, used for other plugins
##=======================================			
