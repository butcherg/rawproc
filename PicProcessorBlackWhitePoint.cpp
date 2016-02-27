
#include "PicProcessor.h"
#include "PicProcessorBlackWhitePoint.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"
#include "util.h"

class BlackWhitePointPanel: public PicProcPanel
{
	public:
		BlackWhitePointPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxArrayString p = split(params,",");

			double blk = atof(p[0]);
			double wht = atof(p[1]);

			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			black = new myTouchSlider((wxFrame *) this, wxID_ANY, "black", SLIDERWIDTH, blk, 1.0, 0.0, 255.0, "%2.0f");
			white = new myTouchSlider((wxFrame *) this, wxID_ANY, "white", SLIDERWIDTH, wht, 1.0, 0.0, 255.0, "%2.0f");
			b->Add(50,100,1);
			b->Add(black, flags);
			b->Add(white, flags);
			b->Add(50,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			//slide->Bind(wxEVT_SCROLL_THUMBRELEASE,&ShadowPanel::paramChanged,this);
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(BlackWhitePointPanel::paramChanged));
		}

		~BlackWhitePointPanel()
		{
			black->~myTouchSlider();
			white->~myTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%0.2f,%0.2f",black->GetDoubleValue(),white->GetDoubleValue()));
			q->processPic();
			event.Skip();
		}


	private:
		myTouchSlider *black, *white;

};


PicProcessorBlackWhitePoint::PicProcessorBlackWhitePoint(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorBlackWhitePoint::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new BlackWhitePointPanel(m_parameters, this, c);
}

bool PicProcessorBlackWhitePoint::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("highlight...");
	m_tree->SetItemBold(GetId(), true);

	wxArrayString cp = split(getParams(),",");
	double blk = atof(cp[0]);
	double wht = atof(cp[1]);

	Curve ctrlpts;
	ctrlpts.insertpoint(blk,0);
	//ctrlpts.insertpoint(thr-20,thr-20);
	//ctrlpts.insertpoint(thr,thr);
	//ctrlpts.insertpoint((thr+thr/2)-shd,(thr+thr/2)+shd);
	ctrlpts.insertpoint(wht,255);
	
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		m_tree->SetItemBold(GetId(), true);
		if (!FreeImage_AdjustCurveControlPoints(dib, ctrlpts.getControlPoints(), FICC_RGB)) 
			result = false;
		else
			dirty = false;
		if (prev) FreeImage_Unload(prev);

		//put in every processPic()...
		if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
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



