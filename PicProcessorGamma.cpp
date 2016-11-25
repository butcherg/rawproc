#include "PicProcessorGamma.h"
#include "PicProcPanel.h"
#include <gimage.h>
#include <omp.h>

#include "util.h"
#include "curve.h"
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
			q->processPic();
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


bool PicProcessorGamma::processPic() 
{
	Curve ctrlpts;
	((wxFrame*) m_display->GetParent())->SetStatusText("gamma...");
	double gamma = atof(c.c_str());
	bool result = true;
	int threadcount;
	wxConfigBase::Get()->Read("tool.gamma.cores",&threadcount,0);
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	double exponent = 1 / gamma;
	double v = 255.0 * (double)pow((double)255, -exponent);
	for (int i = 0; i< 256; i+=1) {
		double color = (double)pow((double)i, exponent) * v;
		if (color > 255.0) color = 255.0;
		ctrlpts.insertpoint((double) i, color);
	}	

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.gamma.log","0") == "1"))
		log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

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





