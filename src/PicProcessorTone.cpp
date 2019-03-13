#include "PicProcessorTone.h"
#include "PicProcPanel.h"
#include "myRowColumnSizer.h"
#include "myFloatCtrl.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"

#define TONEENABLE 7900
#define TONEID 7901
#define TONEGAMMA 7902
#define TONEREINHARD 7903
#define TONELOG2 7904
#define TONELOGGAM 7905
#define TONEFILMIC 7906

class TonePanel: public PicProcPanel
{

	public:
		TonePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);


			enablebox = new wxCheckBox(this, TONEENABLE, "tone:");
			enablebox->SetValue(true);

			//All the radio buttons in the same group:
			gamb = new wxRadioButton(this, TONEGAMMA, "gamma", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			reinb = new wxRadioButton(this, TONEREINHARD, "reinhard");
			log2b = new wxRadioButton(this, TONELOG2, "log2");
			hybloggam = new wxRadioButton(this, TONELOGGAM, "loggamma");
			filmic = new wxRadioButton(this, TONEFILMIC, "filmic");

			gamma = new myFloatCtrl(this, wxID_ANY, atof(p.ToStdString().c_str()), 2);

			wxArrayString str;
			str.Add("channel");
			str.Add("luminance");
			reinop = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(80,TEXTCTRLHEIGHT), str);

			wxArrayString p = split(params,",");
			if (p[0] == "gamma") {
				gamb->SetValue(true);
				if (p.GetCount() >=2) 
					gamma->SetFloatValue(atof(p[1].c_str()));
				else
					gamma->SetFloatValue(1.0);
			}
			else {
				gamma->SetFloatValue(1.0);
			}
			if (p[0] == "reinhard") {
				reinb->SetValue(true);
				if (p.GetCount() >=2) 
					reinop->SetStringSelection(p[1]);
				else
					reinop->SetSelection(reinop->FindString("channel"));
			}
			else {
				reinop->SetSelection(reinop->FindString("channel"));
			}
			if (p[0] == "log2") {
				log2b->SetValue(true);
			}
			if (p[0] == "loggamma") {
				hybloggam->SetValue(true);
			}
			if (p[0] == "filmic") {
				filmic->SetValue(true);
			}

			//Lay out the controls in the panel:
			myRowColumnSizer *m = new myRowColumnSizer(10,3);
			m->AddItem(enablebox, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), wxALIGN_LEFT, 2);
			m->NextRow();
			m->AddItem(gamb, wxALIGN_LEFT);
			m->AddItem(gamma, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(reinb, wxALIGN_LEFT);
			m->AddItem(reinop, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(log2b, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(hybloggam, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(filmic, wxALIGN_LEFT);

			SetSizerAndFit(m);
			m->Layout();
			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &TonePanel::OnTimer, this);
			Bind(myFLOATCTRL_CHANGE, &TonePanel::gammaParamChanged, this);
			Bind(myFLOATCTRL_UPDATE, &TonePanel::gammaParamUpdated, this);
			Bind(wxEVT_CHECKBOX, &TonePanel::onEnable, this, TONEENABLE);
			Bind(wxEVT_RADIOBUTTON, &TonePanel::OnButton, this);
			Bind(wxEVT_CHOICE, &TonePanel::reinopChanged, this);
			Refresh();
			Update();
		}

		~TonePanel()
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

		void reinopChanged(wxCommandEvent& event)
		{
			if (reinb->GetValue()) {
				q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
				q->processPic();
			}
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			processTone(event.GetId());
			event.Skip();
		}

		void processTone(int src)
		{
			switch (src) {
				case TONEGAMMA:
					q->setParams(wxString::Format("gamma,%0.2f",gamma->GetFloatValue()));
					break;
				case TONEREINHARD:
					q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
					break;
				case TONELOG2:
					q->setParams(wxString::Format("log2"));
					break;
				case TONELOGGAM:
					q->setParams(wxString::Format("loggamma"));
					break;
				case TONEFILMIC:
					q->setParams(wxString::Format("filmic"));
					break;
			}
			q->processPic();
			Refresh();
		}


		void gammaParamChanged(wxCommandEvent& event)
		{
			if (gamb->GetValue()) t->Start(500,wxTIMER_ONE_SHOT);
		}
		
		void gammaParamUpdated(wxCommandEvent& event)
		{
			if (gamb->GetValue()) processTone(TONEGAMMA);
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			processTone(TONEGAMMA);
		}

	private:
		wxTimer *t;
		myFloatCtrl *gamma;
		wxCheckBox *enablebox;
		wxRadioButton *gamb, *reinb, *log2b, *hybloggam, *filmic;
		wxChoice *reinop;

};

PicProcessorTone::PicProcessorTone(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorTone::createPanel(wxSimplebook* parent)
{
	toolpanel = new TonePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorTone::processPic(bool processnext) 
{
	wxString d;
	wxArrayString p = split(c,",");
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tone.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());

	if (processingenabled) {
		if (p[0] == "gamma") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: gamma...");
			m_tree->SetItemText(id, "tone:gamma");
			double gamma = 1.0;
			if (p.size() >= 2) gamma = atof(p[1].c_str());
			mark();
			dib->ApplyToneMapGamma(gamma, threadcount);
			d = duration();

		}
		else if (p[0] == "loggamma") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: hybrid log gamma...");
			m_tree->SetItemText(id, "tone:loggamma");
			mark();
			dib->ApplyToneMapLogGamma(threadcount);
			d = duration();

		}
		else if (p[0] == "log2") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: log2...");
			m_tree->SetItemText(id, "tone:log2");
			mark();
			dib->ApplyToneMapLog2(threadcount);
			d = duration();
		}
		else if (p[0] == "reinhard") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: reinhard...");
			m_tree->SetItemText(id, "tone:reinhard");
			bool channel = true;
			if (p.size() >= 2) if (p[1] == "luminance") channel = false;
			mark();
			dib->ApplyToneMapReinhard(channel, threadcount);
			d = duration();
		}
		else if (p[0] == "filmic") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: filmic...");
			m_tree->SetItemText(id, "tone:filmic");
			mark();
			dib->ApplyToneMapFilmic(false, threadcount);
			d = duration();
		}

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.tone.log","0") == "1"))
			log(wxString::Format("tool=tone:%s,imagesize=%dx%d,threads=%d,time=%s",p[0],dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





