
#include "PicProcessor.h"
#include "PicProcessorGray.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "wxTouchSlider.h"

class GrayPanel: public PicProcPanel
{

//blank panel for now, until I implement a channel mixer...
	public:
		GrayPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			panel = new wxPanel(this);
			b->Add(panel, 1, wxALIGN_LEFT, 10);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			//slide = new wxTouchSlider((wxFrame *) this, "", atoi(p.c_str()), -50, 50);
			//b->Add(slide, 1, wxALIGN_LEFT |wxALIGN_TOP |wxEXPAND, 10);
			//SetSizerAndFit(b);
			//Connect(wxID_ANY, wxEVT_SCROLL_THUMBRELEASE,wxCommandEventHandler(GrayPanel::paramChanged));
		}

		~GrayPanel()
		{
			panel->~wxPanel();
			//slide->~wxTouchSlider();
		}

		//void paramChanged(wxCommandEvent& event)
		//{
		//	q->setParams(wxString::Format("%d",event.GetInt()));
		//	event.Skip();
		//}


	private:
		wxPanel *panel;
		//wxTouchSlider *slide;

};


PicProcessorGray::PicProcessorGray(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorGray::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new GrayPanel(m_parameters, this, c);
}

bool PicProcessorGray::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("gray...");
	m_tree->SetItemBold(GetId(), true);

	
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		if (bpp == 48 |bpp == 24 | bpp == 32) {
			if (!FreeImage_Gray16(dib, 0.21, 0.72, 0.07)) {
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
	else result = false;
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



