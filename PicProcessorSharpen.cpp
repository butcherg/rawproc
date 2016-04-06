#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"

class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "sharpen", SLIDERWIDTH, atof(p.c_str()), 1.0, 1.0, 10.0, "%2.0f");
			b->Add(100,100,1);
			b->Add(slide, flags);
			b->Add(100,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(SharpenPanel::paramChanged));
		}

		~SharpenPanel()
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


PicProcessorSharpen::PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new SharpenPanel(p,this,c);
	showParams();
}

void PicProcessorSharpen::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new SharpenPanel(m_parameters, this, c);
}


bool PicProcessorSharpen::processPic() {
	m_tree->SetItemBold(GetId(), true);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("sharpen...");
	double sharp = atof(c.c_str());
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		if (bpp == 8 |bpp == 24 | bpp == 32) {
			if (!FreeImage_AdjustBrightness(dib,sharp)) {
				result = false;
			}
			else dirty = false;
		}
		else if(bpp == 48) {
			WORD LUT[65535];
			FreeImage_GetAdjustColorsLookupTable16(LUT, sharp, 0.0, 0.0, false);
			if (!FreeImage_AdjustCurve16(dib, LUT, FICC_RGB)) {
				result = false;;
			}
			else dirty = false;
		}
		else result = false; 
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
	else {
		
		result = false;
	}
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



