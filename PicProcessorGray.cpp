
#include "PicProcessor.h"
#include "PicProcessorGray.h"
#include "FreeImage_Threaded.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "undo.xpm"

#include "util.h"

#include <wx/fileconf.h>

#define REDSLIDER   7000
#define GREENSLIDER 7001
#define BLUESLIDER  7002

class GrayPanel: public PicProcPanel
{

	public:
		GrayPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			//c = new wxBoxSizer(wxHORIZONTAL); 
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM).Expand();
			wxArrayString p = split(params,",");

			rd = atof(p[0]);
			gr = atof(p[1]);
			bl = atof(p[2]);

			g->Add(0,10, wxGBPosition(0,0));

			g->Add(new wxStaticText(this,wxID_ANY, "red: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			red = new wxSlider(this, wxID_ANY, rd*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(red , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",rd), wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "green: "), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			green = new wxSlider(this, wxID_ANY, gr*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(green , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",gr), wxDefaultPosition, wxSize(30, -1));
			g->Add(val2 , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			g->Add(new wxStaticText(this,wxID_ANY, "blue: "), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			blue = new wxSlider(this, wxID_ANY, bl*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(blue , wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val3 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",bl), wxDefaultPosition, wxSize(30, -1));
			g->Add(val3 , wxGBPosition(3,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			val4 = new wxStaticText(this,wxID_ANY, wxString::Format("Total: %2.2f", rd+gr+bl), wxDefaultPosition, wxSize(-1,-1));
			g->Add(val4, wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset RGB proportions to defaults");
			g->Add(btn, wxGBPosition(4,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &GrayPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &GrayPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &GrayPanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &GrayPanel::OnTimer,  this);		}

		~GrayPanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			rd = red->GetValue()/100.0;
			gr = green->GetValue()/100.0;
			bl = blue->GetValue()/100.0;
			val1->SetLabel(wxString::Format("%2.2f", rd));
			val2->SetLabel(wxString::Format("%2.2f", gr));
			val3->SetLabel(wxString::Format("%2.2f", bl));
			val4->SetLabel(wxString::Format("Total: %2.2f", rd+gr+bl));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			rd = red->GetValue()/100.0;
			gr = green->GetValue()/100.0;
			bl = blue->GetValue()/100.0;
			val1->SetLabel(wxString::Format("%2.2f", rd));
			val2->SetLabel(wxString::Format("%2.2f", gr));
			val3->SetLabel(wxString::Format("%2.2f", bl));
			val4->SetLabel(wxString::Format("Total: %2.2f", rd+gr+bl));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",red->GetValue()/100.0,green->GetValue()/100.0,blue->GetValue()/100.0));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetredval, resetgreenval, resetblueval;

			wxConfigBase::Get()->Read("tool.gray.r",&resetredval,0.21);
			red->SetValue(resetredval*100.0);
			val1->SetLabel(wxString::Format("%2.2f", resetredval));

			wxConfigBase::Get()->Read("tool.gray.g",&resetgreenval,0.72);
			green->SetValue(resetgreenval*100.0);
			val2->SetLabel(wxString::Format("%2.2f", resetgreenval));

			wxConfigBase::Get()->Read("tool.gray.b",&resetblueval,0.07);
			blue->SetValue(resetblueval*100.0);
			val3->SetLabel(wxString::Format("%2.2f", resetblueval));

			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",red->GetValue()/100.0,green->GetValue()/100.0,blue->GetValue()/100.0));
			q->processPic();
			event.Skip();

		}



	private:
		double rd, gr, bl;

		wxSlider *red, *green, *blue;
		wxStaticText *val1, *val2, *val3, *val4;
		wxBitmapButton *btn;
		wxTimer *t;

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
	((wxFrame*) m_display->GetParent())->SetStatusText("gray...");
	wxArrayString cp = split(getParams(),",");
	double r = atof(cp[0]);
	double g = atof(cp[1]);
	double b = atof(cp[2]);
	bool result = true;

	int threadcount;
	wxConfigBase::Get()->Read("tool.gray.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();

	mark();
	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Clone(getPreviousPicProcessor()->getProcessedPic());
	ApplyGray(getPreviousPicProcessor()->getProcessedPic(), dib, r, g, b, threadcount);
	wxString d = duration();

	if (wxConfigBase::Get()->Read("tool.gray.log","0") == "1")
		log(wxString::Format("tool=curve,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

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



