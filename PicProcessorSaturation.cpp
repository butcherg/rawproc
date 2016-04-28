#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "ThreadedSaturate.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"
#include "util.h"

#include <wx/fileconf.h>

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "saturate", SLIDERWIDTH, atof(p.c_str()), 0.1, 0.0, 3.0, "%3.1f");
			b->Add(100,100,1);
			b->Add(slide, flags);
			b->Add(100,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(SaturationPanel::paramChanged));
		}

		~SaturationPanel()
		{
			slide->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{

			q->setParams(wxString::Format("%3.1f",slide->GetDoubleValue()));
			q->processPic();
			event.Skip();
		}


	private:
		myTouchSlider *slide;

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
	((wxFrame*) m_parameters->GetParent())->SetStatusText("saturation...");
	double saturation = atof(c.c_str());
	bool result = true;
	std::vector<ThreadedSaturate *> t;
	int threadcount = 1;
	wxConfigBase::Get()->Read("tool.saturate.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();
	((wxFrame*) m_parameters->GetParent())->SetStatusText(wxString::Format("saturate, %d cores...",threadcount));
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	if (saturation != 1.0) {
		mark();
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedSaturate(getPreviousPicProcessor()->getProcessedPic(), dib, i,threadcount, saturation));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			t.pop_back();
		}
		wxString d = duration();
		if (wxConfigBase::Get()->Read("tool.saturate.log","0") == "1")
			log(wxString::Format("tool=saturate,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	}
	dirty = false;

		//put in every processPic()...
		if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
		m_tree->SetItemBold(GetId(), false);
		wxTreeItemId next = m_tree->GetNextSibling(GetId());
		if (next.IsOk()) {
			PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
			nextitem->processPic();
		}


	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



