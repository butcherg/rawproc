#include "PicProcessor.h"
#include "PicProcessorDenoise.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "undo.xpm"
#include <omp.h>

#include "util.h"
#include "FreeImage_Threaded.h"
#include <wx/fileconf.h>

class DenoisePanel: public PicProcPanel
{
	public:
		DenoisePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			int initialvalue = atoi(params.c_str());

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "sigma: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			sigma = new wxSlider(this, wxID_ANY, initialvalue, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(sigma , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
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
			val->SetLabel(wxString::Format("%4d", sigma->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", sigma->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",sigma->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval;
			wxConfigBase::Get()->Read("tool.denoise.initialvalue",&resetval,35);
			sigma->SetValue(resetval);
			q->setParams(wxString::Format("%d",resetval));
			val->SetLabel(wxString::Format("%4d", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *sigma;
		wxStaticText *val;
		wxBitmapButton *btn;
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
	double sigma = atof(c.c_str());
	bool result = true;


	int threadcount, local, patch;
	wxConfigBase::Get()->Read("tool.denoise.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) omp_get_max_threads();

	wxConfigBase::Get()->Read("tool.denoise.local",&local,1);
	wxConfigBase::Get()->Read("tool.denoise.patch",&patch,3);

	mark();
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	ApplyNLMeans(getPreviousPicProcessor()->getProcessedPic(), dib, sigma, local, patch, threadcount);
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.denoise.log","0") == "1"))
		log(wxString::Format("tool=denoise,sigma=%d,local=%d,patch=%d,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",sigma,local,patch,FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

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



