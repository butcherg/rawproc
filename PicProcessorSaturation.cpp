#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "wxTouchSlider.h"

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			slide = new wxTouchSlider((wxFrame *) this, "", atoi(p.c_str()), 0, 30);
			b->Add(slide, 1, wxALIGN_LEFT |wxALIGN_TOP |wxEXPAND, 10);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(SaturationPanel::paramChanged));
		}

		~SaturationPanel()
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
	double saturation = atof(c.c_str())/10.0;
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



