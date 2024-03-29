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
# ---------------------------------------------------------------------------
# Testplugin Template: v1.0.222.0 25/12/2022  Author: Jon Gough
# ---------------------------------------------------------------------------
## ----- When changing this file do NOT change the order in which sections occur    ----- ##
## ----- Changes should only be made between the section blocks that identify where ----- ##
## ----- these changes should be. The whole configuration process relies on this    ----- ##
## ----- sequence to be successful                                                  ----- ##
##                                                                                  ----- ##
##----- Modify section below to include all the details for your plugin             ----- ##

set(CMLOC "CMakeLists: ")

# define minimum cmake version
cmake_minimum_required(VERSION 3.5.1)
if(COMMAND cmake_policy)
    if(POLICY CMP0043)
        cmake_policy(SET CMP0043 NEW)
    endif(POLICY CMP0043)
    cmake_policy(SET CMP0048 NEW)
# Need cmake 3.13 to implement this, but raspbian builds are at 3.7.2 and bionic at 3.10 and xenial builds at 3.5.1
#    cmake_policy(SET CMP0076 NEW)
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
set(VERSION_PATCH "8")
set(VERSION_TWEAK "0")

set(VERSION_DATE "11/01/2022") # DD/MM/YYYY format
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")

set(PARENT "opencpn")

# The next line allows setup of a local webserver with git for testing purposes.
#set(GIT_REPOSITORY_SERVER "github.com")

# Specifies Cloudsmith upload repository suffix for each catalog
set(PROD "prod")    #Standard Repos
set(BETA "beta")    #Standard Repos
set(ALPHA "alpha")  #Standard Repos

# Set if yourCloudsmith Base Repository name does not match your Git Repository name.
#set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins") #Rick's customization
set (CLOUDSMITH_BASE_REPOSITORY "autopilot-route")  #without "pi" and "-" instead of "_"

set(CLOUDSMITH_USER "opencpn")
#set(CLOUDSMITH_USER "rick-gleason")


set(XML_INFO_URL "https://opencpn.org/wiki/dokuwiki/doku.php?id=opencpn:opencpn_user_manual:plugins:safety:odraw1.6_pi")
set(XML_SUMMARY ${SHORT_DESCRIPTION})
set(XML_DESCRIPTION ${LONG_DESCRIPTION})

##
## ----- Modify section above to include all the details for your plugin ----- ##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

option(PLUGIN_USE_SVG "Use SVG graphics" OFF)

set(CMAKE_CXX_STANDARD 11)

# Use local version of GLU library
# set(USE_LOCAL_GLU TRUE)
# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")
# Don't use local version of GLU library
set(USE_LOCAL_GLU FALSE)

option(USE_GL "Enable OpenGL support" ON)
message(STATUS "${CMLOC}USE_GL: ${USE_GL}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "${CMLOC}Build type: ${CMAKE_BUILD_TYPE}")

##
## ----- The statements below are used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
##

project(${PACKAGE})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##=======================================
include("PluginSetup")
##=======================================
##
## ----- The above is used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

set(CMAKE_VERBOSE_MAKEFILE "Activate verbose mode for make files" ON)

option(Plugin_CXX11 "Use c++11" OFF)

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

#add_definitions(-DUSE_S57)

SET(SRCS
    src/autopilot_route_pi.cpp
    src/PreferencesDialog.cpp
    src/AutopilotRouteUI.cpp
    src/concanv.cpp
    src/computation.cpp
    src/georef.c
    src/icons.cpp
    src/ODAPI.h
	)

SET(HDRS
    src/PreferencesDialog.h
	src/msvcdefs.h
	src/icons.h
	src/georef.h
	src/georef.c
    src/concanv.h
    src/computation.h
	src/AutopilotRouteUI.h
    src/autopilot_route_pi.h
	src/wxWTranslateCatalog.h
)


set(OCPNSRC
    ocpnsrc/cutil.cpp
    ocpnsrc/TexFont.cpp
)

set(OCPNHDRS
    ocpninclude/cutil.h
    ocpninclude/pluginmanager.h
    ocpninclude/shaders.h
    ocpninclude/TexFont.h
    ocpninclude/vector2D.h
    libs/ocpn-api/ocpn_plugin.h
)

#set(EXTSRC)
#set(EXTINCLUDE extinclude/nlohmann/json.hpp extinclude/ODAPI.h)
#set(LIBSSRC libs/wxJSON/src/jsonreader.cpp libs/wxJSON/src/jsonval.cpp libs/wxJSON/src/jsonwriter.cpp)

#added for android  from pypilot
set(LIBHDRS
    libs/GL/gl.h
    libs/GL/gl_private.h
    libs/GL/glext.h
    libs/GL/glu.h
#    libs/wx/include/wx/json_defs.h
#    libs/wx/include/wx/jsonreader.h
#    libs/wx/include/wx/jsonval.h
#    libs/wx/include/wx/jsonwriter.h
	)
	
add_definitions(-DPLUGIN_USE_SVG)

set(EXTINCLUDE_DIR ${EXTINCLUDE_DIR} extinclude libs/ocpn-api/)

#
# ----- If using JSON validation in plugin section below is needed ----- ##
#

# Not using JSON Validation here

#
# ----- If using JSON validation in plugin section above is needed ----- ##
#


##
# Modify/configure 'Include' Directories below for the Plugin
##

#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/extinclude)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/nmea0183)  
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/plugingl)


