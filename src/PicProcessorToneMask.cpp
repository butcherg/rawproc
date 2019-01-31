#include "PicProcessor.h"
#include "PicProcessorToneMask.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "undo.xpm"
#include "util.h"

#include <vector>

#define TONEMASKENABLE 8000

class ToneMaskPanel: public PicProcPanel
{
	public:
		ToneMaskPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, TONEMASKENABLE, "tone mask:");
			enablebox->SetValue(true);
			b->Add(enablebox, flags);
			b->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			b->AddSpacer(10);


			mask = new wxSlider(this, wxID_ANY, 128, 0, 255, wxPoint(10, 30), wxSize(140, -1));
			b->Add(mask , flags);
			val = new wxStaticText(this,wxID_ANY, "128", wxDefaultPosition, wxSize(30, -1));
			b->Add(val , flags);
			

			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &ToneMaskPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &ToneMaskPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ToneMaskPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &ToneMaskPanel::onEnable, this, TONEMASKENABLE);
			Bind(wxEVT_TIMER, &ToneMaskPanel::OnTimer,  this);
		}

		~ToneMaskPanel()
		{
			t->~wxTimer();
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
			val->SetLabel(wxString::Format("%4d", mask->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", mask->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d",mask->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval = atoi(myConfig::getConfig().getValueOrDefault("tool.sharpen.initialvalue","0").c_str());
			mask->SetValue(resetval);
			q->setParams(wxString::Format("%d",resetval));
			val->SetLabel(wxString::Format("%4d", resetval));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *mask;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer *t;


};


PicProcessorToneMask::PicProcessorToneMask(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorToneMask::createPanel(wxSimplebook* parent)
{
	toolpanel = new ToneMaskPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorToneMask::processPic(bool processnext) {
	double kernel[3][3] =
	{
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	double sharp = atof(c.c_str())+1.0;
	double x = -((sharp-1)/4.0);
	kernel[0][1] = x;
	kernel[1][0] = x;
	kernel[1][2] = x;
	kernel[2][1] = x;
	kernel[1][1] = sharp;
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tonemask.cores","0").c_str());

	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("tone mask..."));


	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled & sharp > 1.0) {
		mark();
		dib->ApplyConvolutionKernel(kernel, threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.tonemask.log","0") == "1"))
			log(wxString::Format("tool=ToneMask,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	}

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}




