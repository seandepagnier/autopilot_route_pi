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
v *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/aui/aui.h>

#include "plugingl/pidc.h"

#include "json/json.h"

#include "nmea0183/nmea0183.h"

#include "georef.h"

#include "autopilot_route_pi.h"
#include "concanv.h"

#include "PreferencesDialog.h"
#include "icons.h"

double heading_resolve(double degrees, double offset=0)
{
    while(degrees < offset-180)
        degrees += 360;
    while(degrees >= offset+180)
        degrees -= 360;
    return degrees;
}

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new autopilot_route_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

waypoint::waypoint(double lat, double lon, wxString n, wxString guid,
                   double ar, double ab)
    : wp(lat, lon), name(n), GUID(guid), arrival_radius(ar), arrival_bearing(ab)
{
}


//-----------------------------------------------------------------------------
//
//    Autopilot_Route PlugIn Implementation
//
//-----------------------------------------------------------------------------

autopilot_route_pi::autopilot_route_pi(void *ppimgr)
    : opencpn_plugin_116(ppimgr)
{
    // Create the PlugIn icons
    initialize_images();
    m_ConsoleCanvas = NULL;
    m_PreferencesDialog = NULL;
    m_avg_sog=0;
    m_declination = NAN;
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int autopilot_route_pi::Init(void)
{
    AddLocaleCatalog( PLUGIN_CATALOG_NAME );

    // Read Config
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T( "/Settings/AutopilotRoute" ) );
    preferences &p = prefs;

    // Mode
    p.mode = pConf->Read("Mode", "Route Position Bearing");
    p.xte_multiplier = pConf->Read("XTEP", 1.0);
    p.route_position_bearing_mode = (preferences::RoutePositionBearingMode)
        pConf->Read("RoutePositionBearingMode", 0L);
    p.route_position_bearing_distance = pConf->Read("RoutePositionBearingDistance", 100);
    p.route_position_bearing_time = pConf->Read("RoutePositionBearingTime", 100);
    p.route_position_bearing_max_angle = pConf->Read("RoutePositionBearingMaxAngle", 30);
    
    // Active Route Window
    wxString labels[2] = {pConf->Read("ActiveRouteItems0", "XTE;BRG;RNG;TTG;VMG;Highway;Deactivate;"),
                          pConf->Read("ActiveRouteItems1", "XTE;Route ETA;Route RNG;Route TTG;Highway;")};
    for(int i=0; i<2; i++) {
        while(labels[i].size()) {
            p.active_route_labels[i][labels[i].BeforeFirst(';')] = true;
            labels[i] = labels[i].AfterFirst(';');
        }
    }

    // options
    p.confirm_bearing_change = (bool)pConf->Read("ConfirmBearingChange", 0L);
    p.intercept_route = (bool)pConf->Read("InterceptRoute", 1L);
    p.computation = pConf->Read("Computation", "Great Circle") == "Mercator" ? preferences::MERCATOR : preferences::GREAT_CIRCLE;

    // Boundary
    p.boundary_guid = pConf->Read("Boundary", "");
    p.boundary_width = pConf->Read("BoundaryWidth", 30);

    // NMEA output
    p.rate = pConf->Read("NMEARate", 1L);
    p.magnetic = (bool)pConf->Read("NMEAMagnetic", 0L);
    wxString sentences = pConf->Read("NMEASentences", "APB;");
    while(sentences.size()) {
        p.nmea_sentences[sentences.BeforeFirst(';')] = true;
        sentences = sentences.AfterFirst(';');
    }

    PlugInHandleAutopilotRoute(true);
    m_Timer.Connect(wxEVT_TIMER, wxTimerEventHandler
                    ( autopilot_route_pi::OnTimer ), NULL, this);

    return (WANTS_OVERLAY_CALLBACK |
            WANTS_OPENGL_OVERLAY_CALLBACK |
            WANTS_CURSOR_LATLON       |
            WANTS_NMEA_SENTENCES      |
            WANTS_NMEA_EVENTS         |
            WANTS_AIS_SENTENCES       |
            WANTS_PLUGIN_MESSAGING    |
            WANTS_PREFERENCES         |
            WANTS_CONFIG);
}

