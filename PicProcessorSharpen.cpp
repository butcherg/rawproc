#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "ThreadedConvolve.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "undo.xpm"
#include "util.h"

#include <vector>
#include <wx/fileconf.h>

class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			int initialvalue = atoi(params.c_str());

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "sharpen: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			sharp = new wxSlider(this, wxID_ANY, initialvalue, 0, 10, wxPoint(10, 30), wxSize(140, -1));
			g->Add(sharp , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
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
			Bind(wxEVT_BUTTON, &SharpenPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &SharpenPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &SharpenPanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &SharpenPanel::OnTimer,  this);		}

		~SharpenPanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", sharp->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", sharp->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",sharp->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval;
			wxConfigBase::Get()->Read("tool.sharpen.initialvalue",&resetval,0);
			sharp->SetValue(resetval);
			q->setParams(wxString::Format("%d",resetval));
			val->SetLabel(wxString::Format("%4d", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *sharp;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxTimer *t;


};


PicProcessorSharpen::PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorSharpen::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new SharpenPanel(m_parameters, this, c);
}


bool PicProcessorSharpen::processPic() {
	double kernel[3][3] =
	{
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	std::vector<ThreadedConvolve *> t;
	int threadcount;

	double sharp = atof(c.c_str())+1.0;
	double x = -((sharp-1)/4.0);
	kernel[0][1] = x;
	kernel[1][0] = x;
	kernel[1][2] = x;
	kernel[2][1] = x;
	kernel[1][1] = sharp;
	bool result = true;

	wxConfigBase::Get()->Read("tool.sharpen.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();
	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("sharpen..."));
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	if (sharp > 1.0) {
		mark();
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedConvolve(getPreviousPicProcessor()->getProcessedPic(), dib, i,threadcount, kernel));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			t.pop_back();
		}
		wxString d = duration();
		if (wxConfigBase::Get()->Read("tool.sharpen.log","0") == "1")
			log(wxString::Format("tool=sharpen,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

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




