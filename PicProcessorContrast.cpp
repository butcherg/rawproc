#include "PicProcessor.h"
#include "PicProcessorContrast.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"

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
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		if (bpp == 8 |bpp == 24 | bpp == 32) {
			if (!FreeImage_AdjustContrast(dib,contrast)) {
				result = false;
			}
			else dirty = false;
		}
		else if(bpp == 48) {
			WORD LUT[65535];
			FreeImage_GetAdjustColorsLookupTable16(LUT, 0.0, contrast, 0.0, false);
			if (!FreeImage_AdjustCurve16(dib, LUT, FICC_RGB)) {
				result = false;;
			}
			else dirty = false;
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
	}
	else {
		
		result =  false;
	}
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



