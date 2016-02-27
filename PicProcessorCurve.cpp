
#include "PicProcessor.h"
#include "PicProcessorCurve.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "Curve.h"
#include "CurvePane.h"
#include "util.h"

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
	m_tree->SetItemBold(GetId(), true);
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		m_tree->SetItemBold(GetId(), true);

		if (!FreeImage_AdjustCurveControlPoints(dib, ctrlpts, FICC_RGB)) {
			//wxMessageBox("curve processing didn't go so well...");
			result = false;
		}
		else dirty = false;
		if (prev) FreeImage_Unload(prev);

		//put in every processPic()...
		if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
		m_tree->SetItemBold(GetId(), false);
		wxTreeItemId next = m_tree->GetNextSibling(GetId());
		if (next.IsOk()) {
			PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
			nextitem->processPic();
		}
	}
	else result = false;
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



