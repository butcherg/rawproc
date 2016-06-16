#include "PicProcessorGamma.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"

#include "util.h"
#include "ThreadedCurve.h"
#include <wx/fileconf.h>


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
	BYTE LUT8[256];
	WORD LUT16[65536];
	((wxFrame*) m_display->GetParent())->SetStatusText("gamma...");
	double gamma = atof(c.c_str());
	bool result = true;
	int threadcount;
	wxConfigBase::Get()->Read("tool.gamma.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();

	mark();
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());

	int bpp = FreeImage_GetBPP(dib);
	if (bpp == 24) {
		double exponent = 1 / gamma;
		double v = 255.0 * (double)pow((double)255, -exponent);
		for(int i = 0; i < 256; i++) {
			double color = (double)pow((double)i, exponent) * v;
			if(color > 255)
				color = 255;
			LUT8[i] = (BYTE)floor(color + 0.5);
		}
		ThreadedCurve::ApplyLUT(getPreviousPicProcessor()->getProcessedPic(), dib, LUT8, threadcount);

	}
	else if(bpp == 48) {
		double exponent = 1 / gamma;
		double v = 65535.0 * (double)pow((double)65535, -exponent);
		for(int i = 0; i < 65536; i++) {
			double color = (double)pow((double)i, exponent) * v;
			if(color > 65535)
				color = 65535;
			LUT16[i] = (WORD)floor(color + 0.5);
		}
		ThreadedCurve::ApplyLUT(getPreviousPicProcessor()->getProcessedPic(), dib, LUT16, threadcount);

	}
	else result = false; 
	wxString d = duration();

	if (wxConfigBase::Get()->Read("tool.gamma.log","0") == "1")
		log(wxString::Format("tool=gamma,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	dirty = false;

	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return result;
}





