#include "PicProcessorGamma.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"


class GammaPanel: public PicProcPanel
{

	public:
		GammaPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			b->Add(new wxStaticText(this,-1, "gamma", wxDefaultPosition, wxSize(100,20)), flags);
			edit = new wxTextCtrl(this, wxID_ANY, p, wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			b->Add(edit, flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			Connect(wxID_ANY,wxEVT_TEXT_ENTER,wxCommandEventHandler(GammaPanel::paramChanged));
		}

		~GammaPanel()
		{
			edit->~wxTextCtrl();
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(edit->GetLineText(0));
			event.Skip();
		}


	private:
		wxTextCtrl *edit;

};

PicProcessorGamma::PicProcessorGamma(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command, tree, display, parameters) {
	//p->DestroyChildren();
	//r = new GammaPanel(p, this, c);
	showParams();

}

void PicProcessorGamma::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new GammaPanel(m_parameters, this, c);
}


bool PicProcessorGamma::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("gamma...");
	m_tree->SetItemBold(GetId(), true);
	double gamma = atof(c.c_str());
	bool result = true;
	FIBITMAP *prev = dib;
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	if (dib) {
		int bpp = FreeImage_GetBPP(dib);
		m_tree->SetItemBold(GetId(), true);
		if (bpp == 8 |bpp == 24 | bpp == 32) {
			if (!FreeImage_AdjustGamma(dib,gamma)) {
				result = false;
			}
		}
		else if(bpp == 48) {
			WORD LUT[65535];
			FreeImage_GetAdjustColorsLookupTable16(LUT, 0.0, 0.0, gamma, false);
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
		
		result = false;
	}
	m_tree->SetItemBold(GetId(), false);
	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}





