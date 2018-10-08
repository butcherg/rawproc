#include "myIntegerCtrl.h"


myIntegerCtrl::myIntegerCtrl(wxWindow *parent, wxWindowID id, int value, int lower, int upper, const wxPoint &pos, const wxSize &size): wxControl(parent, id, pos, size, wxBORDER_NONE)
{
	l = lower;
	u = upper;
	v = value;
	fmt = "%d";
	wxBoxSizer *b = new wxBoxSizer(wxVERTICAL);
	textbox = new wxTextCtrl(this, wxID_ANY, wxString::Format(fmt,value), pos, size, wxTE_PROCESS_ENTER);
	b->Add(textbox,0,wxALL,0);
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
	return v;
}

void myIntegerCtrl::SetIntegerValue(int value)
{
	v = value;
	textbox->SetValue(wxString::Format(fmt,value));
}
	
void myIntegerCtrl::OnWheel(wxMouseEvent& event)
{
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
	event.Skip();
}

void myIntegerCtrl::OnEnter(wxCommandEvent& event)
{
	v = atoi(textbox->GetValue().c_str());
	event.Skip();
}
		

