#ifndef MYINTEGERCTRL_H_
#define MYINTEGERCTRL_H_

#include <wx/wx.h>

wxDECLARE_EVENT(myINTEGERCTRL_UPDATE, wxCommandEvent);
wxDECLARE_EVENT(myINTEGERCTRL_CHANGE, wxCommandEvent);

class myIntegerCtrl: public wxControl
{
	public:
		myIntegerCtrl(wxWindow *parent, wxWindowID id, int value=0, int lower=-10, int upper=10, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		myIntegerCtrl(wxWindow *parent, wxWindowID id, wxString label, int value=0, int lower=-10, int upper=10, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		int GetIntegerValue();
		void SetIntegerValue(int value);
		void SetToolTip(const wxString &tipString);
		void OnWheel(wxMouseEvent& event);
		void OnEnter(wxCommandEvent& event);
		void OnMouseEnter(wxMouseEvent& event);
		void OnMouseLeave(wxMouseEvent& event);

	private:
		int v, l, u;
		unsigned p;
		wxString fmt;
		wxTextCtrl *textbox;
	
};

#endif
