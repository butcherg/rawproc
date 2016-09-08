#include "PicProcessor.h"
#include "PicProcessorDenoise.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "undo.xpm"
//#include <omp.h>

#include "util.h"
#include "FreeImage_Threaded.h"
#include <wx/fileconf.h>

#define SIGMASLIDER 8000
#define LOCALSLIDER 8001
#define PATCHSLIDER 8002

#define SIGMARESET 8010
#define LOCALRESET 8011
#define PATCHRESET 8012

class DenoisePanel: public PicProcPanel
{
	public:
		DenoisePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
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
				wxConfigBase::Get()->Read("tool.denoise.initialvalue",&sigmareset,0);
				sigma->SetValue(sigmareset);
				val->SetLabel(wxString::Format("%4d", sigmareset));
				q->setParams(wxString::Format("%d,%d,%d",sigma->GetValue(),local->GetValue(),patch->GetValue()));
				q->processPic();
			}
			if (event.GetId() == LOCALRESET) {
				wxConfigBase::Get()->Read("tool.denoise.local",&localreset,3);
				local->SetValue(localreset);
				val1->SetLabel(wxString::Format("%4d", localreset));
			}
			if (event.GetId() == PATCHRESET) {
				wxConfigBase::Get()->Read("tool.denoise.patch",&patchreset,1);
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


PicProcessorDenoise::PicProcessorDenoise(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorDenoise::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new DenoisePanel(m_parameters, this, c);
}


bool PicProcessorDenoise::processPic() {
	((wxFrame*) m_display->GetParent())->SetStatusText("denoise...");
	int threadcount;

	wxArrayString cp = split(getParams(),",");
	double sigma = atof(cp[0]);
	int local = atoi(cp[1]);
	int patch = atoi(cp[2]);

	bool result = true;



	wxConfigBase::Get()->Read("tool.denoise.cores",&threadcount,0);
	if (threadcount == 0) threadcount = ThreadCount();

	//wxConfigBase::Get()->Read("tool.denoise.local",&local,3);
	//wxConfigBase::Get()->Read("tool.denoise.patch",&patch,1);


	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	if (sigma > 0.0) {
		mark();
		ApplyNLMeans(getPreviousPicProcessor()->getProcessedPic(), dib, sigma, local, patch, threadcount);
		wxString d = duration();

		if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.denoise.log","0") == "1"))
			log(wxString::Format("tool=denoise,sigma=%2.2f,local=%d,patch=%d,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",sigma,local,patch,FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));
	}

	dirty=false;


	((wxFrame*) m_display->GetParent())->SetStatusText("");
	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

	return result;
}



