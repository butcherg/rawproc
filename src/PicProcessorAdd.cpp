#include "PicProcessorAdd.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "util.h"
#include "gimage/strutil.h"
#include "gimage/curve.h"

#define ADDENABLE 8500
#define ADDVAL 8501
#define ADDFILE 8502
//#define ADDCAMERA 8503

class AddPanel: public PicProcPanel
{

	public:
		AddPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, ADDENABLE, _("add:"));
			enablebox->SetValue(true);
			
			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);
			chan->SetStringSelection("rgb");

			addb = new wxRadioButton(this, ADDVAL, _("value:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP); 
			fileb = new wxRadioButton(this, ADDFILE, _("file:"));
			//camb = new wxRadioButton(this, SUBTRACTCAMERA, _("camera:"));
			addb->SetValue(true);

			add = new myFloatCtrl(this, wxID_ANY, 0.0, 2);
			darkfile = new wxTextCtrl(this, wxID_ANY, "(none)", wxDefaultPosition, wxSize(150,TEXTCTRLHEIGHT));
			//cam = new wxStaticText(this, wxID_ANY, "--");

			std::vector<std::string> p = split(params.ToStdString(),",");
			std::string param = p[0];

			if (isFloat(param)) {
				addmode = ADDVAL;
				addb->SetValue(true);
				add->SetFloatValue(atof(param.c_str()));
			}
			else if (param == "rgb" | param == "red" | param == "green" | param == "blue") {
				addmode = ADDVAL;
				addb->SetValue(true);
				if (p.size() >= 2) 
					if (isFloat(p[1])) add->SetFloatValue(atof(p[1].c_str()));
			}
			//else if (param == "camera") {
			//	addmode = SUBTRACTCAMERA;
			//	camb->SetValue(true);
			//	chan->Enable(false);
			//}
			else {
				addmode = ADDFILE;
				fileb->SetValue(true);
				darkfile->SetValue(wxString(param));
				chan->Enable(false);
			}
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(chan, wxSizerFlags(0).Right().Border(wxRIGHT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(addb, flags);
			m->AddRowItem(add, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(fileb, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(darkfile,  wxSizerFlags(1).Left().Border(wxLEFT|wxTOP).CenterVertical());
			m->AddRowItem(new wxButton(this, wxID_ANY, _("Select")), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			//m->NextRow();
			//m->AddRowItem(camb, flags.CenterVertical());
			//m->AddRowItem(cam, flags);
			//m->NextRow();

			m->End();
			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);

			Bind(wxEVT_TIMER, &AddPanel::OnTimer, this);
			Bind(wxEVT_TEXT_ENTER,&AddPanel::fileChanged, this);
			Bind(myFLOATCTRL_CHANGE, &AddPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &AddPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &AddPanel::onEnable, this, ADDENABLE);
			Bind(wxEVT_RADIOBUTTON, &AddPanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &AddPanel::selectDarkFile, this);
			Bind(wxEVT_CHAR_HOOK, &AddPanel::OnKey,  this);
			Bind(wxEVT_CHOICE, &AddPanel::onChoice,  this);
			Thaw();
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				q->processPic();
			}
			else {
				q->enableProcessing(false);
				q->processPic();
			}
		}
		
		void onChoice(wxCommandEvent& event)
		{
			processSUB();
		}

/*
		void setCameraVal(float val)
		{
			camval = val;
			cam->SetLabel(wxString::Format("%f",val));
			Refresh();
		}

		void setCameraVal(wxString str)
		{
			cam->SetLabel(str);
			Refresh();
		}
*/

		void selectDarkFile(wxCommandEvent& event)
		{
			wxFileName fname, pname;
			fname.Assign(wxFileSelector(_("Select dark file")));
			darkfile->SetValue(fname.GetFullName());
			if (addmode == ADDFILE) processSUB();
		}

		void processSUB()
		{
			float evval;
			switch (addmode) {
				case ADDVAL:
					q->setParams(wxString::Format("%s,%f",chan->GetString(chan->GetSelection()),add->GetFloatValue()));
					q->processPic();
					break;
				case ADDFILE:
					q->setParams(wxString::Format("file,%s",darkfile->GetValue()));
					q->processPic();
					break;
			}
		}
		
		void OnRadioButton(wxCommandEvent& event)
		{
			addmode = event.GetId();
			switch (addmode) {
				case ADDVAL:
					chan->Enable(true);
					break;
				case ADDFILE:
					chan->Enable(false);
					break;
			}
			processSUB();
		}
		
		void fileChanged(wxCommandEvent& event)
		{
			if (addmode == ADDFILE) processSUB();
		}

		void paramChanged(wxCommandEvent& event)
		{
			if (addmode == ADDVAL) t.Start(500,wxTIMER_ONE_SHOT);
		}

		void paramUpdated(wxCommandEvent& event)
		{
			if (addmode == ADDVAL) processSUB();
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			processSUB();
		}

	private:
		wxChoice *chan;
		wxCheckBox *enablebox;
		wxRadioButton *addb, *fileb; //, *camb;
		wxTextCtrl *darkfile;
		myFloatCtrl *add;
		//wxStaticText *cam;
		wxTimer t;
		//float camval;
		int addmode;

};

PicProcessorAdd::PicProcessorAdd(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorAdd::createPanel(wxSimplebook* parent)
{
	toolpanel = new AddPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	gImage dib = getPreviousPicProcessor()->getProcessedPic();

	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorAdd::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("add..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_add(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_add(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.add.log","0") == "1"))
					log(wxString::Format(_("tool=add,%s,imagesize=%dx%d,threads=%s,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["threadcount"].c_str(),
						result["duration"].c_str())
					);
		}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}





