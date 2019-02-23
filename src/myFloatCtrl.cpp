#include "myFloatCtrl.h"

wxDEFINE_EVENT(myFLOATCTRL_UPDATE, wxCommandEvent);
wxDEFINE_EVENT(myFLOATCTRL_CHANGE, wxCommandEvent);


myFloatCtrl::myFloatCtrl(wxWindow *parent, wxWindowID id, float value, unsigned precision, const wxPoint &pos, const wxSize &size): wxControl(parent, id, pos, size, wxBORDER_NONE)
{
	v = value;
	p = precision;
	fmt = "%0.";
	fmt.Append(wxString::Format("%d",p));
	fmt.Append("f");
	wxBoxSizer *b = new wxBoxSizer(wxVERTICAL);
	textbox = new wxTextCtrl(this, wxID_ANY, wxString::Format(fmt,value), pos, size, wxTE_PROCESS_ENTER);
	b->Add(textbox,0,wxALL,0);
	SetSizerAndFit(b);
	Bind(wxEVT_MOUSEWHEEL, &myFloatCtrl::OnWheel, this);
	Bind(wxEVT_TEXT_ENTER, &myFloatCtrl::OnEnter, this);
}

myFloatCtrl::myFloatCtrl(wxWindow *parent, wxWindowID id, wxString label, float value, unsigned precision, const wxPoint &pos, const wxSize &size, bool labelleft): wxControl(parent, id, pos, size, wxBORDER_NONE)
{
	v = value;
	p = precision;
	fmt = "%0.";
	SetBackgroundColour(parent->GetBackgroundColour());	
	fmt.Append(wxString::Format("%d",p));
	fmt.Append("f");
	wxBoxSizer *b = new wxBoxSizer(wxHORIZONTAL);
	if (labelleft) b->Add(new wxStaticText(this, wxID_ANY, label),0,wxALL|wxALIGN_CENTER_VERTICAL,0);
	textbox = new wxTextCtrl(this, wxID_ANY, wxString::Format(fmt,value), pos, size, wxTE_PROCESS_ENTER);
	if (!labelleft) b->Add(new wxStaticText(this, wxID_ANY, label),0,wxALL|wxALIGN_CENTER_VERTICAL,0);
	b->Add(textbox,0,wxALL,0);
	SetSizerAndFit(b);
	Bind(wxEVT_MOUSEWHEEL, &myFloatCtrl::OnWheel, this);
	Bind(wxEVT_TEXT_ENTER, &myFloatCtrl::OnEnter, this);
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
	
void myFloatCtrl::OnWheel(wxMouseEvent& event)
{
	v = atof(textbox->GetValue().c_str());
	double inc = pow(10,-((float)p));
	if (event.ShiftDown()) inc *= 10.0;
	if (event.ControlDown()) inc *= 100.0;
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

void myFloatCtrl::OnEnter(wxCommandEvent& event)
{
	v = atof(textbox->GetValue().c_str());
	wxCommandEvent e(myFLOATCTRL_UPDATE);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
}
		

