#include "PicProcessorGamma.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage/curve.h"
#include "copy.xpm"
#include "paste.xpm"

#define GAMMAENABLE 7100
#define GAMMAID 7101
#define GAMMACOPY 7102
#define GAMMAPASTE 7103

class GammaPanel: public PicProcPanel
{

	public:
		GammaPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, GAMMAENABLE, "gamma:");
			enablebox->SetValue(true);
			gamma = new myFloatCtrl(this, wxID_ANY, atof(p.ToStdString().c_str()), 2);

			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, GAMMACOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, GAMMAPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(gamma, flags);
			m->End();
			SetSizerAndFit(m);
			m->Layout();

			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &GammaPanel::OnTimer, this);
			Bind(myFLOATCTRL_CHANGE, &GammaPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &GammaPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &GammaPanel::onEnable, this, GAMMAENABLE);
			Bind(wxEVT_BUTTON, &GammaPanel::OnCopy, this, GAMMACOPY);
			Bind(wxEVT_BUTTON, &GammaPanel::OnPaste, this, GAMMAPASTE);
			Refresh();
			Update();
		}

		~GammaPanel()
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

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Copied command to clipboard: %s",q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				gamma->SetFloatValue(atof(q->getParams().ToStdString().c_str()));
				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Pasted command from clipboard: %s",q->getCommand()));
			}
			else wxMessageBox(wxString::Format("Invalid Paste"));
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%0.2f", gamma->GetFloatValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void paramUpdated(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%0.2f", gamma->GetFloatValue()));
			q->processPic();
			Refresh();
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			q->processPic();
			Refresh();
		}

	private:
		wxCheckBox *enablebox;
		myFloatCtrl *gamma;
		wxTimer *t;

};

PicProcessorGamma::PicProcessorGamma(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorGamma::createPanel(wxSimplebook* parent)
{
	toolpanel = new GammaPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorGamma::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("gamma...");
	double gamma = atof(c.c_str());
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.gamma.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (!global_processing_enabled) return true;

	if (processingenabled) {
		mark();
		dib->ApplyToneMapGamma(gamma, threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.gamma.log","0") == "1"))
			log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





