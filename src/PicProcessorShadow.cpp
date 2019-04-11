       
#include "PicProcessor.h"
#include "PicProcessorShadow.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "undo.xpm"

#include "util.h"

#define SHADOWENABLE 7700

class ShadowPanel: public PicProcPanel
{
	public:
		ShadowPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxGridBagSizer *g = new wxGridBagSizer();
			wxArrayString p = split(params,",");

			int shd = atoi(p[0]);
			int thr = atoi(p[1]);

			SetSize(parent->GetSize());

			enablebox = new wxCheckBox(this, SHADOWENABLE, "shadow:");
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			shadow = new wxSlider(this, wxID_ANY, shd, -50, +50, wxPoint(10, 30), wxSize(140, -1));
			g->Add(shadow , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, wxString::Format("%4d",shd), wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn1 = new wxBitmapButton(this, 1000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset shadow to default");
			g->Add(btn1, wxGBPosition(2,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			g->Add(new wxStaticText(this,wxID_ANY, "threshold: "), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			threshold = new wxSlider(this, wxID_ANY, thr, 0, 128, wxPoint(10, 30), wxSize(140, -1));
			g->Add(threshold , wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, wxString::Format("%4d",thr), wxDefaultPosition, wxSize(30, -1));
			g->Add(val2 , wxGBPosition(3,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn2 = new wxBitmapButton(this, 2000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip("Reset threshold to default");
			g->Add(btn2, wxGBPosition(3,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &ShadowPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &ShadowPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ShadowPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &ShadowPanel::onEnable, this, SHADOWENABLE);
			Bind(wxEVT_TIMER, &ShadowPanel::OnTimer,  this);
		}

		~ShadowPanel()
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

		void OnChanged(wxCommandEvent& event)
		{
			val1->SetLabel(wxString::Format("%4d", shadow->GetValue()));
			val2->SetLabel(wxString::Format("%4d", threshold->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val1->SetLabel(wxString::Format("%4d", shadow->GetValue()));
			val2->SetLabel(wxString::Format("%4d", threshold->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d,%d",shadow->GetValue(),threshold->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetshadowval, resetthresholdval;
			switch (event.GetId()) {
				case 1000:
					resetshadowval = atoi(myConfig::getConfig().getValueOrDefault("tool.shadow.level","0").c_str());
					shadow->SetValue(resetshadowval);
					val1->SetLabel(wxString::Format("%4d", resetshadowval));
					break;
				case 2000:
					resetthresholdval = atoi(myConfig::getConfig().getValueOrDefault("tool.shadow.threshold","64").c_str());
					threshold->SetValue(resetthresholdval);
					val2->SetLabel(wxString::Format("%4d", resetthresholdval));
					break;
			}
			q->setParams(wxString::Format("%d,%d", shadow->GetValue(),threshold->GetValue()));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *shadow, *threshold;
		wxStaticText *val1, *val2;
		wxBitmapButton *btn1, * btn2;
		wxCheckBox *enablebox;
		wxTimer *t;

};


PicProcessorShadow::PicProcessorShadow(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorShadow::createPanel(wxSimplebook* parent)
{
	toolpanel = new ShadowPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorShadow::processPic(bool processnext) 
{
	if (!global_processing_enabled) return true;
	bool result = true;
	((wxFrame*) m_display->GetParent())->SetStatusText("shadow...");

	wxArrayString cp = split(getParams(),",");
	double shd = atof(cp[0]);
	double thr = atof(cp[1]);

	Curve ctrlpts;
	ctrlpts.insertpoint(0,0);
	ctrlpts.insertpoint((thr/2)-shd,(thr/2)+shd);
	ctrlpts.insertpoint(thr,thr);
	ctrlpts.insertpoint(thr+20,thr+20);
	ctrlpts.insertpoint(255,255);

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.highlight.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) {
	mark();
		dib->ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.shadow.log","0") == "1"))
			log(wxString::Format("tool=shadow,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}