bool autopilot_route_pi::DeInit(void)
{    
    PlugInHandleAutopilotRoute(false);
    delete m_PreferencesDialog;

    m_Timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler( autopilot_route_pi::OnTimer ), NULL, this);
    
    RemovePlugInTool(m_leftclick_tool_id);

    // save config
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T( "/Settings/AutopilotRoute" ) );

    if(m_ConsoleCanvas) {
        wxPoint p = GetFrameAuiManager()->GetPane(m_ConsoleCanvas).floating_pos;
        pConf->Write("PosX", p.x);
        pConf->Write("PosY", p.y);
    }
    delete m_ConsoleCanvas;
   
    preferences &p = prefs;

    // Mode
    pConf->Write("Mode", p.mode);
    pConf->Write("XTEP", p.xte_multiplier);
    pConf->Write("RoutePositionBearingMode", (int)p.route_position_bearing_mode);
    pConf->Write("RoutePositionBearingDistance", p.route_position_bearing_distance);
    pConf->Write("RoutePositionBearingTime", p.route_position_bearing_time);
    pConf->Write("RoutePositionBearingMaxAngle", p.route_position_bearing_max_angle);
    // Active Route Window
    for(int i=0; i<2; i++) {
        wxString labels;
        for(std::map<wxString, bool>::iterator it = p.active_route_labels[i].begin();
            it != p.active_route_labels[i].end(); it++)
            if(p.active_route_labels[i][it->first])
                labels += it->first + ";";
        pConf->Write("ActiveRouteItems"+wxString::Format("%d", i), labels);
    }
    // Waypoint Arrival
    pConf->Write("ConfirmBearingChange", p.confirm_bearing_change);
    pConf->Write("InterceptRoute", p.intercept_route);
    pConf->Write("Computation", p.computation == preferences::MERCATOR ? "Mercator" : "Great Circle");

    // Boundary
    pConf->Write("Boundary", p.boundary_guid);
    pConf->Write("BoundaryWidth", p.boundary_width);

    // NMEA output
    pConf->Write("NMEARate", p.rate);
    pConf->Write("NMEAMagnetic", p.magnetic);
    wxString sentences;
    for(std::map<wxString, bool>::iterator it = p.nmea_sentences.begin();
        it != p.nmea_sentences.end(); it++)
        if(p.nmea_sentences[it->first])
            sentences += it->first + ";";
    pConf->Write("NMEASentences", sentences);

    return true;
}

int autopilot_route_pi::GetAPIVersionMajor()
{
    return OCPN_API_VERSION_MAJOR;
}

int autopilot_route_pi::GetAPIVersionMinor()
{
    return OCPN_API_VERSION_MINOR;
}

int autopilot_route_pi::GetPlugInVersionMajor()
{
    return PLUGIN_VERSION_MAJOR;
}

int autopilot_route_pi::GetPlugInVersionMinor()
{
    return PLUGIN_VERSION_MINOR;
}

wxBitmap *autopilot_route_pi::GetPlugInBitmap()
{
    return new wxBitmap(_img_autopilot_route->ConvertToImage().Copy());
}

wxString autopilot_route_pi::GetCommonName()
{
    return _T(PLUGIN_COMMON_NAME);
//    return _("Autopilot Route");
}

wxString autopilot_route_pi::GetShortDescription()
{
    return _(PLUGIN_SHORT_DESCRIPTION);
//    return _("Autopilot Route PlugIn for OpenCPN");
}

wxString autopilot_route_pi::GetLongDescription()
{
    return _(PLUGIN_LONG_DESCRIPTION); 
//    return _("Autopilot Route PlugIn for OpenCPN Configurable Autopilot Route following abilities.");
}

void autopilot_route_pi::SetColorScheme(PI_ColorScheme cs)
{
    m_colorscheme = cs;
    if( m_ConsoleCanvas )
        m_ConsoleCanvas->SetColorScheme(cs);
}

void autopilot_route_pi::ShowPreferencesDialog( wxWindow* parent )
{
    if( NULL == m_PreferencesDialog )
        m_PreferencesDialog = new PreferencesDialog( parent, *this );

    m_PreferencesDialog->ShowModal();
    
    delete m_PreferencesDialog;
    m_PreferencesDialog = NULL;
}

bool autopilot_route_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    piDC odc(dc);
    Render(odc, *vp);
    return true;
}

bool autopilot_route_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    piDC odc;
    glEnable( GL_BLEND );
    Render(odc, *vp);
    glDisable( GL_BLEND );
    return true;
}

