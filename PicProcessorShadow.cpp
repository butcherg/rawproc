
#include "PicProcessor.h"
#include "PicProcessorShadow.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "wxTouchSlider.h"

class ShadowPanel: public PicProcPanel
{
	public:
		ShadowPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			slide = new wxTouchSlider((wxFrame *) this, "", atoi(p.c_str()), -50, 50);
			b->Add(slide, 1, wxALIGN_LEFT |wxALIGN_TOP |wxEXPAND, 10);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(ShadowPanel::paramChanged));
		}

		~ShadowPanel()
		{
			slide->~wxTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",event.GetInt()));
			event.Skip();
		}


	private:
		wxTouchSlider *slide;

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

	Curve ctrlpts;
	ctrlpts.insertpoint(0,0);
	ctrlpts.insertpoint(64-atoi(getParams()),64+atoi(getParams()));
	ctrlpts.insertpoint(128,128);
	ctrlpts.insertpoint(150,150);
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



