#include "myMetadataDialog.h"
//#include "myConfig.h"
#include "util.h"
#include <wx/sizer.h>
#include <wx/propgrid/advprops.h>

#include <map>
#include <string>

#include <wx/wx.h>

#define TAGFILTERID 3110
#define TAGADDID 3111
#define TAGDELETEID 3112
#define TAGHIDEID 3113
#define TAGRESETFILTERID 3114

class AddTagDialog: public wxDialog
{
	public:
		AddTagDialog(wxWindow *parent, wxWindowID id): 
		wxDialog(parent, id, "Add Property", wxDefaultPosition, wxDefaultSize) //wxSize(220,300))
		{
			wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
			wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);
			
			sz->Add(new wxStaticText(this, wxID_ANY, "Name: "), 0, wxLEFT, 5);
			name = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			sz->Add(name, 0, wxLEFT | wxRIGHT, 5);
			sz->Add(new wxStaticText(this, wxID_ANY, "Value: "), 0, wxLEFT, 5);
			value = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			sz->Add(value, 0, wxLEFT | wxRIGHT, 5);
			
			ct->Add(new wxButton(this, wxID_OK, "Ok"), 0, wxALL, 10);
			ct->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
			sz->Add(ct, 0, wxALL, 10);
			SetSizerAndFit(sz);

			Bind(wxEVT_TEXT_ENTER, &AddTagDialog::OnTextEnter, this);
		}
		
		~AddTagDialog()
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
		
		void OnTextEnter(wxCommandEvent& event)
		{
			EndModal(wxID_OK);
		}
		
		
	private:
		wxTextCtrl *name, *value;
	
};

MetadataDialog::MetadataDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size, wxCAPTION|wxRESIZE_BORDER)
{
	dib = NULL;
	sz = new wxBoxSizer(wxVERTICAL);
	ct = new wxBoxSizer(wxHORIZONTAL);

	pg = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, size, wxPG_BOLD_MODIFIED | wxPG_HIDE_MARGIN);
	SetExtraStyle(GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS);
	
	sz->Add(pg, 1, wxEXPAND | wxALL, 3);
	
	ct->Add(new wxButton(this, wxID_OK, "Dismiss"), 0, wxALL, 10);
	ct->Add(new wxStaticText(this, wxID_ANY, "Filter: "), 0, wxALL, 10);
	fil = new wxTextCtrl(this, TAGFILTERID, "", wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxALL, 10);
	ct->Add(new wxButton(this, TAGRESETFILTERID, "X", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), 0, wxTOP|wxBOTTOM|wxRIGHT, 10);
	ct->Add(new wxButton(this, TAGADDID, "Add"), 0, wxALL, 10);
	ct->Add(new wxButton(this, TAGDELETEID, "Delete"), 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	fil->SetFocus();
	Bind(wxEVT_PG_CHANGED,&MetadataDialog::UpdateTag,this);
	Bind(wxEVT_TEXT_ENTER, &MetadataDialog::FilterGrid, this);
	Bind(wxEVT_TEXT, &MetadataDialog::FilterGrid, this, TAGFILTERID);
	Bind(wxEVT_BUTTON, &MetadataDialog::AddTag, this, TAGADDID);
	Bind(wxEVT_BUTTON, &MetadataDialog::DelTag, this, TAGDELETEID);
	Bind(wxEVT_BUTTON, &MetadataDialog::HideDialog, this, TAGHIDEID);
	Bind(wxEVT_BUTTON, &MetadataDialog::resetFilter, this, TAGRESETFILTERID);
}


