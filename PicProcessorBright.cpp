#include "PicProcessor.h"
#include "PicProcessorBright.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
//#include "myTouchSlider.h"
//#include "myFloatSlider.h"

#include "util.h"
#include "ThreadedCurve.h"
#include <wx/fileconf.h>

class BrightPanel: public PicProcPanel
{
	public:
		BrightPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "bright: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			bright = new wxSlider(this, wxID_ANY, 0, -100, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(bright , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, "  0", wxDefaultPosition, wxSize(140, -1));
			g->Add(val , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_SCROLL_CHANGED, &BrightPanel::OnChanged, this);
			Bind(wxEVT_TIMER, &BrightPanel::OnTimer,  this);
		}

		~BrightPanel()
		{
			bright->~wxSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",bright->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", bright->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",bright->GetValue()));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *bright;
		wxStaticText *val;
		wxTimer *t;

};


PicProcessorBright::PicProcessorBright(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorBright::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new BrightPanel(m_parameters, this, c);
}


bool PicProcessorBright::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("bright...");
	double bright = atof(c.c_str());
	bool result = true;

	Curve ctrlpts;
	ctrlpts.insertpoint(0,0);
	if (bright < 0)
		ctrlpts.insertpoint(255,255+bright);
	else
		ctrlpts.insertpoint(255-bright,255);

	int threadcount;
	wxConfigBase::Get()->Read("tool.bright.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();

	mark();
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	ThreadedCurve::ApplyCurve(getPreviousPicProcessor()->getProcessedPic(), dib, ctrlpts.getControlPoints(), threadcount);
	wxString d = duration();

	if (wxConfigBase::Get()->Read("tool.bright.log","0") == "1")
		log(wxString::Format("tool=bright,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));
	dirty=false;


	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

	return result;
}



