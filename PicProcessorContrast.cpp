#include "PicProcessor.h"
#include "PicProcessorContrast.h"
#include "PicProcPanel.h"
//#include <gimage/gimage.h>
#include "undo.xpm"

#include "util.h"
#include <wx/fileconf.h>


class ContrastPanel: public PicProcPanel
{
	public:
		ContrastPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);


			int initialvalue = atoi(params.c_str());

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "contrast: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			contrast = new wxSlider(this, wxID_ANY, initialvalue, -100, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(contrast , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, params, wxDefaultPosition, wxSize(30, -1));
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
			Bind(wxEVT_BUTTON, &ContrastPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &ContrastPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ContrastPanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &ContrastPanel::OnTimer,  this);
		}

		~ContrastPanel()
		{
			t->~wxTimer();
		}
/*
		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%d",contrast->GetValue()));
			q->processPic();
			event.Skip();
		}
*/
		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", contrast->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", contrast->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",contrast->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval;
			wxConfigBase::Get()->Read("tool.contrast.initialvalue",&resetval,0);
			contrast->SetValue(resetval);
			q->setParams(wxString::Format("%d",resetval));
			val->SetLabel(wxString::Format("%4d", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *contrast;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxTimer *t;

};


PicProcessorContrast::PicProcessorContrast(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//showParams();
}

void PicProcessorContrast::createPanel(wxSimplebook* parent)
{
	toolpanel = new ContrastPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
}

void PicProcessorContrast::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new ContrastPanel(m_parameters, this, c);
}


bool PicProcessorContrast::processPic() {
	((wxFrame*) m_display->GetParent())->SetStatusText("contrast...");
	double contrast = atof(c.c_str());
	bool result = true;

	Curve ctrlpts;
	if (contrast < 0) {
		ctrlpts.insertpoint(0,-contrast);
		ctrlpts.insertpoint(255,255+contrast);
	}
	else {
		ctrlpts.insertpoint(contrast,0);
		ctrlpts.insertpoint(255-contrast,255);
	}

	int threadcount;
	wxConfigBase::Get()->Read("tool.contrast.cores",&threadcount,0);
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.contrast.log","0") == "1"))
		log(wxString::Format("tool=contrast,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	processNext();

	return result;
}