void MetadataDialog::LoadMetadata(gImage *infodib)
{
	dib = infodib;
	std::map<std::string,std::string> imginfo =  dib->getInfo();
	for (std::map<std::string, std::string>::iterator it=imginfo.begin(); it!=imginfo.end(); ++it) {
		pg->Append(new wxStringProperty(it->first, it->first, it->second));
	}
	/*
	std::map<std::string, std::string> props = myConfig::getConfig().getDefault();
	std::map<std::string, std::string> temps = myConfig::getConfig().getSection("Templates");
	
	myConfig& config = myConfig::getConfig();
	
	for (std::map<std::string, std::string>::iterator it=props.begin(); it!=props.end(); ++it) {
		wxString name = it->first.c_str();
		wxString value = it->second.c_str();

		//find the applicable template, if it exists:
		std::string tplate = config.match_name("Templates", it->first.c_str());
		
		if (tplate != std::string()) {
			//std::string tplate = config.getValue("Templates", it->first);
			if (tplate.find_first_of("|") != std::string::npos) {
				wxArrayString choices = split(wxString(tplate.c_str()), "|");
				wxPGChoices ch(choices);
				pg->Append(new wxEnumProperty(name, wxPG_LABEL, ch, ch.Index(value)));
			}
			else if (tplate.find("iccfile") != std::string::npos) {
				//wxArrayString parms = split(wxString(template.c_str()), ":");
				wxString iccdirectory = config.getValue("cms.profilepath").c_str();
				wxPGProperty* prop = pg->Append(new wxFileProperty(name, wxPG_LABEL, value));
				pg->SetPropertyAttribute(prop,"InitialPath",iccdirectory );
				pg->SetPropertyAttribute(prop,"ShowFullPath",0);
			}
			else if (tplate.find("longstring") != std::string::npos) {
				wxPGProperty* prop = pg->Append(new wxLongStringProperty(name, wxPG_LABEL, value));
			}
		}
		//if no template:
		else pg->Append(new wxStringProperty(name, name, value));
	}
	*/
	
	pg->Sort();
}



MetadataDialog::~MetadataDialog()
{
	//if (pg) delete pg;
}

void MetadataDialog::HideDialog(wxCommandEvent& event)
{
	Hide();
}

void MetadataDialog::ClearModifiedStatus()
{
	if (pg) pg->ClearModifiedStatus();
}

void MetadataDialog::UpdateTag(wxPropertyGridEvent& event)
{
	wxPGProperty * prop = event.GetProperty();
	wxString propname = event.GetPropertyName();
	wxString propval = event.GetPropertyValue().GetString();
	dib->setInfoValue(propname.ToStdString(), propval.ToStdString());
	Refresh();
	event.Skip();
}

void MetadataDialog::resetFilter(wxCommandEvent& event)
{
	fil->SetValue("");
	int i;
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
    		wxPGProperty* p = *it;
		p->Hide(false);
	}
	fil->SetFocus();
}

void MetadataDialog::FilterGrid(wxCommandEvent& event)
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

std::map<std::string,std::string> MetadataDialog::FilterList(wxString filter)
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

std::string MetadataDialog::FilterString(wxString filter)
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

bool MetadataDialog::TagExists(wxString name)
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

void MetadataDialog::AddTag(wxCommandEvent& event)
{
	AddTagDialog *add = new AddTagDialog(this, wxID_ANY);
	if (add->ShowModal() == wxID_OK) {
		if (add->GetName() != "") {
			if (!TagExists(add->GetName())) {
				pg->Append(new wxStringProperty(add->GetName(), add->GetName(), add->GetValue()));
				pg->Sort();
				dib->setInfoValue(add->GetName().ToStdString(), add->GetValue().ToStdString());
				wxMessageBox(wxString::Format(_("Changed %s to %s."), add->GetName(), add->GetValue()));
				//myConfig::getConfig().setValue((const char  *) add->GetName().mb_str(),  (const char  *) add->GetValue().mb_str());
				//if (!myConfig::getConfig().flush()) wxMessageBox(_("Write to configuration file failed."));
			}
			else
				wxMessageBox(_("Tag already exists."));
		}
		else
			wxMessageBox(_("No name specified."));
	}
	add->~AddTagDialog();
	
}


void MetadataDialog::DelTag(wxCommandEvent& event)
{
	wxPGProperty* p = pg->GetSelectedProperty();
	if (p) {
		wxString name = p->GetName();
		int answer = wxMessageBox(wxString::Format(_("Delete %s?"),name), _("Confirm"),wxYES_NO | wxCANCEL, this);
		if (answer == wxYES) {
			pg->DeleteProperty(p);
			dib->deleteInfoValue(name.ToStdString());
			//myConfig::getConfig().deleteValue((const char  *) name.mb_str());
			//if (!myConfig::getConfig().flush()) wxMessageBox(_("Write to configuration file failed."));
		}
	}
}
