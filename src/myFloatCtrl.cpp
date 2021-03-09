#include "myFloatCtrl.h"
#include "myConfig.h"
#include "util.h"

wxDEFINE_EVENT(myFLOATCTRL_UPDATE, wxCommandEvent);
wxDEFINE_EVENT(myFLOATCTRL_CHANGE, wxCommandEvent);


myFloatCtrl::myFloatCtrl(wxWindow *parent, wxWindowID id, float value, unsigned precision, const wxPoint &pos, const wxSize &size): wxControl(parent, id, pos, size, wxBORDER_NONE)
{
	v = value;
	p = precision;
	incr = 0.0;
	fmt = "%0.";
	SetBackgroundColour(parent->GetBackgroundColour());	
	fmt.Append(wxString::Format("%d",p));
	fmt.Append("f");
	wxBoxSizer *b = new wxBoxSizer(wxVERTICAL);
	textbox = new wxTextCtrl(this, wxID_ANY, wxString::Format(fmt,value), pos, size, wxTE_PROCESS_ENTER);
	b->Add(textbox,0,wxALL,0);
	//parm app.floatctrl.backgroundcolor: Specifies the float control background color. Can be a RGB triple or a single gray, 0-255.  Default=192,255,192
	wxColour tcolor = wxString2wxColour(wxString(myConfig::getConfig().getValueOrDefault("app.floatctrl.backgroundcolor","192,255,192")));  
	textbox->SetBackgroundColour(tcolor);
	SetSizerAndFit(b);
	Bind(wxEVT_MOUSEWHEEL, &myFloatCtrl::OnWheel, this);
	Bind(wxEVT_TEXT_ENTER, &myFloatCtrl::OnEnter, this);
	textbox->Bind(wxEVT_ENTER_WINDOW, &myFloatCtrl::OnMouseEnter, this);
	textbox->Bind(wxEVT_LEAVE_WINDOW, &myFloatCtrl::OnMouseLeave, this);
}


myFloatCtrl::myFloatCtrl(wxWindow *parent, wxWindowID id, wxString label, float value, unsigned precision, const wxPoint &pos, const wxSize &size, bool labelleft): wxControl(parent, id, pos, size, wxBORDER_NONE)
{
	v = value;
	p = precision;
	incr = 0.0;
	fmt = "%0.";
	SetBackgroundColour(parent->GetBackgroundColour());	
	fmt.Append(wxString::Format("%d",p));
	fmt.Append("f");
	wxBoxSizer *b = new wxBoxSizer(wxHORIZONTAL);
	if (labelleft) b->Add(new wxStaticText(this, wxID_ANY, label),0,wxALL|wxALIGN_CENTER_VERTICAL,3);
	textbox = new wxTextCtrl(this, wxID_ANY, wxString::Format(fmt,value), pos, size, wxTE_PROCESS_ENTER);
	if (!labelleft) b->Add(new wxStaticText(this, wxID_ANY, label),0,wxALL|wxALIGN_CENTER_VERTICAL,0);
	b->Add(textbox,0,wxALL,0);
	wxColour tcolor = wxString2wxColour(wxString(myConfig::getConfig().getValueOrDefault("app.floatctrl.backgroundcolor","192,255,192")));
	textbox->SetBackgroundColour(tcolor);
	SetSizerAndFit(b);
	Bind(wxEVT_MOUSEWHEEL, &myFloatCtrl::OnWheel, this);
	Bind(wxEVT_TEXT_ENTER, &myFloatCtrl::OnEnter, this);
	textbox->Bind(wxEVT_ENTER_WINDOW, &myFloatCtrl::OnMouseEnter, this);
	textbox->Bind(wxEVT_LEAVE_WINDOW, &myFloatCtrl::OnMouseLeave, this);
}
		
float myFloatCtrl::GetFloatValue()
{
	v = atof(textbox->GetValue().c_str());
	return v;
}

void myFloatCtrl::SetFloatValue(double value)
{
	v = value;
	textbox->SetValue(wxString::Format(fmt,value));
}

void myFloatCtrl::SetIncrement(double increment)
{
 	incr = increment;
}

void myFloatCtrl::OnMouseEnter(wxMouseEvent& event)
{
	//parm app.floatctrl.mousefocus: Toggles the focus-on-mouseenter behavior. Default=0, require mouse click to set focus.  Note: Setting this parameter to 1 may cause inopportune parameter changing... 
	if (myConfig::getConfig().getValueOrDefault("app.floatctrl.mousefocus","0") == "1") {
		textbox->SetFocus();
		textbox->SetInsertionPoint(0);
	}
	event.Skip();
}

void myFloatCtrl::OnMouseLeave(wxMouseEvent& event)
{
	GetParent()->SetFocus();
	event.Skip();
}

	
void myFloatCtrl::OnWheel(wxMouseEvent& event)
{
	double inc;
	if (textbox->HasFocus()) { 
		v = atof(textbox->GetValue().c_str());
		if (incr > 0.0) {
			inc = incr;
		}
		else {
			inc = pow(10,-((float)p));
			if (event.ShiftDown()) inc *= 10.0;
			if (event.ControlDown()) inc *= 100.0;
		}
		if (event.GetWheelRotation() > 0) { 
			v += inc;
		}
		else {
			v -= inc;
		}
		textbox->SetValue(wxString::Format(fmt,v));
		textbox->Refresh();
		wxCommandEvent e(myFLOATCTRL_CHANGE);
		e.SetEventObject(this);
		e.SetString("change");
		ProcessWindowEvent(e);
	}
	else event.Skip();
}

void myFloatCtrl::OnEnter(wxCommandEvent& event)
{
	v = atof(textbox->GetValue().c_str());
	wxCommandEvent e(myFLOATCTRL_UPDATE);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
}
		

