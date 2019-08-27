##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3+
##---------------------------------------------------------------------------
 
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(autopilot_route_pi)
SET(PACKAGE_NAME autopilot_route_pi)
SET(VERBOSE_NAME Autopilot_Route)
SET(TITLE_NAME AUTOPILOT_ROUTE)
SET(CPACK_PACKAGE_CONTACT "Sean D'Epagnier ")

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_AUTOPILOT_ROUTE
            src/autopilot_route_pi.cpp
            src/autopilot_route_pi.cpp
            src/PreferencesDialog.cpp
            src/AutopilotRouteUI.cpp
            src/concanv.cpp
            src/computation.cpp
            src/georef.c
            src/icons.cpp
            src/ODAPI.h
	)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_AUTOPILOT_ROUTE})

INCLUDE("cmake/PluginNMEA.cmake")
INCLUDE("cmake/PluginJSON.cmake")
INCLUDE("cmake/PluginGL.cmake")


INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
