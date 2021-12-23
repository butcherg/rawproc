#include "PicProcessor.h"
#include "PicProcessorBanding.h"
#include "PicProcPanel.h"
#include "myFloatCtrl.h"
#include "myIntegerCtrl.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "util.h"
#include "undo.xpm"

#include <stdio.h>

#define BANDINGENABLE 8600

class BandingPanel: public PicProcPanel
{
	public:
		BandingPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());
			
			std::map<std::string,std::string> parm = parse_banding(params.ToStdString());

			enablebox = new wxCheckBox(this, BANDINGENABLE, _("banding:"));
			enablebox->SetValue(true);

			darkh = new myIntegerCtrl(this, wxID_ANY, "dark height:", 0, 0, 1000, wxDefaultPosition,wxDefaultSize);
			lighth = new myIntegerCtrl(this, wxID_ANY, "light height:", 0, 0, 1000, wxDefaultPosition,wxDefaultSize);
			evcomp = new myFloatCtrl(this, wxID_ANY, "ev comp:", 0.01, 2, wxDefaultPosition,wxDefaultSize);
			rolloff = new myIntegerCtrl(this, wxID_ANY, "rolloff:", 0, 0, 1000, wxDefaultPosition,wxDefaultSize);
			offset = new myIntegerCtrl(this, wxID_ANY, "offset height:", 0, 0, 1000, wxDefaultPosition,wxDefaultSize);			

			if (parm.find("darkheight") != parm.end()) darkh->SetIntegerValue(atoi(parm["darkheight"].c_str()));
			if (parm.find("lightheight") != parm.end()) lighth->SetIntegerValue(atoi(parm["lightheight"].c_str()));
			if (parm.find("ev") != parm.end()) evcomp->SetFloatValue(atof(parm["ev"].c_str()));
			
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxSizerFlags patchflags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT);
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(darkh,flags);
			m->NextRow();
			m->AddRowItem(lighth,flags);
			m->NextRow();
			m->AddRowItem(evcomp,flags);
			m->NextRow();
			m->AddRowItem(rolloff,flags);
			m->NextRow();
			m->AddRowItem(offset,flags);
			m->End();


			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);
			Bind(myFLOATCTRL_CHANGE,&BandingPanel::OnChanged, this);
			Bind(myFLOATCTRL_UPDATE,&BandingPanel::OnEnter, this);
			Bind(myINTEGERCTRL_CHANGE,&BandingPanel::OnChanged, this);
			Bind(myINTEGERCTRL_UPDATE,&BandingPanel::OnEnter, this);
			Bind(wxEVT_CHECKBOX, &BandingPanel::onEnable, this, BANDINGENABLE);
			Bind(wxEVT_TIMER, &BandingPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &BandingPanel::OnKey,  this);
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

		void OnEnter(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d,%d,%0.2f,%d,%d",
				darkh->GetIntegerValue(),
				lighth->GetIntegerValue(),
				evcomp->GetFloatValue(),
				rolloff->GetIntegerValue(),
				offset->GetIntegerValue()));
			q->processPic();
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d,%d,%0.2f,%d,%d",
				darkh->GetIntegerValue(),
				lighth->GetIntegerValue(),
				evcomp->GetFloatValue(),
				rolloff->GetIntegerValue(),
				offset->GetIntegerValue()));
			q->processPic();
			event.Skip();
		}

	private:
		wxChoice *chan;
		myIntegerCtrl *darkh, *lighth, *rolloff, *offset;
		myFloatCtrl *evcomp;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer t;

};


PicProcessorBanding::PicProcessorBanding(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorBanding::createPanel(wxSimplebook* parent)
{
	toolpanel = new BandingPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorBanding::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("banding..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_banding(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_banding(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.banding.log","0") == "1"))
					log(wxString::Format(_("tool=banding,%s,imagesize=%dx%d,threads=%s,time=%s"),
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




