#include "myPropertyDialog.h"
#include <wx/sizer.h>

#include <wx/wx.h>

PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::map<std::string, std::string> props, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size, wxRESIZE_BORDER)
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	pg = new wxPropertyGrid(this, wxID_ANY);
	SetExtraStyle(GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS);
	for (std::map<std::string, std::string>::iterator it=props.begin(); it!=props.end(); ++it)
		pg->Append(new wxStringProperty(it->first.c_str(), it->first.c_str(), it->second.c_str()));
	pg->Sort();
	sz->Add(pg, 0, wxEXPAND | wxALL, 3);
	sz->Add(new wxButton(this, wxID_OK, "Dismiss"), 0, wxALL, 10);
	SetSizerAndFit(sz);
}


PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxFileConfig *config, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size, wxRESIZE_BORDER)
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	pg = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_BOLD_MODIFIED | wxPG_HIDE_MARGIN);
	SetExtraStyle(GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS);
	wxString name, val; 
	long dummy;
	bool bCont = config->GetFirstEntry(name, dummy);
	while ( bCont ) {
		val = config->Read(name, "");
		pg->Append(new wxStringProperty(name, name, val));
		bCont = config->GetNextEntry(name, dummy);
	}

	pg->Sort();
	sz->Add(pg, 0, wxEXPAND | wxALL, 3);
	sz->Add(new wxButton(this, wxID_OK, "Dismiss"), 0, wxALL, 10);
	SetSizerAndFit(sz);
	Bind(wxEVT_PG_CHANGED,&PropertyDialog::UpdateProperty,this);
}


PropertyDialog::~PropertyDialog()
{
	if (pg) delete pg;
}

void PropertyDialog::UpdateProperty(wxPropertyGridEvent& event)
{
	Refresh();
	event.Skip();
}

