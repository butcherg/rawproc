#include "PicProcessorSubtract.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "util.h"
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
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, SUBTRACTENABLE, "subtract:");
			enablebox->SetValue(true);

			subb = new wxRadioButton(this, SUBTRACTVAL, "value:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP); 
			fileb = new wxRadioButton(this, SUBTRACTFILE, "file:", wxDefaultPosition, wxDefaultSize);
			camb = new wxRadioButton(this, SUBTRACTCAMERA, "camera:", wxDefaultPosition, wxDefaultSize);

			subtract = new myFloatCtrl(this, wxID_ANY, atof(p.ToStdString().c_str()), 2);
			darkfile = new wxTextCtrl(this, wxID_ANY, "(none)", wxDefaultPosition, wxSize(150,TEXTCTRLHEIGHT));
			cam = new wxStaticText(this, wxID_ANY, "--");

			std::map<std::string,std::string> p = proc->paramMap(params.ToStdString(), "value,filename");

			if (p.find("value") != p.end())
				if (p["value"] == "camera") {
					submode = SUBTRACTCAMERA;
					camb->SetValue(true);
				}
				else if (p["value"] == "file" & p.find("filename") != p.end()) {
					submode = SUBTRACTFILE;
					fileb->SetValue(true);
					darkfile->SetValue(wxString(p["filename"]));
				}
				else {
					submode = SUBTRACTVAL;
					subb->SetValue(true);
					subtract->SetFloatValue(atof(p["subtract"].c_str()));
				}
			else {
				submode = SUBTRACTVAL;
				subb->SetValue(true);
				subtract->SetFloatValue(0.0);
			}

			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(subb, flags);
			m->AddRowItem(subtract, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(fileb, flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, wxID_ANY, "Select"), flags);
			m->AddRowItem(darkfile, flags.CenterVertical());

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(camb, flags.CenterVertical());
			m->AddRowItem(cam, flags);
			m->NextRow();

			m->End();
			SetSizerAndFit(m);
			m->Layout();


			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &SubtractPanel::OnTimer, this);
			Bind(wxEVT_TEXT_ENTER,&SubtractPanel::fileChanged, this);
			Bind(myFLOATCTRL_CHANGE, &SubtractPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &SubtractPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &SubtractPanel::onEnable, this, SUBTRACTENABLE);
			Bind(wxEVT_RADIOBUTTON, &SubtractPanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &SubtractPanel::selectDarkFile, this);
			Refresh();
			Update();
		}

		~SubtractPanel()
		{
			t->~wxTimer();
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
			fname.Assign(wxFileSelector("Select dark file"));
			darkfile->SetValue(fname.GetFullName());
			if (submode == SUBTRACTFILE) processSUB();
		}

		void processSUB()
		{
			float evval;
			switch (submode) {
				case SUBTRACTVAL:
					q->setParams(wxString::Format("%f",subtract->GetFloatValue()));
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
			processSUB();
		}
		
		void fileChanged(wxCommandEvent& event)
		{
			if (submode == SUBTRACTFILE) processSUB();
		}

		void paramChanged(wxCommandEvent& event)
		{
			if (submode == SUBTRACTVAL) t->Start(500,wxTIMER_ONE_SHOT);
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
		wxCheckBox *enablebox;
		wxRadioButton *subb, *fileb, *camb;
		wxTextCtrl *darkfile;
		myFloatCtrl *subtract;
		wxStaticText *cam;
		wxTimer *t;
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

bool PicProcessorSubtract::processPic(bool processnext) 
{
	double subtract;
	wxFileName fname;
	//gImage darkfile;

	((wxFrame*) m_display->GetParent())->SetStatusText("subtract...");

	std::map<std::string,std::string> p = paramMap(c.ToStdString(), "value,filename");

	if (p["value"] == "camera") {
		m_tree->SetItemText(id, "subtract:camera");
		subtract = atof(getPreviousPicProcessor()->getProcessedPic().getInfoValue("LibrawBlack").c_str()) / 65536.0;
	}
	else if (p["value"] == "file") {
		m_tree->SetItemText(id, "subtract:file");
	}
	else {
		m_tree->SetItemText(id, "subtract:val");
		subtract = atof(p["value"].c_str());
	}

	bool result = false;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (!global_processing_enabled) return true;

	if (processingenabled) {
		mark();

		std::vector<pix> &img = dib->getImageData();
		unsigned w = dib->getWidth(); unsigned h = dib->getHeight();

		if (p["value"] == "file") {
			if (wxFileName::FileExists(wxString(p["filename"]))) {
				if (dib->ApplySubtract(p["filename"].c_str(), threadcount)) {
					result = true;
				}
				else {
					wxMessageBox("dark image subtraction not successful.");
					result = false;
				}
			}
			else {
				wxMessageBox("dark image file not found.");
				result = false;
			}
		}
		else {
			dib->ApplySubtract(subtract, threadcount);
			result = true;
		}

		wxString d = duration();

		if (result) 
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.subtract.log","0") == "1"))
				log(wxString::Format("tool=subtract,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