void autopilot_route_pi::ShowConsoleCanvas()
{
    if( !m_ConsoleCanvas ) {
        wxFileConfig *pConf = GetOCPNConfigObject();
        pConf->SetPath ( _T( "/Settings/AutopilotRoute" ) );
        wxPoint pos(pConf->Read("PosX", 0L), pConf->Read("PosY", 50));
        
        m_ConsoleCanvas = new ConsoleCanvas( GetOCPNCanvasWindow(), *this );
        wxSize sz = m_ConsoleCanvas->GetSize();
        wxAuiPaneInfo p = wxAuiPaneInfo().
            Name( "Console Canvas" ).
            Caption( "Console Canvas" ).
            CaptionVisible( false ).
            TopDockable(false).
            BottomDockable(false).
            LeftDockable(true).
            RightDockable(true).
            MinSize(wxSize(20, 20)).
            BestSize(sz).
            FloatingSize(sz).
            FloatingPosition(pos).
            Float().
            Gripper(false);
        wxAuiManager *pauimgr = GetFrameAuiManager();
        pauimgr->AddPane(m_ConsoleCanvas, p);
        pauimgr->Update();
        SetColorScheme(m_colorscheme);
        m_ConsoleCanvas->Show();
    }

    m_ConsoleCanvas->ShowWithFreshFonts();
    GetFrameAuiManager()->GetPane(m_ConsoleCanvas).Show(true);
    GetFrameAuiManager()->Update();
    wxSize sz = GetFrameAuiManager()->GetPane(m_ConsoleCanvas).floating_size;
    sz.y++; // horrible stupid hack to make things look right
    GetFrameAuiManager()->GetPane(m_ConsoleCanvas).FloatingSize(sz);
    GetFrameAuiManager()->Update();

}

void autopilot_route_pi::ShowPreferences()
{
    if( !m_PreferencesDialog ) {
        m_PreferencesDialog = new PreferencesDialog( GetOCPNCanvasWindow(), *this );
        wxIcon icon;
        icon.CopyFromBitmap(*_img_autopilot_route);
        m_PreferencesDialog->SetIcon(icon);
    }

    m_PreferencesDialog->Show();
}

wxString autopilot_route_pi::StandardPath()
{
    wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
    wxString s = wxFileName::GetPathSeparator();

#if defined(__WXMSW__)
    wxString stdPath  = std_path.GetConfigDir();
#elif defined(__WXGTK__) || defined(__WXQT__)
    wxString stdPath  = std_path.GetUserDataDir();
#elif defined(__WXOSX__)
    wxString stdPath  = (std_path.GetUserConfigDir() + s + _T("opencpn"));
#endif

    stdPath += s + _T("plugins");
    if (!wxDirExists(stdPath))
      wxMkdir(stdPath);

    stdPath += s + _T("autopilot_route");

#ifdef __WXOSX__
    // Compatibility with pre-OCPN-4.2; move config dir to
    // ~/Library/Preferences/opencpn if it exists
    wxString oldPath = (std_path.GetUserConfigDir() + s + _T("plugins") + s + _T("weatherfax"));
    if (wxDirExists(oldPath) && !wxDirExists(stdPath)) {
	wxLogMessage("weatherfax_pi: moving config dir %s to %s", oldPath, stdPath);
	wxRenameFile(oldPath, stdPath);
    }
#endif

    if (!wxDirExists(stdPath))
      wxMkdir(stdPath);

    stdPath += s; // is this necessary?
    return stdPath;
}

double autopilot_route_pi::Declination()
{
    if(prefs.magnetic &&
       (!m_declinationTime.IsValid() || (wxDateTime::Now() - m_declinationTime).GetSeconds() > 1200)) {
        m_declination = NAN;
        SendPluginMessage("WMM_VARIATION_BOAT_REQUEST", "");
    }
    return m_declination;
}

bool autopilot_route_pi::GetConsoleInfo(double &sog, double &cog,
                                        double &bearing, double &xte,
                                        double *rng, double *nrng)
{
    sog = m_lastfix.Sog;
    cog = m_lastfix.Cog;
    bearing = m_current_bearing;
    xte = m_current_xte;

    if(rng) {
        double rbrg;
        DistanceBearing(m_lastfix.Lat, m_lastfix.Lon,
                        m_current_wp.lat, m_current_wp.lon, &rbrg, rng);
        *nrng = *rng * acos((rbrg - bearing)*M_PI/180);
    }
    
    return true;
}

void autopilot_route_pi::DeactivateRoute()
{
    SendPluginMessage("OCPN_RTE_DEACTIVATED", "");
}

void autopilot_route_pi::Render(piDC &dc, PlugIn_ViewPort &vp)
{
    if(m_active_guid.IsEmpty())
        return;
    
    if(prefs.mode != "Route Position Bearing")
        RenderArrivalWaypoint(dc, vp);
    else
        RenderRoutePositionBearing(dc, vp);
    
    wxPoint r1, r2;
    GetCanvasPixLL(&vp, &r1, m_current_wp.lat, m_current_wp.lon);
    GetCanvasPixLL(&vp, &r2, m_lastfix.Lat, m_lastfix.Lon);
    dc.SetPen(wxPen(*wxRED, 2));
    dc.DrawLine(r1.x, r1.y, r2.x, r2.y);
}

