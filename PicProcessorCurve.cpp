
#include "PicProcessor.h"
#include "PicProcessorCurve.h"
#include "ThreadedCurve.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "Curve.h"
#include "CurvePane.h"
#include "util.h"

#include <wx/fileconf.h>
#include <vector>

class CurvePanel: public PicProcPanel
{
	public:
		CurvePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			curve = new CurvePane(this, params);
			b->Add(curve, flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Bind(wxEVT_SCROLL_THUMBRELEASE, &CurvePanel::paramChanged, this);
		}

		~CurvePanel()
		{
			curve->~CurvePane();
		}


		void paramChanged(wxScrollEvent& event)
		{
			((PicProcessorCurve *) q)->setControlPoints(curve->getPoints());
			q->setParams(curve->getControlPoints());
			q->processPic();
			event.Skip();
		}


	private:
		CurvePane *curve;

};

PicProcessorCurve::PicProcessorCurve(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	Curve crv;
	wxArrayString cpts = split(command,",");
	for (int i=0; i<cpts.GetCount()-1; i+=2) {
		crv.insertpoint(atof(cpts[i]), atof(cpts[i+1]));
	}
	ctrlpts = crv.getControlPoints();
	showParams();
}

void PicProcessorCurve::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new CurvePanel(m_parameters, this, c);
	m_parameters->Refresh();
	m_parameters->Update();
}

void PicProcessorCurve::setControlPoints(std::vector<cp> ctpts)
{
	ctrlpts.clear();
	ctrlpts = ctpts;
}

void PicProcessorCurve::setParams(std::vector<cp> ctpts, wxString params)
{
	PicProcessor::setParams(params);
	ctrlpts.clear();
	ctrlpts = ctpts;
}

bool PicProcessorCurve::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("curve...");
	int threadcount;
	std::vector<ThreadedCurve *> t;
	bool result = true;
	Curve c;
	BYTE LUT8[256];
	WORD LUT16[65535];

	wxConfigBase::Get()->Read("tool.curve.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();
	mark();

	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	c.setControlPoints(ctrlpts);
	int bpp = FreeImage_GetBPP(dib);
	if (bpp == 24) {
		c.clampto(0.0,255.0);
		for (int x=0; x<256; x++) {
			LUT8[x] = (BYTE)floor(c.getpoint(x) + 0.5);
		}
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedCurve(getPreviousPicProcessor()->getProcessedPic(), dib, i,threadcount, LUT8));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			t.pop_back();
		}
	}
	if (bpp == 48) {
		c.scalepoints(256.0);
		c.clampto(0.0,65535.0);
		for (int x=0; x<65536; x++) {
			LUT16[x] = (WORD)floor(c.getpoint(x) + 0.5);
		}
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedCurve(getPreviousPicProcessor()->getProcessedPic(), dib, i,threadcount, LUT16));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			t.pop_back();
		}
	}

	wxString d = duration();
	if (wxConfigBase::Get()->Read("tool.curve.log","0") == "1")
		log(wxString::Format("tool=curve,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	dirty = false;
	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



