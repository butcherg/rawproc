#include "PicProcessor.h"
#include "PicProcessorContrast.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "wxTouchSlider.h"

class ContrastPanel: public PicProcPanel
{
	public:
		ContrastPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			slide = new wxTouchSlider((wxFrame *) this, "", atoi(p.c_str()), -100, 100);
			b->Add(slide, flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(ContrastPanel::paramChanged));
		}

		~ContrastPanel()
		{
			slide->~wxTouchSlider();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",event.GetInt()));
			event.Skip();
		}


	private:
		//wxSlider *slide;
		wxTouchSlider *slide;

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
	m_tree->SetItemBold(GetId(), true);
	double contrast = atof(c.c_str());
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		m_tree->SetItemBold(GetId(), true);
		if (bpp == 8 |bpp == 24 | bpp == 32) {
			if (!FreeImage_AdjustContrast(dib,contrast)) {
				result = false;
			}
		}
		else if(bpp == 48) {
			WORD LUT[65535];
			FreeImage_GetAdjustColorsLookupTable16(LUT, 0.0, contrast, 0.0, false);
			if (!FreeImage_AdjustCurve16(dib, LUT, FICC_RGB)) {
				result = false;;
			}
		}
		else result = false; 
		if (prev) FreeImage_Unload(prev);

		//put in every processPic()...
		if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
		wxTreeItemId next = m_tree->GetNextSibling(GetId());
		if (next.IsOk()) {
			PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
			nextitem->processPic();
		}
		m_tree->SetItemBold(GetId(), false);
	}
	else {
		
		result =  false;
	}
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



