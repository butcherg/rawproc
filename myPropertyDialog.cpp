#include "myPropertyDialog.h"
#include <wx/sizer.h>

PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::map<std::string, std::string> props, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size)
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	pg = new wxPropertyGrid(this, wxID_ANY);
	pg->SetExtraStyle(pg->GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS);
	for (std::map<std::string, std::string>::iterator it=props.begin(); it!=props.end(); ++it)
		pg->Append(new wxStringProperty(it->first.c_str(), it->first.c_str(), it->second.c_str()));
	pg->FitColumns();
	sz->Add(pg, 0, wxEXPAND | wxALL, 3);
	sz->Add(new wxButton(this, wxID_OK, "Dismiss"), 0, wxALL, 10);
	SetSizerAndFit(sz);
}


PropertyDialog::~PropertyDialog()
{
	if (pg) delete pg;
}

/*
void PropertyDialog::OnPropertyChange(wxPropertyGridEvent& event)
{

}
*/