void autopilot_route_pi::RenderArrivalWaypoint(piDC &dc, PlugIn_ViewPort &vp)
{
    wxPoint r1, r2;
    dc.SetPen(wxPen(*wxGREEN, 2));
    GetCanvasPixLL(&vp, &r1, m_current_wp.lat, m_current_wp.lon);
    GetCanvasPixLL(&vp, &r2, m_current_wp.lat + m_current_wp.arrival_radius/60.0, m_current_wp.lon);

    double radius = hypot(r1.x-r2.x, r1.y-r2.y);
    dc.DrawCircle( r1.x, r1.y, radius );

    dc.SetPen(wxPen(*wxGREEN, 1));
    double lat, lon;
    double dist = 5 * m_current_wp.arrival_radius;
    
    ll_gc_ll(m_current_wp.lat, m_current_wp.lon, m_current_wp.arrival_bearing + 90, dist, &lat, &lon);
    GetCanvasPixLL(&vp, &r2, lat, lon);
    dc.DrawLine(r1.x, r1.y, r2.x, r2.y);
    
    ll_gc_ll(m_current_wp.lat, m_current_wp.lon, m_current_wp.arrival_bearing - 90, dist, &lat, &lon);
    GetCanvasPixLL(&vp, &r2, lat, lon);
    dc.DrawLine(r1.x, r1.y, r2.x, r2.y);
}

void autopilot_route_pi::RenderRoutePositionBearing(piDC &dc, PlugIn_ViewPort &vp)
{
    wxPoint r1;
    dc.SetPen(wxPen(*wxGREEN, 2));
    GetCanvasPixLL(&vp, &r1, m_current_wp.lat, m_current_wp.lon);
    dc.DrawCircle( r1.x, r1.y, 10 );
}

void autopilot_route_pi::OnTimer( wxTimerEvent & )
{
    if(m_active_guid.IsEmpty())
        return;

    // for now poll active route (not efficient)
    if((wxDateTime::Now() - m_active_request_time).GetSeconds() > 10)
        RequestRoute(m_active_guid);
    
    Recompute();

    m_ConsoleCanvas->UpdateRouteData();
    SendNMEA();
}

void autopilot_route_pi::Recompute()
{
    if(prefs.mode == "Standard XTE") ComputeXTE(); else
    if(prefs.mode == "Waypoint Bearing") ComputeWaypointBearing(); else
    if(prefs.mode == "Route Position Bearing") ComputeRoutePositionBearing(); else
        prefs.mode = "Route Position Bearing";
}

void autopilot_route_pi::SetCursorLatLon(double lat, double lon)
{
    wxPoint pos = wxGetMouseState().GetPosition();
    if(pos == m_cursor_position)
        return;
    m_cursor_position = pos;
}

void autopilot_route_pi::SetNMEASentence(wxString &sentence)
{
    // Check for conflicting autopilot messages
}

void autopilot_route_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{
    m_lastfix = pfix;
    if(pfix.nSats > 3)
        m_avg_sog = m_avg_sog*.9 + pfix.Sog*.1;
}

static bool ParseMessage(wxString &message_body, Json::Value &root)
{
    Json::Reader reader;
    if(reader.parse( std::string(message_body), root ))
        return true;
    
    wxLogMessage(wxString("autopilot_route_pi: Error parsing JSON message: ") + reader.getFormattedErrorMessages() );
    return false;
}