# All from tesplugin v1.0.180
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/${EXTINCLUDE_DIR})
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/GL)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/glu)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/nmea0183) 
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/ocpn-api)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/jsoncpp)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/jsoncpp/json)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/nmea0183) 
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wxJSON/include)


#
# ----- Change below to match project requirements for android build ----- ##
#


if(QT_ANDROID)
    include_directories( ${PROJECT_SOURCE_DIR}/extinclude/GLES2)
endif(QT_ANDROID)

##
## ----- Change above to match project requirements for android build ----- ##
##


## These are NOT used in Testplugin, used for other plugins
##=======================================
# INCLUDE("PluginJSON")
INCLUDE("PluginGL")   #not needed in earlier version
# INCLUDE("PluginNMEA")
##=======================================


##
## Statement below is required to collect all the set( headers and SRCS  - Adjust as required
## This add_library  can be used in conditionals below.
##
# target_sources(${PACKAGE_NAME} PUBLIC ${SRCS} ${HDRS} ${OCPNSRC} ${EXTSRC} ${LIBSSRC})
target_sources(${PACKAGE_NAME} PUBLIC ${SRCS} ${HDRS} ${LIBHDRS} ${SRC_NMEA0183} ${SRC_JSON} ${SRC_PLUGINGL})

#try
#target_sources(${PACKAGE_NAME} PUBLIC ${SRCS} ${HDRS} ${LIBHDRS}  ${SRC_PLUGINGL} ${OCPNSRC})


if(NOT OCPN_FLATPAK_CONFIG)
    # Build environment not available when flatpak is being configured so following statements will not work
    message(STATUS "${CMLOC}Adding target link libraries to ${PACKAGE_NAME}")

    add_subdirectory(libs/tinyxml)
    target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)
	
	add_subdirectory(libs/jsoncpp)
    target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)
	
	add_subdirectory(libs/nmea0183)
    target_link_libraries(${PACKAGE_NAME} ocpn::nmea0183)

endif(NOT OCPN_FLATPAK_CONFIG)


add_definitions(-DTIXML_USE_STL)


# add_definitions(-DPLUGIN_USE_SVG)


#
# ----- If using JSON validation in plugin section below is needed ----- ##
#
# Not using JSON Validation here

#
# ----- If using JSON validation in plugin section above is needed ----- ##
#

##
## ----- Change above to match project requirements ----- ##
##

##
## ----- do not change next section - needed to configure build process ----- ##
##

# Needed for android builds
if(QT_ANDROID)
    include_directories(BEFORE ${qt_android_include})
endif(QT_ANDROID)


##=======================================
# Needed for all builds
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
message(STATUS "${CMLOC}include directories: ")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "${CMLOC}dir='${dir}'")
endforeach()
