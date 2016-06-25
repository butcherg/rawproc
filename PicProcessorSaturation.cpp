#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "FreeImage_Threaded.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "util.h"
#include "undo.xpm"
#include <omp.h>

#include <wx/fileconf.h>

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			double initialvalue = atof(params.c_str());

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "saturation:"), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			saturate = new wxSlider(this, wxID_ANY, initialvalue*10.0, 0, 30, wxPoint(10, 30), wxSize(140, -1));
			g->Add(saturate, wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f", initialvalue), wxDefaultPosition, wxSize(30, -1));
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
			Bind(wxEVT_BUTTON, &SaturationPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &SaturationPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &SaturationPanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &SaturationPanel::OnTimer,  this);		}

		~SaturationPanel()
		{
			t->~wxTimer();
		}
/*
		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",saturate->GetValue()/10.0));
			q->processPic();
			event.Skip();
		}
*/
		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", saturate->GetValue()/10.0));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", saturate->GetValue()/10.0));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",saturate->GetValue()/10.0));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval;
			wxConfigBase::Get()->Read("tool.saturate.initialvalue",&resetval,1.0);
			saturate->SetValue(resetval*10.0);
			q->setParams(wxString::Format("%2.2f",resetval));
			val->SetLabel(wxString::Format("%2.2f", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *saturate;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxTimer *t;

};


PicProcessorSaturation::PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new SaturationPanel(p,this,c);
	showParams();
}

void PicProcessorSaturation::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new SaturationPanel(m_parameters, this, c);
}


bool PicProcessorSaturation::processPic() {
	((wxFrame*) m_display->GetParent())->SetStatusText("saturation...");
	double saturation = atof(c.c_str());
	bool result = true;
	int threadcount;
	wxConfigBase::Get()->Read("tool.saturate.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) omp_get_max_threads();
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	if (saturation != 1.0) {
		mark();
		ApplySaturation(getPreviousPicProcessor()->getProcessedPic(), dib, saturation, threadcount);
		wxString d = duration();

		if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.saturate.log","0") == "1"))
			log(wxString::Format("tool=saturate,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	}
	dirty = false;

		//put in every processPic()...
		if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
		wxTreeItemId next = m_tree->GetNextSibling(GetId());
		if (next.IsOk()) {
			PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
			nextitem->processPic();
		}


	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return result;
}



