///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec  3 2017)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __AUTOPILOTROUTEUI_H__
#define __AUTOPILOTROUTEUI_H__

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
#include <wx/checklst.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
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
		wxSpinCtrl* m_sXTEP;
		wxStaticText* m_staticText561;
		wxStaticText* m_staticText571;
		wxSpinCtrl* m_sXTED;
		wxStaticText* m_staticText56;
		wxPanel* m_panel7;
		wxStaticText* m_staticText70;
		wxSpinCtrl* m_sXTEBoundaryP;
		wxStaticText* m_staticText20;
		wxPanel* m_panel8;
		wxPanel* m_panel9;
		wxChoicebook* m_cbRoutePositionBearingMode;
		wxPanel* m_panel91;
		wxSpinCtrl* m_sRoutePositionBearingDistance;
		wxStaticText* m_staticText74;
		wxPanel* m_panel10;
		wxSpinCtrl* m_sRoutePositionBearingTime;
		wxStaticText* m_staticText30;
		wxCheckListBox* m_cbActiveRouteItems;
		wxStaticText* m_staticText12;
		wxTextCtrl* m_tTalkerID;
		wxCheckListBox* m_cbNMEASentences;
		wxStaticText* m_staticText29;
		wxTextCtrl* m_tBoundary;
		wxButton* m_button22;
		wxStaticText* m_staticText71;
		wxSpinCtrl* m_sBoundaryWidth;
		wxStaticText* m_staticText72;
		wxCheckBox* m_cbConfirmBearingChange;
		wxButton* m_button4;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnMode( wxChoicebookEvent& event ) { event.Skip(); }
		virtual void OnInformation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		PreferencesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Autopilot Route Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL ); 
		~PreferencesDialogBase();
	
};

#endif //__AUTOPILOTROUTEUI_H__
