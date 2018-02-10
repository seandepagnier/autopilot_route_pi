/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  autopilot_route Plugin
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

#include "PreferencesDialog.h"

bool PreferencesDialog::Show( bool show )
{
    if(show) {

        // load preferences
        autopilot_route_pi::preferences &p = m_pi.prefs;

        // Mode
        m_cbMode->SetSelection(p.mode);
        m_sXTEP->SetValue(p.xte_multiplier*100);
        m_sXTED->SetValue(p.xte_rate_multiplier*100);
        m_sXTEBoundaryP->SetValue(p.xte_boundary_multiplier*100);
        m_cbRoutePositionBearingMode->SetSelection(p.route_position_bearing_mode);
        m_sRoutePositionBearingDistance->SetValue(p.route_position_bearing_distance);
        m_sRoutePositionBearingTime->SetValue(p.route_position_bearing_time);
    
        // Active Route Window
        wxCheckListBox *cbActiveRouteItems[2] = {m_cbActiveRouteItems0, m_cbActiveRouteItems1};
        for(unsigned int ind = 0; ind < 2; ind++)
            for(unsigned int i=0; i<cbActiveRouteItems[ind]->GetCount(); i++)
                if(p.active_route_labels[ind].find(cbActiveRouteItems[ind]->GetString(i))
                   != p.active_route_labels[ind].end())
                    cbActiveRouteItems[ind]->Check
                        (i, p.active_route_labels[ind][cbActiveRouteItems[ind]->GetString(i)]);

        // Waypoint Arrival
        m_cbConfirmBearingChange->SetValue(p.confirm_bearing_change);

        // Boundary
        m_tBoundary->SetValue(p.boundary_guid);
        m_sBoundaryWidth->SetValue(p.boundary_width);

        // NMEA output
        long l;
        m_cRate->SetSelection(0);
        for(unsigned int i=0; i<m_cRate->GetCount(); i++)
            if(m_cRate->GetString(i).ToLong(&l) && l == p.rate) {
                m_cRate->SetSelection(i);
                break;
            }

        m_cbMagnetic->SetValue(p.magnetic);
        for(unsigned int i=0; i<m_cbNMEASentences->GetCount(); i++)
            if(p.nmea_sentences.find(m_cbNMEASentences->GetString(i)) != p.nmea_sentences.end())
                m_cbNMEASentences->Check(i, p.nmea_sentences[m_cbNMEASentences->GetString(i)]);
    }
    return PreferencesDialogBase::Show(show);
}


void PreferencesDialog::OnMode( wxChoicebookEvent& event )
{
    if(m_cbMode->GetSelection() == 0)
        m_tBoundary->Disable();
    else
        m_tBoundary->Enable();
}

void PreferencesDialog::OnInformation( wxCommandEvent& event )
{
    wxLaunchDefaultBrowser(_T("http://www.github.com/seandepagnier/autopilot_route_pi"));
}

void PreferencesDialog::OnCancel( wxCommandEvent& event )
{
    if(IsModal())
        EndModal(wxID_CANCEL);
    Hide();
}

void PreferencesDialog::OnOk( wxCommandEvent& event )
{
    autopilot_route_pi::preferences &p = m_pi.prefs;

    // Mode
    p.mode = (autopilot_route_pi::preferences::Mode)m_cbMode->GetSelection();
    p.xte_multiplier = m_sXTEP->GetValue() / 100.0;
    p.xte_rate_multiplier = m_sXTED->GetValue() / 100.0;
    p.xte_boundary_multiplier = m_sXTEBoundaryP->GetValue() / 100.0;
    p.route_position_bearing_mode = (autopilot_route_pi::preferences::RoutePositionBearingMode)
        m_cbRoutePositionBearingMode->GetSelection();
    p.route_position_bearing_distance = m_sRoutePositionBearingDistance->GetValue();
    p.route_position_bearing_time = m_sRoutePositionBearingTime->GetValue();
    
    // Active Route Window
    wxCheckListBox *cbActiveRouteItems[2] = {m_cbActiveRouteItems0, m_cbActiveRouteItems1};
    for(unsigned int ind = 0; ind < 2; ind++)
        for(unsigned int i=0; i<cbActiveRouteItems[ind]->GetCount(); i++)
            p.active_route_labels[ind][cbActiveRouteItems[ind]->GetString(i)]
                = cbActiveRouteItems[ind]->IsChecked(i);

    // Waypoint Arrival
    p.confirm_bearing_change = m_cbConfirmBearingChange->GetValue();

    // Boundary
    p.boundary_guid = m_tBoundary->GetValue();
    p.boundary_width = m_sBoundaryWidth->GetValue();

    // NMEA output
    long l;
    if(m_cRate->GetStringSelection().ToLong(&l))
        p.rate = l;
    p.magnetic = m_cbMagnetic->GetValue();
    for(unsigned int i=0; i<m_cbNMEASentences->GetCount(); i++)
        p.nmea_sentences[m_cbNMEASentences->GetString(i)] = m_cbNMEASentences->IsChecked(i);

    if(IsModal())
        EndModal(wxID_OK);
    else {
        m_pi.ShowConsoleCanvas(); // update fields
        Hide();
    }
}
