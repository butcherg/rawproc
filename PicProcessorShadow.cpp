
#include "PicProcessor.h"
#include "PicProcessorShadow.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"
#include "util.h"

#include <vector>
wxString getCPs(Curve ctrlpts)
{
	wxString s = "";
	bool first = true;
	std::vector<cp> pts = ctrlpts.getControlPoints();
	for (unsigned int i=0; i<pts.size(); i++) {
		if (!first) s.Append(",");
		first=false;
		s.Append(wxString::Format("%.1f,%.1f",pts[i].x,pts[i].y));
	}
	return s;
}

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
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "shadow", 60, shd, 1.0, -50.0, 50.0, "%2.0f");
			threshold = new myTouchSlider((wxFrame *) this, wxID_ANY, "threshold", 60, thr, 1.0, 32.0, 128.0, "%2.0f");
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
			q->setParams(wxString::Format("%02f,%02f",slide->GetDoubleValue(),threshold->GetDoubleValue()));
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
	((wxFrame*) m_parameters->GetParent())->SetStatusText("shadow...");
	m_tree->SetItemBold(GetId(), true);

	wxArrayString cp = split(getParams(),",");
	double shd = atof(cp[0]);
	double thr = atof(cp[1]);

	Curve ctrlpts;
	ctrlpts.insertpoint(0,0);
	ctrlpts.insertpoint((thr/2)-shd,(thr/2)+shd);
	ctrlpts.insertpoint(thr,thr);
	ctrlpts.insertpoint(thr+20,thr+20);
	ctrlpts.insertpoint(255,255);

	
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		m_tree->SetItemBold(GetId(), true);
		if (!FreeImage_AdjustCurveControlPoints(dib, ctrlpts.getControlPoints(), FICC_RGB)) result = false;
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



