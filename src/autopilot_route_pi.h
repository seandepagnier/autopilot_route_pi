/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  autopilot route Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2018 by Sean D'Epagnier                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef _AUTOPILOTROUTEPI_H_
#define _AUTOPILOTROUTEPI_H_

#include "wx/wx.h"

#include <wx/fileconf.h>

#include "version.h"
#include "wxWTranslateCatalog.h"

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    10

#define ABOUT_AUTHOR_URL "http://seandepagnier.users.sourceforge.net"

#define OPC wxS("opencpn-autopilot_route_pi")

#include <list>
#include <map>

#ifndef WXINTL_NO_GETTEXT_MACRO
#ifdef OPC
#ifdef _
#undef _
#endif // _
#define _(s) wxGetTranslation((s),OPC)
#endif // OPC
#else 
#define _(s) wxGetTranslation((s))
#endif // WXINTL_NO_GETTEXT_MACRO


#include "ocpn_plugin.h"

#ifdef __MSVC__
#include "msvcdefs.h"
#endif

//----------------------------------
//    The PlugIn Class Definition
//----------------------------------

class apDC;
class ConsoleCanvas;
class PreferencesDialog;

#include "computation.h"

class waypoint : public wp {
public:
    waypoint() {}
    waypoint(double lat, double lon) : wp(lat, lon) {}
    waypoint(double lat, double lon, wxString name, wxString guid, double ar, double lat0, double lon0);

    wxString name, GUID;
    double arrival_radius;
    double arrival_bearing;
};

typedef std::list<waypoint> ap_route;
typedef std::list<waypoint>::iterator ap_route_iterator;

class autopilot_route_pi : public wxEvtHandler, public opencpn_plugin_110
{
    friend ConsoleCanvas;
public:

    autopilot_route_pi(void *ppimgr);
    
    int Init(void);
    bool DeInit(void);

    int GetAPIVersionMajor();
    int GetAPIVersionMinor();
    int GetPlugInVersionMajor();
    int GetPlugInVersionMinor();
    wxBitmap *GetPlugInBitmap();
    wxString GetCommonName();
    wxString GetShortDescription();
    wxString GetLongDescription();

    void SetColorScheme(PI_ColorScheme cs);
    void ShowPreferencesDialog( wxWindow* parent );

    bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
    bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);

    void ShowConsoleCanvas();
    void ShowPreferences();

    static wxString StandardPath();

    PlugIn_Position_Fix_Ex &LastFix() { return m_lastfix; }
    double Declination();

    // these are stored to the config
    struct preferences {
        enum Mode {STANDARD_XTE, BOUNDARY_XTE, WAYPOINT_BEARING, ROUTE_POSITION_BEARING} mode;

        // Standard XTE
        double xte_multiplier, xte_rate_multiplier;

        // Boundary XTE
        double xte_boundary_multiplier;

        // Waypoint Bearing

        // Route Position Bearing
        enum RoutePositionBearingMode {DISTANCE, TIME} route_position_bearing_mode;
        double route_position_bearing_distance, route_position_bearing_time;

        // Active Route Window
        std::map<wxString, bool> active_route_labels[2];
        bool ActiveRouteLabel(int i, wxString label) {
            if(active_route_labels[i].find(label) != active_route_labels[i].end())
                return active_route_labels[i][label];
            return false;
        }

        // Waypoint Arrival
        bool confirm_bearing_change;

        // Boundary
        wxString boundary_guid;
        int boundary_width;

        // NMEA output
        int rate;
        bool magnetic;
        std::map<wxString, bool> nmea_sentences;
        bool NmeaSentences(wxString sentence) {
            if(nmea_sentences.find(sentence) != nmea_sentences.end())
                return nmea_sentences[sentence];
            return false;
        }
    } prefs;

    // for console canvas
    bool GetConsoleInfo(double &sog, double &cog, double &bearing, double &xte,
                        double *rng, double *nrng);
    void DeactivateRoute();
protected:
    void Render(apDC &dc, PlugIn_ViewPort &vp);
    void RenderArrivalWaypoint(apDC &dc, PlugIn_ViewPort &vp);
    void RenderRoutePositionBearing(apDC &dc, PlugIn_ViewPort &vp);
    void OnTimer( wxTimerEvent & );

    wxPoint m_cursor_position;
    PlugIn_Position_Fix_Ex m_lastfix;

private:
    void SetCursorLatLon(double lat, double lon);
    void SetNMEASentence(wxString &sentence);
    void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);
    void SetPluginMessage(wxString &message_id, wxString &message_body);

    void RearrangeWindow();

    void RequestRoute(wxString guid);
    void AdvanceWaypoint();
    void UpdateWaypoint();
    double FindXTE();
    void ComputeXTE();
    void ComputeBoundaryXTE();
    void ComputeWaypointBearing();
    void ComputeRoutePositionBearing();
    void MagneticHeading(double &val);

    void SendRMB();
    void SendRMC();
    void SendAPB();
    void SendXTE();
    void SendNMEA();

    int m_leftclick_tool_id;
    wxTimer m_Timer;

    double m_declination;
    wxDateTime m_declinationTime;

    ConsoleCanvas *m_ConsoleCanvas;
    PreferencesDialog *m_PreferencesDialog;

    PI_ColorScheme m_colorscheme;

    wxString m_active_guid, m_active_request_guid;
    wxDateTime m_active_request_time;
    ap_route m_route;

    waypoint m_current_wp;
    wxString m_next_route_wp_GUID;
    wxString m_last_wp_name, m_last_wpt_activated_guid;

    bool m_bArrival;

    double m_current_bearing, m_current_xte;
    // for xte mode
    double m_xte_rate;

    // for xte boundary mode
    
    // for bearing mode
    
    // optimum route mode variables

    double m_avg_sog;
};

#endif