void autopilot_route_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
    // construct the JSON root object
    Json::Value  root;
    // construct a JSON parser
    wxString    out;
    
    if(message_id == wxS("AUTOPILOT_ROUTE_PI")) {
        return; // nothing yet
    } else if(message_id == wxS("AIS")) {
    } else if(message_id == _T("WMM_VARIATION_BOAT")) {
        if(ParseMessage( message_body, root )) {
            wxString(root["Decl"].asString()).ToDouble(&m_declination);
            m_declinationTime = wxDateTime::Now();
        }
    } else if(message_id == "OCPN_RTE_ACTIVATED") {
        if(ParseMessage( message_body, root )) {
            // when route is activated, request the route
            RequestRoute(root["GUID"].asString());
            ShowConsoleCanvas();
        }
    } else if(message_id == "OCPN_WPT_ACTIVATED") {
        wxString guid = root["GUID"].asString();
        m_last_wpt_activated_guid = guid;
        //ShowConsoleCanvas();
    } else if(message_id == "OCPN_WPT_ARRIVED") {
    } else if(message_id == "OCPN_RTE_DEACTIVATED" || message_id == "OCPN_RTE_ENDED") {
        m_Timer.Stop();
        m_active_guid = "";
        m_active_request_guid = "";
        if( m_ConsoleCanvas ) {
            GetFrameAuiManager()->GetPane(m_ConsoleCanvas).Float();
            GetFrameAuiManager()->GetPane(m_ConsoleCanvas).Show(false);
            GetFrameAuiManager()->Update();
        }
    } else if(message_id == "OCPN_ROUTE_RESPONSE") {
        if(!ParseMessage( message_body, root ))
            return;
        
        if(root["error"].asBool())
            return;
        
        wxString guid = root["GUID"].asString();
        if(guid != m_active_request_guid)
            return;
        
        m_active_request_time = wxDateTime::Now();        
        m_active_guid = guid;
        Json::Value w = root["waypoints"];
        int size = w.size();
        m_route.clear();
        double lat0 = m_lastfix.Lat, lon0 = m_lastfix.Lon;
        for(int i=0; i<size; i++) {
            double lat = w[i]["lat"].asDouble(), lon = w[i]["lon"].asDouble();
            double brg;
            DistanceBearing(lat0, lon0, lat, lon, &brg, 0);
            waypoint wp(lat, lon, w[i]["Name"].asString(), w[i]["GUID"].asString(),
                        w[i]["ArrivalRadius"].asDouble(), brg);
            
            // set arrival bearing to current course for first waypoint
            if(i == 0 && m_avg_sog > 1)
                wp.arrival_bearing = m_lastfix.Cog;
            m_route.push_back(wp);
            lat0 = lat, lon0 = lon;
        }
        
        // add interception points
        if(prefs.intercept_route) {
            ap_route_iterator it = m_route.begin(), closest;
            wp p0 = *it;
            wp boat(m_lastfix.Lat, m_lastfix.Lon);
            double mindist = INFINITY;
            // find closest segment
            for(it++; it!=m_route.end(); it++) {
                waypoint p1 = *it;
                wp x = ClosestSeg(boat, p0, p1);
                double dist = Distance(boat, x);
                if(dist < mindist) {
                    mindist = dist;
                    closest = it;
                }
                p0 = p1;
            }
            
            // do we intersect this segment on current course?
            wp p1 = *closest, w;
            closest--;
            p0 = *closest;
            // insert boat position after p0
            double brg;
            DistanceBearing(boat.lat, boat.lon, p0.lat, p0.lon, &brg, 0);
            waypoint boat_wp(boat.lat, boat.lon, "boat", "", 0, brg);
            m_route.insert(closest, boat_wp);
            
            // if intersect, insert this position
            if(Intersect(boat, m_lastfix.Cog, p0, p1, w)) {
                double brg;
                DistanceBearing(w.lat, w.lon, boat.lat, boat.lon, &brg, 0);                        waypoint i(w.lat, w.lon, "intersection", "",
                                                                                                              closest->arrival_radius, brg);
                m_route.insert(closest, i);
            }
        }
        
        m_current_wp.GUID = "";
        Recompute();
        m_Timer.Start(1000/prefs.rate);
    }
}

void autopilot_route_pi::RearrangeWindow()
{
    SetColorScheme(PI_ColorScheme());
}

void autopilot_route_pi::PositionBearing(double lat0, double lon0, double brg, double dist, double *dlat, double *dlon)
{
    if(prefs.computation == preferences::MERCATOR)
        PositionBearingDistanceMercator_Plugin(lat0, lon0, brg, dist, dlat, dlon);
    else
        ll_gc_ll(lat0, lon0, brg, dist, dlat, dlon);
}

void autopilot_route_pi::DistanceBearing(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist)
{
    if(prefs.computation == preferences::MERCATOR)
        DistanceBearingMercator_Plugin(lat0, lon0, lat1, lon1, brg, dist);
    else
        ll_gc_ll_reverse(lat0, lon0, lat1, lon1, brg, dist);
}


wp autopilot_route_pi::Closest(wp &p, wp &p0, wp &p1)
{
    if(prefs.computation == preferences::MERCATOR)
        return computation_mc::closest(p, p0, p1);
    return computation_gc::closest(p, p0, p1);
}
    
wp autopilot_route_pi::ClosestSeg(wp &p, wp &p0, wp &p1)
{
    if(prefs.computation == preferences::MERCATOR)
        return computation_mc::closest_seg(p, p0, p1);
    return computation_gc::closest_seg(p, p0, p1);
}

