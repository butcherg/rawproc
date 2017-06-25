#include "PicProcessor.h"
#include "PicProcessorExposure.h"
#include "PicProcPanel.h"
#include "util.h"
#include "undo.xpm"

#include <wx/fileconf.h>

class ExposurePanel: public PicProcPanel
{
	public:
		ExposurePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			double initialvalue = atof(params.c_str());

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "exposure:"), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			ev = new wxSlider(this, wxID_ANY, 50.0+(initialvalue*10.0), 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(ev, wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f", initialvalue), wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			g->Add(btn, wxGBPosition(1,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &ExposurePanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &ExposurePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ExposurePanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &ExposurePanel::OnTimer,  this);		}

		~ExposurePanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",(ev->GetValue()-50.0)/10.0));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval;
			wxConfigBase::Get()->Read("tool.exposure.initialvalue",&resetval,0.0);
			ev->SetValue(50.0+(resetval*10));
			q->setParams(wxString::Format("%2.2f",resetval));
			val->SetLabel(wxString::Format("%2.2f", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *ev;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxTimer *t;

};


PicProcessorExposure::PicProcessorExposure(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorExposure::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new ExposurePanel(m_parameters, this, c);
}


bool PicProcessorExposure::processPic() 
{
	double ev = atof(c.c_str());
	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("exposure %2.2f...", ev));
	bool result = true;
	int threadcount;
	wxConfigBase::Get()->Read("tool.exposure.cores",&threadcount,0);
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	
	mark();
	dib->ApplyExposureCompensation(ev, threadcount);
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.exposure.log","0") == "1"))
		log(wxString::Format("tool=exposure,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	processNext();
	
	return result;
}



