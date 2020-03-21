#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "PicProcPanel.h"
#include "myFloatCtrl.h"
#include "myConfig.h"
#include "util.h"
#include "undo.xpm"

#define SATURATIONENABLE 7600

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());

			rmin=0.0; rmax=FLT_MAX; gmin=0.0; gmax=FLT_MAX; bmin=0.0; bmax=FLT_MAX;
			ispatch = false;

			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxGridBagSizer *g = new wxGridBagSizer();

			double initialvalue = atof(params.c_str());

			enablebox = new wxCheckBox(this, SATURATIONENABLE, _("saturation:"));
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxGBSpan(1,3), wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			saturate = new myFloatCtrl(this, wxID_ANY, 1.0, 2, wxDefaultPosition,wxDefaultSize);
			g->Add(saturate, wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset to default"));
			g->Add(btn, wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &SaturationPanel::OnButton, this);
			Bind(myFLOATCTRL_CHANGE,&SaturationPanel::OnChanged, this);
			Bind(myFLOATCTRL_UPDATE,&SaturationPanel::OnEnter, this);
			Bind(wxEVT_CHECKBOX, &SaturationPanel::onEnable, this, SATURATIONENABLE);
			Bind(wxEVT_TIMER, &SaturationPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &SaturationPanel::OnKey,  this);
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
			q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			q->processPic();
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval = atof(myConfig::getConfig().getValueOrDefault("tool.saturate.initialvalue","1.0").c_str());
			saturate->SetFloatValue(resetval);
			q->setParams(wxString::Format("%2.2f",resetval));
			q->processPic();
			event.Skip();
		}

		void processSA()
		{
			if (ispatch) 
				q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f",rmin, rmax, gmin, gmax, bmin, bmax, saturate->GetFloatValue()));
			else
				q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			q->processPic();
		}

		void addToPatch(pix p)
		{
			ispatch = true;
			if (p.r < rmin) rmin = p.r;
			if (p.r > rmax) rmax = p.r;
			if (p.g < rmin) gmin = p.g;
			if (p.g > rmax) gmax = p.g;
			if (p.b < rmin) bmin = p.b;
			if (p.b > rmax) bmax = p.b;
		}

		void clearPatch()
		{
			rmin=0.0; rmax=FLT_MAX; gmin=0.0; gmax=FLT_MAX; bmin=0.0; bmax=FLT_MAX;
			ispatch = false;
		}


	private:
		myFloatCtrl *saturate;
		float rmin, rmax, gmin, gmax, bmin, bmax;
		bool ispatch;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer t;

};


PicProcessorSaturation::PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorSaturation::createPanel(wxSimplebook* parent)
{
	toolpanel = new SaturationPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSaturation::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText(_("saturation..."));

	float rmin=0.0, rmax=FLT_MAX, gmin=0.0, gmax=FLT_MAX, bmin=0.0, bmax=FLT_MAX;

	wxArrayString p = split(wxString(c),",");
	double saturation = atof(p[0].c_str());
	if (p.size() >= 2) rmin = atof(p[1].c_str());
	if (p.size() >= 3) rmin = atof(p[2].c_str());
	if (p.size() >= 4) rmin = atof(p[3].c_str());
	if (p.size() >= 5) rmin = atof(p[4].c_str());
	if (p.size() >= 6) rmin = atof(p[5].c_str());
	if (p.size() >= 7) rmin = atof(p[6].c_str());
	bool result = true;
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.saturate.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;

	if (processingenabled & saturation != 1.0) {
		mark();
		dib->ApplySaturate(saturation, rmin, rmax, gmin, gmax, bmin, bmax, 0.0, 0.0, 1.0, 1.0, threadcount);
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.saturate.log","0") == "1"))
			log(wxString::Format(_("tool=saturate,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));

	}
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}



