# ~~~
# Summary:      Local, generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "opencpn/autopilot_route-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "opencpn/autopilot_route-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "opencpn/autopilot_route-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME autopilot_route_pi) #or _pi
set(PKG_VERSION  0.7.0)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME Autopilot_Route)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME Autopilot_Route) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Autopilot_Routenstration plugin")
set(PKG_DESCRIPTION "Autopilot_Routenstration plugin, illustrates the use of the different OpenCPN Plugin API's ")
set(PKG_AUTHOR "Autopilot_Route Author")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/seandepagnier/autopilot_route_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/autopilot_routeplugin.html)
set(PKG_API_LIB api-18)   # Specify the plugin API level

add_definitions(-DocpnUSE_GL) # Specifiy the use of OpenGL

include_directories(${CMAKE_SOURCE_DIR}/inc)

SET(SRC
    src/autopilot_route_pi.cpp
    src/PreferencesDialog.cpp
    src/AutopilotRouteUI.cpp
    src/concanv.cpp
    src/computation.cpp
    src/georef.c
    src/icons.cpp
#    src/ODAPI.h
	)

SET(INC
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


add_definitions(-DPLUGIN_USE_SVG)

set (SOURCE_FILES ${SRC} ${INC})

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
  if (APPLE)
    target_compile_definitions(${PACKAGE_NAME} PUBLIC OCPN_GHC_FILESYSTEM)
  endif ()
endmacro ()

macro(add_plugin_libraries)

    add_subdirectory(opencpn-libs/nmea0183)
    target_link_libraries(${PACKAGE_NAME} ocpn::nmea0183)

    add_subdirectory(opencpn-libs/plugin_dc)
    target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

    add_subdirectory(opencpn-libs/jsoncpp)
    target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)

endmacro ()
