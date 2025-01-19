///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/choicebk.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/dialog.h>

#include "wxWTranslateCatalog.h"

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class PreferencesDialogBase
///////////////////////////////////////////////////////////////////////////////
class PreferencesDialogBase : public wxDialog
{
	private:

	protected:
		wxChoicebook* m_cbMode;
		wxPanel* m_panel5;
		wxStaticText* m_staticText57;
		wxSpinCtrlDouble* m_sXTEP;
		wxStaticText* m_staticText561;
		wxPanel* m_panel8;
		wxPanel* m_panel9;
		wxChoicebook* m_cbRoutePositionBearingMode;
		wxPanel* m_panel91;
		wxSpinCtrl* m_sRoutePositionBearingDistance;
		wxStaticText* m_staticText74;
		wxStaticText* m_staticText10;
		wxSpinCtrl* m_sRoutePositionBearingMaxAngle;
		wxPanel* m_panel10;
		wxSpinCtrl* m_sRoutePositionBearingTime;
		wxStaticText* m_staticText30;
		wxChoice* m_cRate;
		wxStaticText* m_staticText13;
		wxCheckBox* m_cbMagnetic;
		wxCheckListBox* m_cbNMEASentences;
		wxCheckBox* m_cbBoundary;
		wxTextCtrl* m_tBoundary;
		wxButton* m_button22;
		wxStaticText* m_staticText71;
		wxSpinCtrl* m_sBoundaryWidth;
		wxStaticText* m_staticText72;
		wxCheckBox* m_cbConfirmBearingChange;
		wxCheckBox* m_cbInterceptRoute;
		wxChoice* m_cComputation;
		wxCheckListBox* m_cbActiveRouteItems0;
		wxCheckListBox* m_cbActiveRouteItems1;
		wxButton* m_button4;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnMode( wxChoicebookEvent& event ) { event.Skip(); }
		virtual void OnInformation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }


	public:

		PreferencesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Autopilot Route Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );

		~PreferencesDialogBase();

};

