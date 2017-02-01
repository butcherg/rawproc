#include "myPropertyDialog.h"
#include <wx/sizer.h>

#include <wx/wx.h>

PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::map<std::string, std::string> props, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size, wxRESIZE_BORDER)
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);
	pg = new wxPropertyGrid(this, wxID_ANY);
	SetExtraStyle(GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS);
	for (std::map<std::string, std::string>::iterator it=props.begin(); it!=props.end(); ++it)
		pg->Append(new wxStringProperty(it->first.c_str(), it->first.c_str(), it->second.c_str()));
	pg->Sort();
	sz->Add(pg, 0, wxEXPAND | wxALL, 3);
	
	ct->Add(new wxButton(this, wxID_OK, "Dismiss"), 0, wxALL, 10);
	ct->Add(new wxStaticText(this, wxID_ANY, "Filter: "), 0, wxALL, 10);
	fil = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	Bind(wxEVT_TEXT_ENTER, &PropertyDialog::FilterGrid, this);
}


PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxFileConfig *config, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size, wxRESIZE_BORDER)
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);
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

	ct->Add(new wxButton(this, wxID_OK, "Dismiss"), 0, wxALL, 10);
	ct->Add(new wxStaticText(this, wxID_ANY, "Filter: "), 0, wxALL, 10);
	fil = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	Bind(wxEVT_PG_CHANGED,&PropertyDialog::UpdateProperty,this);
	Bind(wxEVT_TEXT_ENTER, &PropertyDialog::FilterGrid, this);
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

void PropertyDialog::FilterGrid(wxCommandEvent& event)
{
	wxString str = fil->GetValue();
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
    		wxPGProperty* p = *it;
		if (str == "") {
			p->Hide(false);
		}
		else {
			if (p->GetName().Find(str) == wxNOT_FOUND) {
				p->Hide(true);
			}
			else {
				p->Hide(false);
			}
		}
	}
}


