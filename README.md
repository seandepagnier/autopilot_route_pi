Autopilot Route Plugin for OpenCPN
=======================================
Implement Configurable Autopilot Route following abilities for opencpn

Compiling
=========

* git clone git://github.com/seandepagnier/autopilot_route_pi.git

Do NOT attempt to build this plugin from within the OpenCPN source tree.

Under windows, you must find the file "opencpn.lib" (Visual Studio) or "libopencpn.dll.a" (mingw) which is built in the build directory after compiling opencpn.  This file must be copied to the plugin directory.

Build as normally:

* cd ..
* cd build
* cmake ..
* make
* make install

License
=======
The plugin code is licensed under the terms of the GPL v3 or, at your will, later.
