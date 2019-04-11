#include "PicProcessorGamma.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "util.h"
#include "gimage/curve.h"

#define GAMMAENABLE 7100
#define GAMMAID 7101

class GammaPanel: public PicProcPanel
{

	public:
		GammaPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, GAMMAENABLE, "gamma:");
			enablebox->SetValue(true);
			b->Add(enablebox, flags);
			b->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			b->AddSpacer(10);

			gamma = new myFloatCtrl(this, wxID_ANY, atof(p.ToStdString().c_str()), 2);

			b->Add(gamma, flags);
			SetSizerAndFit(b);
			b->Layout();
			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &GammaPanel::OnTimer, this);
			Bind(myFLOATCTRL_CHANGE, &GammaPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &GammaPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &GammaPanel::onEnable, this, GAMMAENABLE);
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
	if (!global_processing_enabled) return true;
	Curve ctrlpts;
	((wxFrame*) m_display->GetParent())->SetStatusText("gamma...");
	double gamma = atof(c.c_str());
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.gamma.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	
	double exponent = 1 / gamma;
	double v = 255.0 * (double)pow((double)255, -exponent);
	for (int i = 0; i< 256; i+=1) {
		double color = (double)pow((double)i, exponent) * v;
		if (color > 255.0) color = 255.0;
		ctrlpts.insertpoint((double) i, color);
	}	

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());

	if (processingenabled) {
		mark();
		dib->ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.gamma.log","0") == "1"))
			log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





