#include "PicProcessor.h"
#include "PicProcessorBright.h"
#include "PicProcPanel.h"
#include "undo.xpm"

#include "util.h"
#include "myConfig.h"

class BrightPanel: public PicProcPanel
{
	public:
		BrightPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			int initialvalue = atoi(params.c_str());

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "bright: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			bright = new wxSlider(this, wxID_ANY, initialvalue, -100, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(bright , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, params, wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			g->Add(btn, wxGBPosition(1,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &BrightPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &BrightPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &BrightPanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &BrightPanel::OnTimer,  this);
		}

		~BrightPanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", bright->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", bright->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",bright->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval =  atoi(myConfig::getConfig().getValueOrDefault("tool.bright.initialvalue","0").c_str());
			bright->SetValue(resetval);
			q->setParams(wxString::Format("%d",resetval));
			val->SetLabel(wxString::Format("%4d", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *bright;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxTimer *t;

};


PicProcessorBright::PicProcessorBright(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorBright::createPanel(wxSimplebook* parent)
{
	toolpanel = new BrightPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorBright::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("bright...");
	double bright = atof(c.c_str());
	bool result = true;

	Curve ctrlpts;
	ctrlpts.insertpoint(0,0);
	if (bright < 0)
		ctrlpts.insertpoint(255,255+bright);
	else
		ctrlpts.insertpoint(255-bright,255);

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.bright.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.bright.log","0") == "1"))
		log(wxString::Format("tool=bright,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty=false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



