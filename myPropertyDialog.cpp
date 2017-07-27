#include "myPropertyDialog.h"
#include <wx/sizer.h>

#include <wx/wx.h>

#define FILTERID 8400
#define ADDID 8401
#define DELETEID 8402
#define HIDEID 8403

class AddDialog: public wxDialog
{
	public:
		AddDialog(wxWindow *parent, wxWindowID id): 
		wxDialog(parent, id, "Add Property", wxDefaultPosition, wxDefaultSize) //wxSize(220,300))
		{
			wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
			wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);
			
			sz->Add(new wxStaticText(this, wxID_ANY, "Name: "), 0, wxLEFT, 5);
			name = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250,25),wxTE_PROCESS_ENTER);
			sz->Add(name, 0, wxLEFT, 5);
			sz->Add(new wxStaticText(this, wxID_ANY, "Value: "), 0, wxLEFT, 5);
			value = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250,25),wxTE_PROCESS_ENTER);
			sz->Add(value, 0, wxLEFT, 5);
			
			ct->Add(new wxButton(this, wxID_OK, "Ok"), 0, wxALL, 10);
			ct->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
			sz->Add(ct, 0, wxALL, 10);
			SetSizerAndFit(sz);
		}
		
		~AddDialog()
		{
			if (name) delete name;
			if (value) delete value;
		}
		
		wxString GetName()
		{
			return name->GetValue();
		}
		
		wxString GetValue()
		{
			return value->GetValue();
		}
		
		
		
	private:
		wxTextCtrl *name, *value;
	
};

PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::map<std::string, std::string> props, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size)
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
	fil = new wxTextCtrl(this, FILTERID, "", wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxALL, 10);
	ct->Add(new wxButton(this, ADDID, "Add"), 0, wxALL, 10);
	ct->Add(new wxButton(this, DELETEID, "Delete"), 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	Bind(wxEVT_TEXT_ENTER, &PropertyDialog::FilterGrid, this);
	Bind(wxEVT_TEXT, &PropertyDialog::FilterGrid, this, FILTERID);
}


PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxFileConfig *config, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size)
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
	fil = new wxTextCtrl(this, FILTERID, "", wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxALL, 10);
	ct->Add(new wxButton(this, ADDID, "Add"), 0, wxALL, 10);
	ct->Add(new wxButton(this, DELETEID, "Delete"), 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	Bind(wxEVT_PG_CHANGED,&PropertyDialog::UpdateProperty,this);
	Bind(wxEVT_TEXT_ENTER, &PropertyDialog::FilterGrid, this);
	Bind(wxEVT_TEXT, &PropertyDialog::FilterGrid, this, FILTERID);
	Bind(wxEVT_BUTTON, &PropertyDialog::AddProp, this, ADDID);
	Bind(wxEVT_BUTTON, &PropertyDialog::DelProp, this, DELETEID);
	Bind(wxEVT_BUTTON, &PropertyDialog::HideDialog, this, HIDEID);
}


PropertyDialog::~PropertyDialog()
{
	if (pg) delete pg;
}

void PropertyDialog::HideDialog(wxCommandEvent& event)
{
	Hide();
}

void PropertyDialog::ClearModifiedStatus()
{
	if (pg) pg->ClearModifiedStatus();
}

void PropertyDialog::UpdateProperty(wxPropertyGridEvent& event)
{
	//Note: rawprocFrm::UpdateConfig() is changing the .conf file, so don't remove event.Skip()
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

std::map<std::string,std::string> PropertyDialog::FilterList(wxString filter)
{
	std::map<std::string,std::string> params;
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
  		wxPGProperty* p = *it;

		if (p->GetName().Find(filter) != wxNOT_FOUND) {
			params[std::string(p->GetName().c_str())] = std::string(p->GetValue().GetString().c_str());
		}

	}
	return params;
}

std::string PropertyDialog::FilterString(wxString filter)
{
	std::string params;
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
  		wxPGProperty* p = *it;

		if (p->GetName().Find(filter) != wxNOT_FOUND) {
			params.append(wxString::Format("%s=%s;",p->GetName(),p->GetValue().GetString()).c_str());
			//params[std::string(p->GetName().c_str())] = std::string(p->GetValue().GetString().c_str());
		}

	}
	return params;
}

bool PropertyDialog::PropExists(wxString name)
{
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
		wxPGProperty* p = *it;
		if (p->GetName() == name) return true;
	}
	return false;
}

void PropertyDialog::AddProp(wxCommandEvent& event)
{
	AddDialog *add = new AddDialog(this, wxID_ANY);
	if (add->ShowModal() == wxID_OK) {
		if (!PropExists(add->GetName())) {
			pg->Append(new wxStringProperty(add->GetName(), add->GetName(), add->GetValue()));
			pg->Sort();
			wxMessageBox(wxString::Format("Changed %s to %s.", add->GetName(), add->GetValue()));
			wxConfigBase::Get()->Write(add->GetName(), add->GetValue());
			wxConfigBase::Get()->Flush();
		}
		else
			wxMessageBox("Property already exists.");
	}
	add->~AddDialog();
	
}


void PropertyDialog::DelProp(wxCommandEvent& event)
{
	wxPGProperty* p = pg->GetSelectedProperty();
	wxString name = p->GetName();
	int answer = wxMessageBox(wxString::Format("Delete %s?",name), "Confirm",wxYES_NO | wxCANCEL, this);
	if (answer == wxYES) {
		pg->DeleteProperty(p);
		wxConfigBase::Get()->DeleteEntry(name);
		wxConfigBase::Get()->Flush();
	}
}
