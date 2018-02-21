##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   2015
## License:     GPLv3+
##---------------------------------------------------------------------------

SET(SRC_NMEA0183
    src/nmea0183/latlong.cpp
    src/nmea0183/long.cpp
    src/nmea0183/nmea0183.cpp
    src/nmea0183/response.cpp
    src/nmea0183/rmb.cpp
    src/nmea0183/sentence.cpp
    src/nmea0183/talkerid.cpp
    src/nmea0183/rmc.cpp
    src/nmea0183/hexvalue.cpp
    src/nmea0183/lat.cpp
    src/nmea0183/expid.cpp
    src/nmea0183/apb.cpp
    src/nmea0183/xte.cpp
    src/nmea0183/GPwpl.cpp
    src/nmea0183/wpl.cpp
    src/nmea0183/rte.cpp
    src/nmea0183/hdt.cpp
    src/nmea0183/hdg.cpp
    src/nmea0183/hdm.cpp
    src/nmea0183/gll.cpp
    src/nmea0183/vtg.cpp
    src/nmea0183/gga.cpp
    src/nmea0183/gsv.cpp
)
INCLUDE_DIRECTORIES(src/nmea0183)
