#include "PicProcessor.h"
#include "PicProcessorHLRecover.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "undo.xpm"
#include "util.h"

#include <vector>

#define HLRECOVERENABLE 8400

class HLRecoverPanel: public PicProcPanel
{
	public:
		HLRecoverPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxGridBagSizer *g = new wxGridBagSizer();
			bool low = true;

			int initialvalue = atoi(params.c_str());

			enablebox = new wxCheckBox(this, HLRECOVERENABLE, _("hlrecover:"));
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxGBSpan(1,3), wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			//sharp = new wxSlider(this, wxID_ANY, initialvalue, 0, 10, wxPoint(10, 30), wxSize(140, -1));
			//g->Add(sharp , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			//val = new wxStaticText(this,wxID_ANY, params, wxDefaultPosition, wxSize(30, -1));
			//g->Add(val , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			//btn->SetToolTip("Reset to default");
			g->Add(btn, wxGBPosition(2,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			//t.SetOwner(this);
			Bind(wxEVT_BUTTON, &HLRecoverPanel::OnButton, this);
			//Bind(wxEVT_SCROLL_CHANGED, &HLRecoverPanel::OnChanged, this);
			//Bind(wxEVT_SCROLL_THUMBTRACK, &HLRecoverPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &HLRecoverPanel::onEnable, this, HLRECOVERENABLE);
			//Bind(wxEVT_TIMER, &HLRecoverPanel::OnTimer,  this);
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
			q->processPic();
		}
/*
		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", sharp->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",sharp->GetValue()));
			q->processPic();
			event.Skip();
		}
*/

		void OnButton(wxCommandEvent& event)
		{
			//int resetval = atoi(myConfig::getConfig().getValueOrDefault("tool.sharpen.initialvalue","0").c_str());
			//sharp->SetValue(resetval);
			//q->setParams(wxString::Format("%d",resetval));
			//val->SetLabel(wxString::Format("%4d", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		//wxSlider *sharp;
		//wxStaticText *val;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		//wxTimer t;
		bool low;


};


PicProcessorHLRecover::PicProcessorHLRecover(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorHLRecover::createPanel(wxSimplebook* parent)
{
	toolpanel = new HLRecoverPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorHLRecover::processPicture(gImage *processdib) 
{
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.hlrecover.cores","0").c_str());

	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format(_("hlrecover...")));

	dib = processdib;
	if (!global_processing_enabled) return true;

	if (global_processing_enabled & processingenabled) {
		mark();
		dib->ApplyHLRecover(threadcount);
		//if (!dib->ApplyHaldCLUT("/home/glenn/Photography/Hald_CLUT_Identity.png", threadcount)) wxMessageBox("HaldCLUT didn't work, for some reason...");

		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.hlrecover.log","0") == "1"))
			log(wxString::Format(_("tool=hlrecover,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));

	}

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");

	return result;
}