double autopilot_route_pi::Distance(wp &p0, wp &p1)
{
    if(prefs.computation == preferences::MERCATOR)
        return computation_mc::distance(p0, p1);
    return computation_gc::distance(p0, p1);
}

bool autopilot_route_pi::IntersectCircle(wp &p, double dist, wp &p0, wp &p1, wp &w)
{
    if(prefs.computation == preferences::MERCATOR)
        return computation_mc::intersect_circle(p, dist, p0, p1, w);
    return computation_gc::intersect_circle(p, dist, p0, p1, w);
}
        
bool autopilot_route_pi::Intersect(wp &p, double brg, wp &p0, wp &p1, wp &w)
{
    if(prefs.computation == preferences::MERCATOR)
        return computation_mc::intersect(p, brg, p0, p1, w);
    return computation_gc::intersect(p, brg, p0, p1, w);
}

void autopilot_route_pi::RequestRoute(wxString guid)
{
    Json::FastWriter w;
    Json::Value v;
    v["GUID"] = std::string(guid);
    m_active_request_guid = guid;
    SendPluginMessage("OCPN_ROUTE_REQUEST", w.write(v));
}

void autopilot_route_pi::AdvanceWaypoint()
{
    for(ap_route_iterator it=m_route.begin(); it!=m_route.end(); it++) {
        if(m_current_wp.GUID != it->GUID)
            continue;

        if(++it == m_route.end()) {
            // reached destination
            SendPluginMessage("OCPN_RTE_ENDED", "");
            DeactivateRoute();
        }
        if(prefs.confirm_bearing_change) {
            wxMessageDialog mdlg(GetOCPNCanvasWindow(), _("Advance Waypoint?"),
                                 _("Autopilot Route"), wxYES | wxNO);
            if(mdlg.ShowModal() == wxID_NO)
                break;
        }

        m_last_wp_name = m_current_wp.name;
        m_current_wp = *it;
        return;
    }
    // failed to advance waypoint
    DeactivateRoute();
}

void autopilot_route_pi::UpdateWaypoint()
{
    double bearing, dist;
    if(m_current_wp.GUID.IsEmpty()) {
        m_last_wp_name = "---";
        // activate nearest waypoint
        double mindist = INFINITY;
        for(ap_route_iterator it=m_route.begin(); it!=m_route.end(); it++) {
            ll_gc_ll_reverse(m_lastfix.Lat, m_lastfix.Lon, it->lat, it->lon, 0, &dist);
            if(dist < mindist) {
                m_current_wp = *it;
                mindist = dist;
            }
        }
    }

    ll_gc_ll_reverse(m_lastfix.Lat, m_lastfix.Lon, m_current_wp.lat, m_current_wp.lon,
                     &bearing, &dist);

    // if in the arrival radius, advance
    m_bArrival = dist < m_current_wp.arrival_radius;
    if(m_bArrival ||
       fabs(heading_resolve(m_current_wp.arrival_bearing - bearing)) > 90) {
        AdvanceWaypoint();
        UpdateWaypoint();
    }

    m_next_route_wp_GUID = m_current_wp.GUID; // for total calculations
    if(m_last_wpt_activated_guid != m_current_wp.GUID) {
        Json::FastWriter w;
        Json::Value v;
        v["GUID"] = std::string(m_last_wpt_activated_guid);
        SendPluginMessage("OCPN_WPT_ACTIVATED", w.write(v));
    }
}

double autopilot_route_pi::FindXTE()
{
    // find a position along this bearing
    double dlat, dlon, brg, xte;
    PositionBearing(m_current_wp.lat, m_current_wp.lon, m_current_wp.arrival_bearing, 1, &dlat, &dlon);
    wp b(m_lastfix.Lat, m_lastfix.Lon), w(dlat, dlon);
    wp p = Closest(b, m_current_wp, w);
    DistanceBearing(m_lastfix.Lat, m_lastfix.Lon, p.lat, p.lon, &brg, &xte);
    if(isnan(xte))
        xte = 0;
    else if(heading_resolve(brg - m_current_wp.arrival_bearing) < 0)
        xte = -xte;
    return xte;
}    

void autopilot_route_pi::ComputeXTE()
{
    UpdateWaypoint();

    double xte = FindXTE();
    m_current_xte = xte;
    
    m_current_bearing = m_current_wp.arrival_bearing;
    m_current_xte = xte*prefs.xte_multiplier;
}

void autopilot_route_pi::ComputeWaypointBearing()
{
    UpdateWaypoint();    
    DistanceBearing(m_lastfix.Lat, m_lastfix.Lon,
                    m_current_wp.lat, m_current_wp.lon,
                    &m_current_bearing, 0);
    m_current_xte = 0;
}

