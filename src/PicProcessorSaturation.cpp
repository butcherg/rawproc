#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "undo.xpm"

#define SATURATIONENABLE 7600

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxGridBagSizer *g = new wxGridBagSizer();

			double initialvalue = atof(params.c_str());

			enablebox = new wxCheckBox(this, SATURATIONENABLE, _("saturation:"));
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxGBSpan(1,3), wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			saturate = new wxSlider(this, wxID_ANY, initialvalue*10.0, 0, 30, wxPoint(10, 30), wxSize(140, -1));
			g->Add(saturate, wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f", initialvalue), wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset to default"));
			g->Add(btn, wxGBPosition(2,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &SaturationPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &SaturationPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &SaturationPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &SaturationPanel::onEnable, this, SATURATIONENABLE);
			Bind(wxEVT_TIMER, &SaturationPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &SaturationPanel::OnKey,  this);
		}

		void OnKey(wxKeyEvent& event)
		{
			wxChar uc = event.GetUnicodeKey();
			if ( uc != WXK_NONE )
			{
				// It's a "normal" character. Notice that this includes
				// control characters in 1..31 range, e.g. WXK_RETURN or
				// WXK_BACK, so check for them explicitly.
				if ( uc >= 32 )
				{
					switch (uc) {
					}
				}
				else
				{
					// It's a control character, < WXK_START
					switch (uc)
					{
						case WXK_TAB:
							event.Skip();
							break;
					}
				}
			}
			else // No Unicode equivalent.
			{
				// It's a special key, > WXK_START, deal with all the known ones:
				switch ( event.GetKeyCode() )
				{
				}
			}
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				q->processPic();
			}
			else {
				q->enableProcessing(false);
				q->processPic();
			}
		}

		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", saturate->GetValue()/10.0));
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", saturate->GetValue()/10.0));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",saturate->GetValue()/10.0));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval = atof(myConfig::getConfig().getValueOrDefault("tool.saturate.initialvalue","1.0").c_str());
			saturate->SetValue(resetval*10.0);
			q->setParams(wxString::Format("%2.2f",resetval));
			val->SetLabel(wxString::Format("%2.2f", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *saturate;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer t;

};


PicProcessorSaturation::PicProcessorSaturation(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorSaturation::createPanel(wxSimplebook* parent)
{
	toolpanel = new SaturationPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSaturation::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText(_("saturation..."));
	double saturation = atof(c.c_str());
	bool result = true;
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.saturate.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;

	if (processingenabled & saturation != 1.0) {
		mark();
		dib->ApplySaturate(saturation, threadcount);
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.saturate.log","0") == "1"))
			log(wxString::Format(_("tool=saturate,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));

	}
	
	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}



