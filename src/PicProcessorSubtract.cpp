#include "PicProcessorSubtract.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "util.h"
#include "gimage/strutil.h"
#include "gimage/curve.h"

#define SUBTRACTENABLE 8100
#define SUBTRACTVAL 8101
#define SUBTRACTFILE 8102
#define SUBTRACTCAMERA 8103

class SubtractPanel: public PicProcPanel
{

	public:
		SubtractPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, SUBTRACTENABLE, _("subtract:"));
			enablebox->SetValue(true);
			
			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);
			chan->SetStringSelection("rgb");

			subb = new wxRadioButton(this, SUBTRACTVAL, _("value:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP); 
			fileb = new wxRadioButton(this, SUBTRACTFILE, _("file:"));
			camb = new wxRadioButton(this, SUBTRACTCAMERA, _("camera:"));
			subb->SetValue(true);

			subtract = new myFloatCtrl(this, wxID_ANY, 0.0, 2);
			darkfile = new wxTextCtrl(this, wxID_ANY, "(none)", wxDefaultPosition, wxSize(150,TEXTCTRLHEIGHT));
			cam = new wxStaticText(this, wxID_ANY, "--");

			std::vector<std::string> p = split(params.ToStdString(),",");
			std::string param = p[0];

			if (isFloat(param)) {
				submode = SUBTRACTVAL;
				subb->SetValue(true);
				subtract->SetFloatValue(atof(param.c_str()));
			}
			else if (param == "rgb" | param == "red" | param == "green" | param == "blue") {
				submode = SUBTRACTVAL;
				subb->SetValue(true);
				if (p.size() >= 2) 
					if (isFloat(p[1])) subtract->SetFloatValue(atof(p[1].c_str()));
			}
			else if (param == "camera") {
				submode = SUBTRACTCAMERA;
				camb->SetValue(true);
				chan->Enable(false);
			}
			else {
				submode = SUBTRACTFILE;
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
			m->AddRowItem(subb, flags);
			m->AddRowItem(subtract, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(fileb, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(darkfile,  wxSizerFlags(1).Left().Border(wxLEFT|wxTOP).CenterVertical());
			m->AddRowItem(new wxButton(this, wxID_ANY, _("Select")), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(camb, flags.CenterVertical());
			m->AddRowItem(cam, flags);
			m->NextRow();

			m->End();
			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);

			Bind(wxEVT_TIMER, &SubtractPanel::OnTimer, this);
			Bind(wxEVT_TEXT_ENTER,&SubtractPanel::fileChanged, this);
			Bind(myFLOATCTRL_CHANGE, &SubtractPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &SubtractPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &SubtractPanel::onEnable, this, SUBTRACTENABLE);
			Bind(wxEVT_RADIOBUTTON, &SubtractPanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &SubtractPanel::selectDarkFile, this);
			Bind(wxEVT_CHAR_HOOK, &SubtractPanel::OnKey,  this);
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

		void setCameraVal(float val)
		{
			camval = val;
			cam->SetLabel(wxString::Format("%f",val));
			Refresh();
		}

		void selectDarkFile(wxCommandEvent& event)
		{
			wxFileName fname, pname;
			fname.Assign(wxFileSelector(_("Select dark file")));
			darkfile->SetValue(fname.GetFullName());
			if (submode == SUBTRACTFILE) processSUB();
		}

		void processSUB()
		{
			float evval;
			switch (submode) {
				case SUBTRACTVAL:
					q->setParams(wxString::Format("%s,%f",chan->GetString(chan->GetSelection()),subtract->GetFloatValue()));
					q->processPic();
					break;
				case SUBTRACTFILE:
					q->setParams(wxString::Format("file,%s",darkfile->GetValue()));
					q->processPic();
					break;
				case SUBTRACTCAMERA:
					q->setParams("camera");
					q->processPic();
					break;

			}
		}
		
		void OnRadioButton(wxCommandEvent& event)
		{
			submode = event.GetId();
			switch (submode) {
				case SUBTRACTVAL:
					chan->Enable(true);
					break;
				case SUBTRACTCAMERA:
				case SUBTRACTFILE:
					chan->Enable(false);
					break;
			}
			processSUB();
		}
		
		void fileChanged(wxCommandEvent& event)
		{
			if (submode == SUBTRACTFILE) processSUB();
		}

		void paramChanged(wxCommandEvent& event)
		{
			if (submode == SUBTRACTVAL) t.Start(500,wxTIMER_ONE_SHOT);
		}

		void paramUpdated(wxCommandEvent& event)
		{
			if (submode == SUBTRACTVAL) processSUB();
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			processSUB();
		}

	private:
		wxChoice *chan;
		wxCheckBox *enablebox;
		wxRadioButton *subb, *fileb, *camb;
		wxTextCtrl *darkfile;
		myFloatCtrl *subtract;
		wxStaticText *cam;
		wxTimer t;
		float camval;
		int submode;

};

PicProcessorSubtract::PicProcessorSubtract(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorSubtract::createPanel(wxSimplebook* parent)
{
	toolpanel = new SubtractPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	((SubtractPanel *) toolpanel)->setCameraVal(atof(getPreviousPicProcessor()->getProcessedPic().getInfoValue("LibrawBlack").c_str()) / 65536.0);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSubtract::processPicture(gImage *processdib) 
{
	double subtract;
	wxFileName fname;
	//gImage darkfile;

	((wxFrame*) m_display->GetParent())->SetStatusText(_("subtract..."));

	bool result = false;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;
	
	if (processingenabled) {
		
		mark();
		std::vector<std::string> p = split(c.ToStdString(),",");
		std::string param = p[0];
	
		if (param == "rgb" | param == "red" | param == "green" | param == "blue") {
			m_tree->SetItemText(id, _("subtract:val"));
			setChannel(wxString(param));
			if (p.size() >=2) subtract = atof(p[1].c_str());
			dib->ApplySubtract(subtract, channel, true, threadcount);
			m_display->SetModified(true);
			result = true;
		}
		else if (param == "camera") {
			m_tree->SetItemText(id, _("subtract:camera"));
			subtract = atof(getPreviousPicProcessor()->getProcessedPic().getInfoValue("LibrawBlack").c_str()) / 65536.0;
			setChannel("rgb");
			dib->ApplySubtract(subtract, channel, true, threadcount);
			m_display->SetModified(true);
			result = true;
		}
		else {
			m_tree->SetItemText(id, _("subtract:file"));
			if (wxFileName::FileExists(wxString(param))) {
				if (dib->ApplySubtract(param.c_str(), true, threadcount)) {
					m_display->SetModified(true);
					result = true;
				}
				else {
					wxMessageBox(_("dark image subtraction not successful."));
					result = false;
				}
			}
			else {
				wxMessageBox(_("dark image file not found."));
				result = false;
			}
		}
		wxString d = duration();

		if (result) 
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.subtract.log","0") == "1"))
				log(wxString::Format(_("tool=subtract,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));

	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}