void autopilot_route_pi::ComputeRoutePositionBearing()
{
    double dist, bearing;

    // distance ahead to steer to
    dist = prefs.route_position_bearing_mode == preferences::TIME ?
        prefs.route_position_bearing_time*m_avg_sog/3600.0 :
        prefs.route_position_bearing_distance;

    // arrival radius only for final route point to deactivate route
    waypoint &finish = *m_route.rbegin();
    double finish_dist;
    DistanceBearing(m_lastfix.Lat, m_lastfix.Lon, finish.lat, finish.lon,
                    &bearing, &finish_dist);

    // if in the arrival radius for final route point or heading away, deactivate
    m_bArrival = finish_dist * 1852.0 < dist;
    if(m_bArrival ||
       (m_current_wp.eq(finish) && fabs(heading_resolve(finish.arrival_bearing - bearing)) > 90)) {
        // reached destination
        SendPluginMessage("OCPN_RTE_ENDED", "");
        DeactivateRoute();
    }

    wp boat(m_lastfix.Lat, m_lastfix.Lon);
    
    // find optimal position
    bool havew = false;
    ap_route_iterator it = m_route.begin();
    wp p0 = *it, w;
    for(it++; it!=m_route.end(); it++) {
        wp p1 = *it;
        if(IntersectCircle(boat, dist, p0, p1, w))
            havew = true;
        p0 = p1;
    }

    if(!havew) {
        // find closest position in route to boat
        it = m_route.begin();
        p0 = *it;
        double best_dist = INFINITY;
        for(it++; it!=m_route.end(); it++) {
            waypoint p1 = *it;
            wp x = ClosestSeg(boat, p0, p1);
            double dist = Distance(boat, x);
            if(dist < best_dist) {
                best_dist = dist;
                w = x;
                m_next_route_wp_GUID = p1.GUID; // for total calculations
            }
            p0 = p1;
        }
    }

    // compute bearing from position
    m_current_wp.lat = w.lat;
    m_current_wp.lon = w.lon;
    m_current_wp.GUID = "";

    DistanceBearing(m_lastfix.Lat, m_lastfix.Lon, m_current_wp.lat, m_current_wp.lon, &m_current_bearing, 0);

    // clamp to max angle
    double ang = heading_resolve(m_current_bearing - m_current_wp.arrival_bearing);
    double brg = m_current_wp.arrival_bearing;
    double max_angle = prefs.route_position_bearing_max_angle;
    if(ang > max_angle)
        m_current_bearing = brg + max_angle;
    else
    if(ang < -max_angle)
        m_current_bearing = brg - max_angle;
        
    m_current_xte = 0;
}

void autopilot_route_pi::MagneticHeading(double &val)
{
    val = heading_resolve(val+m_declination, 180);
}

NMEA0183    NMEA0183;

void autopilot_route_pi::SendRMB()
{
    if(!prefs.NmeaSentences("RMB"))
        return;

    NMEA0183.TalkerID = "EC"; // overrided by opencpn anyway

    SENTENCE snt;
    NMEA0183.Rmb.IsDataValid = NTrue;
    NMEA0183.Rmb.CrossTrackError = m_current_xte;
    NMEA0183.Xte.DirectionToSteer = m_current_xte < 0 ? Left : Right;
    NMEA0183.Rmb.To = m_current_wp.name.Truncate( 6 );
    NMEA0183.Rmb.From = m_last_wp_name.Truncate( 6 );

    NMEA0183.Rmb.DestinationPosition.Latitude.Set(
        fabs(m_current_wp.lat), m_current_wp.lat < 0 ? "S" : "N" );
    NMEA0183.Rmb.DestinationPosition.Longitude.Set(
        fabs(m_current_wp.lon), m_current_wp.lon < 0 ? "W" : "E" );

    double brg, dist;
    DistanceBearing(m_lastfix.Lat, m_lastfix.Lon, m_current_wp.lat, m_current_wp.lon,
                     &brg, &dist);
    NMEA0183.Rmb.RangeToDestinationNauticalMiles = dist;
    NMEA0183.Rmb.BearingToDestinationDegreesTrue = brg;
    NMEA0183.Rmb.DestinationClosingVelocityKnots = m_lastfix.Sog;

    NMEA0183.Rmb.IsArrivalCircleEntered = m_bArrival ? NTrue : NFalse;
    NMEA0183.Rmb.Write( snt );

    PushNMEABuffer( snt.Sentence  );
}

