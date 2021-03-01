#include "myIntegerCtrl.h"
#include "myConfig.h"
#include "util.h"

wxDEFINE_EVENT(myINTEGERCTRL_UPDATE, wxCommandEvent);
wxDEFINE_EVENT(myINTEGERCTRL_CHANGE, wxCommandEvent);


myIntegerCtrl::myIntegerCtrl(wxWindow *parent, wxWindowID id, wxString label, int value, int lower, int upper, const wxPoint &pos, const wxSize &size): wxControl(parent, id, pos, size, wxBORDER_NONE)
{
	l = lower;
	u = upper;
	v = value;
	fmt = "%d";
	SetBackgroundColour(parent->GetBackgroundColour());	
	wxBoxSizer *b = new wxBoxSizer(wxHORIZONTAL);
	textbox = new wxTextCtrl(this, wxID_ANY, wxString::Format(fmt,value), pos, size, wxTE_PROCESS_ENTER);
	b->Add(new wxStaticText(this, wxID_ANY, label),0,wxALL|wxALIGN_CENTER_VERTICAL,0);
	b->Add(textbox,0,wxALL,0);
	//parm app.integerctrl.backgroundcolor: Specifies the integer control background color. Can be a RGB triple or a single gray, 0-255.  Default=192,192,255
	wxColour tcolor = wxString2wxColour(wxString(myConfig::getConfig().getValueOrDefault("app.integerctrl.backgroundcolor","192,192,255")));  
	textbox->SetBackgroundColour(tcolor);
	SetSizerAndFit(b);
	Bind(wxEVT_MOUSEWHEEL, &myIntegerCtrl::OnWheel, this);
	Bind(wxEVT_TEXT_ENTER, &myIntegerCtrl::OnEnter, this);
}

myIntegerCtrl::myIntegerCtrl(wxWindow *parent, wxWindowID id, int value, int lower, int upper, const wxPoint &pos, const wxSize &size): wxControl(parent, id, pos, size, wxBORDER_NONE)
{
	l = lower;
	u = upper;
	v = value;
	fmt = "%d";
	SetBackgroundColour(parent->GetBackgroundColour());
	wxBoxSizer *b = new wxBoxSizer(wxVERTICAL);
	textbox = new wxTextCtrl(this, wxID_ANY, wxString::Format(fmt,value), pos, size, wxTE_PROCESS_ENTER);
	b->Add(textbox,0,wxALL,0);
	wxColour tcolor = wxString2wxColour(wxString(myConfig::getConfig().getValueOrDefault("app.integerctrl.backgroundcolor","192,192,255")));  
	textbox->SetBackgroundColour(tcolor);
	SetSizerAndFit(b);
	Bind(wxEVT_MOUSEWHEEL, &myIntegerCtrl::OnWheel, this);
	Bind(wxEVT_TEXT_ENTER, &myIntegerCtrl::OnEnter, this);
}

void myIntegerCtrl::SetToolTip(const wxString &tipString)
{
	textbox->SetToolTip(tipString);
}
		
int myIntegerCtrl::GetIntegerValue()
{
	v = atoi(textbox->GetValue().c_str());
	return v;
}

void myIntegerCtrl::SetIntegerValue(int value)
{
	v = value;
	textbox->SetValue(wxString::Format(fmt,value));
}
	
void myIntegerCtrl::OnWheel(wxMouseEvent& event)
{
	if (textbox->HasFocus()) { 
		v = atoi(textbox->GetValue().c_str());
		double inc = 1;
		if (event.ShiftDown()) inc = 10;
		if (event.ControlDown()) inc = 100;
		if (event.GetWheelRotation() > 0) { 
			v += inc;
		}
		else {
			v -= inc;
		}
		if (v > u) v = u;
		if (v < l) v = l;
		textbox->SetValue(wxString::Format(fmt,v));
		textbox->Refresh();
		wxCommandEvent e(myINTEGERCTRL_CHANGE);
		e.SetEventObject(this);
		e.SetString("change");
		ProcessWindowEvent(e);
	}
	else event.Skip();
}

void myIntegerCtrl::OnEnter(wxCommandEvent& event)
{
	v = atoi(textbox->GetValue().c_str());
	wxCommandEvent e(myINTEGERCTRL_UPDATE);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
}
		

