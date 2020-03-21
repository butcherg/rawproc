#include "PicProcessor.h"
#include "PicProcessorSaturation.h"
#include "PicProcPanel.h"
#include "myFloatCtrl.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "util.h"
#include "undo.xpm"

#define SATURATIONENABLE 7600

class SaturationPanel: public PicProcPanel
{
	public:
		SaturationPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());

			double initialvalue = atof(params.c_str());

			enablebox = new wxCheckBox(this, SATURATIONENABLE, _("saturation:"));
			enablebox->SetValue(true);

			saturate = new myFloatCtrl(this, wxID_ANY, 1.0, 2, wxDefaultPosition,wxDefaultSize);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset to default"));
			
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxSizerFlags patchflags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(saturate,flags);
			m->AddRowItem(btn,flags);

			m->End();

			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);
			Bind(myFLOATCTRL_CHANGE,&SaturationPanel::OnChanged, this);
			Bind(myFLOATCTRL_UPDATE,&SaturationPanel::OnEnter, this);
			Bind(wxEVT_CHECKBOX, &SaturationPanel::onEnable, this, SATURATIONENABLE);
			Bind(wxEVT_TIMER, &SaturationPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &SaturationPanel::OnKey,  this);
			Thaw();
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

		void OnEnter(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			q->processPic();
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",saturate->GetFloatValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval = atof(myConfig::getConfig().getValueOrDefault("tool.saturate.initialvalue","1.0").c_str());
			saturate->SetFloatValue(resetval);
			q->setParams(wxString::Format("%2.2f",resetval));
			q->processPic();
			event.Skip();
		}


	private:
		myFloatCtrl *saturate;
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



