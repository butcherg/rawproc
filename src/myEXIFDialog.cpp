#include "myEXIFDialog.h"
#include <wx/sizer.h>


myEXIFDialog::myEXIFDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxString &exif, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE) // | wxRESIZE_BORDER)
{
	s = new wxBoxSizer( wxVERTICAL );
	html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, size);
	html->SetPage(exif);
	s->Add(html, 1, wxEXPAND | wxALL, 10);
	wxButton *ok = new wxButton(this, wxID_OK, "Dismiss", wxDefaultPosition, wxDefaultSize);
	s->Add(ok, 0, wxALL, 10);
	SetSizerAndFit(s);

}


myEXIFDialog::~myEXIFDialog()
{
	if (html) delete html;
}



