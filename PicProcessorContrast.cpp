#include "PicProcessor.h"
#include "PicProcessorContrast.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "myTouchSlider.h"

#include "util.h"
#include "ThreadedCurve.h"
#include <wx/fileconf.h>

class ContrastPanel: public PicProcPanel
{
	public:
		ContrastPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "contrast", SLIDERWIDTH, atof(p.c_str()), 1.0, -100.0, 100.0, "%2.0f");
			b->Add(100,100,1);
			b->Add(slide, flags);
			b->Add(100,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(ContrastPanel::paramChanged));
		}

		~ContrastPanel()
		{
			slide->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",slide->GetIntValue()));
			q->processPic();
			event.Skip();
		}


	private:
		myTouchSlider *slide;

};


PicProcessorContrast::PicProcessorContrast(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorContrast::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new ContrastPanel(m_parameters, this, c);
}


bool PicProcessorContrast::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("contrast...");
	double contrast = atof(c.c_str());
	bool result = true;

	Curve ctrlpts;
	//ctrlpts.insertpoint(0,0);
	if (contrast < 0) {
		ctrlpts.insertpoint(0,-contrast);
		ctrlpts.insertpoint(255,255+contrast);
	}
	else {
		ctrlpts.insertpoint(contrast,0);
		ctrlpts.insertpoint(255-contrast,255);
	}

	int threadcount;
	wxConfigBase::Get()->Read("tool.contrast.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();

	mark();
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	ThreadedCurve::ApplyCurve(getPreviousPicProcessor()->getProcessedPic(), dib, ctrlpts.getControlPoints(), threadcount);
	wxString d = duration();

	if (wxConfigBase::Get()->Read("tool.contrast.log","0") == "1")
		log(wxString::Format("tool=contrast,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	dirty = false;
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



