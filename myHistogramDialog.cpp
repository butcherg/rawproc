#include "myHistogramDialog.h"
#include <wx/sizer.h>

#define FILTERID 8400

myHistogramDialog::myHistogramDialog(wxWindow *parent, wxWindowID id, const wxString &title, gImage &dib, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE) // | wxRESIZE_BORDER)
{
	s = new wxBoxSizer( wxVERTICAL );
	hpane = new myHistogramPane(this, dib, wxDefaultPosition,  wxDefaultSize); //wxSize(500,400));
	s->Add(hpane, 1, wxEXPAND | wxALL, 10);
	//s->Add(hpane,wxSizerFlags(100).Expand().FixedMinSize ());
	wxButton *ok = new wxButton(this, wxID_OK, "Dismiss", wxDefaultPosition, wxDefaultSize); //wxSize(50,20));
	s->Add(ok, 0, wxALL, 10);
	SetSizerAndFit(s);
	Bind(wxEVT_SIZE, &myHistogramDialog::OnSize, this);
}

void myHistogramDialog::OnSize(wxSizeEvent& event) 
{
	s->SetSizeHints(this);
	s->Layout();
}


myHistogramDialog::~myHistogramDialog()
{
	if (hpane) delete hpane;
}



