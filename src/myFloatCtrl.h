#ifndef MYFLOATCTRL_H_
#define MYFLOATCTRL_H_

#include <wx/wx.h>

wxDECLARE_EVENT(myFLOATCTRL_UPDATE, wxCommandEvent);
wxDECLARE_EVENT(myFLOATCTRL_CHANGE, wxCommandEvent);


class myFloatCtrl: public wxControl
{
	public:
		myFloatCtrl(wxWindow *parent, wxWindowID id, float value=0.0, unsigned precision=1, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		myFloatCtrl(wxWindow *parent, wxWindowID id, wxString label, float value=0.0, unsigned precision=1, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, bool labelleft=true);

		float GetFloatValue();
		void SetFloatValue(double value);
		void OnWheel(wxMouseEvent& event);
		void OnEnter(wxCommandEvent& event);

	private:
		double v;
		unsigned p;
		wxString fmt;
		wxTextCtrl *textbox;
	
};

#endif
