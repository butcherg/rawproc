#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "myTouchSlider.h"

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			b->SetOrientation(wxHORIZONTAL);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			slide = new myTouchSlider((wxFrame *) this, wxID_ANY, "saturate", 60, atof(p.c_str()), 0.1, 0.0, 3.0, "%3.1f");
			b->Add(100,100,1);
			b->Add(slide, flags);
			b->Add(100,100,1);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(SaturationPanel::paramChanged));
		}

		~SaturationPanel()
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


PicProcessorSaturation::PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new SaturationPanel(p,this,c);
	showParams();
}

void PicProcessorSaturation::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new SaturationPanel(m_parameters, this, c);
}


bool PicProcessorSaturation::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("saturation...");
	m_tree->SetItemBold(GetId(), true);
	double saturation = atof(c.c_str());
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		m_tree->SetItemBold(GetId(), true);
		if (bpp == 48 |bpp == 24 | bpp == 32) {
			if (!FreeImage_Saturate16(dib,saturation)) {
				result = false;
			}
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



