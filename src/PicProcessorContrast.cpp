#include "PicProcessor.h"
#include "PicProcessorContrast.h"
#include "PicProcPanel.h"
#include "undo.xpm"

#include "util.h"
#include "myConfig.h"

#define CONTRASTENABLE 6700

class ContrastPanel: public PicProcPanel
{
	public:
		ContrastPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);


			int initialvalue = atoi(params.c_str());

			enablebox = new wxCheckBox(this, CONTRASTENABLE, "contrast:");
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxGBSpan(1,3), wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(200,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);


			contrast = new wxSlider(this, wxID_ANY, initialvalue, -100, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(contrast , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, params, wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			g->Add(btn, wxGBPosition(2,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &ContrastPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &ContrastPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ContrastPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &ContrastPanel::onEnable, this, CONTRASTENABLE);
			Bind(wxEVT_TIMER, &ContrastPanel::OnTimer,  this);
		}

		~ContrastPanel()
		{
			t->~wxTimer();
		}
/*
		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",contrast->GetValue()));
			q->processPic();
			event.Skip();
		}
*/

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
			val->SetLabel(wxString::Format("%4d", contrast->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", contrast->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",contrast->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval =  atoi(myConfig::getConfig().getValueOrDefault("tool.contrast.initialvalue","0").c_str());
			contrast->SetValue(resetval);
			q->setParams(wxString::Format("%d",resetval));
			val->SetLabel(wxString::Format("%4d", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *contrast;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer *t;

};


PicProcessorContrast::PicProcessorContrast(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorContrast::createPanel(wxSimplebook* parent)
{
	toolpanel = new ContrastPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorContrast::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("contrast...");
	double contrast = atof(c.c_str());
	bool result = true;

	Curve ctrlpts;
	if (contrast < 0) {
		ctrlpts.insertpoint(0,-contrast);
		ctrlpts.insertpoint(255,255+contrast);
	}
	else {
		ctrlpts.insertpoint(contrast,0);
		ctrlpts.insertpoint(255-contrast,255);
	}


	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.contrast.cores","0").c_str());
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

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.contrast.log","0") == "1"))
			log(wxString::Format("tool=contrast,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}



