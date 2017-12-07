#include "PicProcessor.h"
#include "PicProcessorDenoise.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "undo.xpm"

#include "util.h"

#define SIGMASLIDER 8000
#define LOCALSLIDER 8001
#define PATCHSLIDER 8002

#define SIGMARESET 8010
#define LOCALRESET 8011
#define PATCHRESET 8012

class DenoisePanel: public PicProcPanel
{
	public:
		DenoisePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxArrayString p = split(params,",");
			int initialvalue = atoi(p[0]);
			int localval = atoi(p[1]);
			int patchval = atoi(p[2]);

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "sigma: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			sigma = new wxSlider(this, SIGMASLIDER, initialvalue, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(sigma , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, p[0], wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, SIGMARESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			g->Add(btn, wxGBPosition(1,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(0,20, wxGBPosition(2,0));

			g->Add(new wxStaticText(this,wxID_ANY, "local: "), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			local = new wxSlider(this, LOCALSLIDER, localval, 0, 15, wxPoint(10, 30), wxSize(140, -1));
			g->Add(local , wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, p[1], wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(3,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn1 = new wxBitmapButton(this, LOCALRESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset to default");
			g->Add(btn1, wxGBPosition(3,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "patch: "), wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			patch = new wxSlider(this, PATCHSLIDER, patchval, 0, 15, wxPoint(10, 30), wxSize(140, -1));
			g->Add(patch , wxGBPosition(4,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, p[2], wxDefaultPosition, wxSize(30, -1));
			g->Add(val2 , wxGBPosition(4,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn2 = new wxBitmapButton(this, PATCHRESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip("Reset to default");
			g->Add(btn2, wxGBPosition(4,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &DenoisePanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &DenoisePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &DenoisePanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &DenoisePanel::OnTimer,  this);
		}

		~DenoisePanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			if (event.GetId() == SIGMASLIDER) val->SetLabel(wxString::Format("%4d", sigma->GetValue()));
			if (event.GetId() == LOCALSLIDER) val1->SetLabel(wxString::Format("%4d", local->GetValue()));
			if (event.GetId() == PATCHSLIDER) val2->SetLabel(wxString::Format("%4d", patch->GetValue()));
			if (event.GetId() == SIGMASLIDER) t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			if (event.GetId() == SIGMASLIDER) val->SetLabel(wxString::Format("%4d", sigma->GetValue()));
			if (event.GetId() == LOCALSLIDER) val1->SetLabel(wxString::Format("%4d", local->GetValue()));
			if (event.GetId() == PATCHSLIDER) val2->SetLabel(wxString::Format("%4d", patch->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d,%d,%d",sigma->GetValue(),local->GetValue(),patch->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int sigmareset, localreset, patchreset;
			if (event.GetId() == SIGMARESET) {
				sigmareset = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue","0").c_str());
				sigma->SetValue(sigmareset);
				val->SetLabel(wxString::Format("%4d", sigmareset));
				q->setParams(wxString::Format("%d,%d,%d",sigma->GetValue(),local->GetValue(),patch->GetValue()));
				q->processPic();
			}
			if (event.GetId() == LOCALRESET) {
				localreset = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.local","3").c_str());
				local->SetValue(localreset);
				val1->SetLabel(wxString::Format("%4d", localreset));
			}
			if (event.GetId() == PATCHRESET) {
				patchreset = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.patch","1").c_str());
				patch->SetValue(patchreset);
				val2->SetLabel(wxString::Format("%4d", patchreset));
			}

			event.Skip();
		}


	private:
		wxSlider *sigma, *local, *patch;
		wxStaticText *val, *val1, *val2;
		wxBitmapButton *btn, *btn1, *btn2;
		wxTimer *t;

};


PicProcessorDenoise::PicProcessorDenoise(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorDenoise::createPanel(wxSimplebook* parent)
{
	toolpanel = new DenoisePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorDenoise::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("denoise...");

	wxArrayString cp = split(getParams(),",");
	double sigma = atof(cp[0]);
	int local = atoi(cp[1]);
	int patch = atoi(cp[2]);

	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValue("tool.denoise.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (sigma > 0.0) {
		mark();
		dib->ApplyNLMeans(sigma,local, patch, threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValue("tool.all.log","0") == "1") || (myConfig::getConfig().getValue("tool.denoise.log","0") == "1"))
			log(wxString::Format("tool=denoise,sigma=%2.2f,local=%d,patch=%d,imagesize=%dx%d,threads=%d,time=%s",sigma,local,patch,dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty=false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}



