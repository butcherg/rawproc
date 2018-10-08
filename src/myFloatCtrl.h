#ifndef MYFLOATCTRL_H_
#define MYFLOATCTRL_H_

#include <wx/wx.h>


class myFloatCtrl: public wxControl
{
	public:
		myFloatCtrl(wxWindow *parent, wxWindowID id, float value=0.0, unsigned precision=1, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
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