void autopilot_route_pi::SendRMC()
{
    if(!prefs.NmeaSentences("RMC"))
        return;

    NMEA0183.TalkerID = "EC"; // overrided by opencpn anyway

    SENTENCE snt;
    NMEA0183.Rmc.IsDataValid = NTrue;

    double lat = m_lastfix.Lat, lon = m_lastfix.Lon;
    NMEA0183.Rmc.Position.Latitude.Set( fabs(lat), lat < 0 ? "S" : "N" );
    NMEA0183.Rmc.Position.Longitude.Set( fabs(lon), lon < 0 ? "W" : "E" );

    NMEA0183.Rmc.SpeedOverGroundKnots = m_lastfix.Sog;
    NMEA0183.Rmc.TrackMadeGoodDegreesTrue = m_lastfix.Cog;

    if( !wxIsNaN(m_lastfix.Var) ) {
        NMEA0183.Rmc.MagneticVariation = fabs(m_lastfix.Var);
        NMEA0183.Rmc.MagneticVariationDirection = m_lastfix.Var < 0. ? West : East;
    } else
        NMEA0183.Rmc.MagneticVariation = 361.; // A signal to NMEA converter, gVAR is unknown

    wxDateTime now = wxDateTime::Now();
    wxDateTime utc = now.ToUTC();
    wxString time = utc.Format( _T("%H%M%S") );
    NMEA0183.Rmc.UTCTime = time;

    wxString date = utc.Format( _T("%d%m%y") );
    NMEA0183.Rmc.Date = date;

    NMEA0183.Rmc.Write( snt );
    PushNMEABuffer( snt.Sentence  );
}

void autopilot_route_pi::SendAPB()
{
    if(!prefs.NmeaSentences("APB"))
        return;
    
    // compute APB sentence
    NMEA0183.TalkerID = "EC"; // overrided by opencpn anyway
    NMEA0183.Apb.IsLoranBlinkOK = NTrue;
    NMEA0183.Apb.IsLoranCCycleLockOK = NTrue;
    NMEA0183.Apb.CrossTrackErrorMagnitude = fabs(m_current_xte);
    NMEA0183.Apb.DirectionToSteer = m_current_xte < 0 ? Left : Right;
    NMEA0183.Apb.CrossTrackUnits = "N";
    NMEA0183.Apb.IsArrivalCircleEntered = m_bArrival ? NTrue : NFalse;

    //  We never pass the perpendicular, since we declare arrival before reaching this point
    NMEA0183.Apb.IsPerpendicular = NFalse;
    NMEA0183.Apb.To = m_current_wp.name.Truncate( 6 );

    NMEA0183.Apb.BearingOriginToDestination = m_current_wp.arrival_bearing;

    double brg;
    DistanceBearing(m_lastfix.Lat, m_lastfix.Lon, m_current_wp.lat, m_current_wp.lon, &brg, 0);
    
    NMEA0183.Apb.BearingPresentPositionToDestination = brg;
            
    NMEA0183.Apb.HeadingToSteer = m_current_bearing;
    if( prefs.magnetic && !wxIsNaN(m_declination) ) {
        NMEA0183.Apb.BearingOriginToDestinationUnits = _T("M");
        NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("M");
        NMEA0183.Apb.HeadingToSteerUnits = _T("M");
        
        MagneticHeading(NMEA0183.Apb.BearingOriginToDestination);
        MagneticHeading(NMEA0183.Apb.BearingPresentPositionToDestination);
        MagneticHeading(NMEA0183.Apb.HeadingToSteer);
    } else {
        NMEA0183.Apb.BearingOriginToDestinationUnits = _T("T");
        NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("T");
        NMEA0183.Apb.HeadingToSteerUnits = _T("T");
    }
    SENTENCE snt;
    NMEA0183.Apb.Write( snt );
    PushNMEABuffer( snt.Sentence );
}

void autopilot_route_pi::SendXTE()
{
    if(!prefs.NmeaSentences("XTE"))
        return;
    
    // compute XTE sentence
    NMEA0183.TalkerID = "EC"; // overrided by opencpn anyway
            
    SENTENCE snt;
    NMEA0183.Xte.IsLoranBlinkOK = NTrue;
    NMEA0183.Xte.IsLoranCCycleLockOK = NTrue;
    NMEA0183.Xte.CrossTrackErrorDistance = fabs(m_current_xte);
    NMEA0183.Xte.DirectionToSteer = m_current_xte < 0 ? Left : Right;
    NMEA0183.Xte.CrossTrackUnits = _T("N");
    NMEA0183.Xte.Write( snt );
    PushNMEABuffer( snt.Sentence  );
}

void autopilot_route_pi::SendNMEA()
{
    SendRMB();
    SendRMC();
    SendAPB();
    SendXTE();
}
