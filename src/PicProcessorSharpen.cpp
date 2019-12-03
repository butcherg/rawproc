#include "PicProcessor.h"
#include "PicProcessorSharpen.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "myFloatCtrl.h"
#include "undo.xpm"
#include "util.h"

#include <vector>

#define SHARPENENABLE 7800

class SharpenPanel: public PicProcPanel
{
	public:
		SharpenPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			int initialvalue = atoi(params.c_str());

			enablebox = new wxCheckBox(this, SHARPENENABLE, "sharpen:");
			enablebox->SetValue(true);

			sharpval = new myFloatCtrl(this, wxID_ANY, 0.0, 1, wxDefaultPosition,wxDefaultSize);
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			for (int r=0; r<3; r++)
				for (int c=0; c<3; c++)
					kernel[r][c] = new wxStaticText(this, wxID_ANY, "0.00", wxDefaultPosition, wxSize(30,-1),wxALIGN_RIGHT);
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(sharpval, flags);
			m->AddRowItem(btn, flags);
			
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "Kernel:"), flags);
			m->NextRow();
			m->AddRowItem(kernel[0][0], flags);
			m->AddRowItem(kernel[0][1], flags);
			m->AddRowItem(kernel[0][2], flags);
			m->NextRow();
			m->AddRowItem(kernel[1][0], flags);
			m->AddRowItem(kernel[1][1], flags);
			m->AddRowItem(kernel[1][2], flags);
			m->NextRow();
			m->AddRowItem(kernel[2][0], flags);
			m->AddRowItem(kernel[2][1], flags);
			m->AddRowItem(kernel[2][2], flags);
			
			m->End();
			SetSizerAndFit(m);

			Refresh();
			Update();
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &SharpenPanel::OnButton, this);
			Bind(myFLOATCTRL_CHANGE,&SharpenPanel::OnChanged, this);
			Bind(myFLOATCTRL_UPDATE,&SharpenPanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &SharpenPanel::onEnable, this, SHARPENENABLE);
			Bind(wxEVT_TIMER, &SharpenPanel::OnTimer,  this);
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
			float val = sharpval->GetFloatValue();
			if (val < 0.0) sharpval->SetFloatValue(0.0);
			if (val > 10.0) sharpval->SetFloatValue(10.0);
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{

		}
		
		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%f",sharpval->GetFloatValue()));
			q->processPic();
			event.Skip();
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%f",sharpval->GetFloatValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			int resetval = atoi(myConfig::getConfig().getValueOrDefault("tool.sharpen.initialvalue","0").c_str());
			sharpval->SetFloatValue((float) resetval);
			q->setParams(wxString::Format("%f",sharpval->GetFloatValue()));
			q->processPic();
			event.Skip();
		}
		
		void setKernel(double k[3][3])
		{
			for (int r=0; r<3; r++)
				for (int c=0; c<3; c++)
					kernel[r][c]->SetLabel(wxString::Format("%0.2f",k[r][c]+0.0));
		}


	private:
		myFloatCtrl *sharpval;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer t;
		wxStaticText * kernel[3][3];


};


PicProcessorSharpen::PicProcessorSharpen(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorSharpen::createPanel(wxSimplebook* parent)
{
	toolpanel = new SharpenPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSharpen::processPicture(gImage *processdib) 
{
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
	
	((SharpenPanel *) toolpanel)->setKernel(kernel);

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.sharpen.cores","0").c_str());

	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("sharpen..."));


	dib = processdib;
	if (!global_processing_enabled) return true;

	if (global_processing_enabled & processingenabled & sharp > 0.0) {
		mark();
		dib->ApplyConvolutionKernel(kernel, threadcount);
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.sharpen.log","0") == "1"))
			log(wxString::Format("tool=sharpen,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	}

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");

	return result;
}




