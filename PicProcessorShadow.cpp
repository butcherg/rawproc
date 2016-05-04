       
#include "PicProcessor.h"
#include "PicProcessorShadow.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "myTouchSlider.h"

#include "util.h"
#include "ThreadedCurve.h"
#include <wx/fileconf.h>

class ShadowPanel: public PicProcPanel
{
	public:
		ShadowPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxArrayString p = split(params,",");

			double shd = atof(p[0]);
			double thr = atof(p[1]);

			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "shadow", SLIDERWIDTH, shd, 1.0, -50.0, 50.0, "%2.0f");
			threshold = new myTouchSlider((wxFrame *) this, wxID_ANY, "threshold", SLIDERWIDTH, thr, 1.0, 32.0, 128.0, "%2.0f");
			b->Add(50,100,1);
			b->Add(slide, flags);
			b->Add(threshold, flags);
			b->Add(50,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			//slide->Bind(wxEVT_SCROLL_THUMBRELEASE,&ShadowPanel::paramChanged,this);
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(ShadowPanel::paramChanged));
		}

		~ShadowPanel()
		{
			slide->~myTouchSlider();
			threshold->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%0.2f,%0.2f",slide->GetDoubleValue(),threshold->GetDoubleValue()));
			q->processPic();
			event.Skip();
		}


	private:
		myTouchSlider *slide, *threshold;

};


PicProcessorShadow::PicProcessorShadow(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorShadow::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new ShadowPanel(m_parameters, this, c);
}

bool PicProcessorShadow::processPic() {
	bool result = true;
	((wxFrame*) m_parameters->GetParent())->SetStatusText("shadow...");

	wxArrayString cp = split(getParams(),",");
	double shd = atof(cp[0]);
	double thr = atof(cp[1]);

	Curve ctrlpts;
	ctrlpts.insertpoint(0,0);
	ctrlpts.insertpoint((thr/2)-shd,(thr/2)+shd);
	ctrlpts.insertpoint(thr,thr);
	ctrlpts.insertpoint(thr+20,thr+20);
	ctrlpts.insertpoint(255,255);

	int threadcount;
	wxConfigBase::Get()->Read("tool.highlight.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();

	mark();
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	ThreadedCurve::ApplyCurve(getPreviousPicProcessor()->getProcessedPic(), dib, ctrlpts.getControlPoints(), threadcount);
	wxString d = duration();

	if (wxConfigBase::Get()->Read("tool.highlight.log","0") == "1")
		log(wxString::Format("tool=highlight,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

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



