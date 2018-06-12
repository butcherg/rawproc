#include "PicProcessorWhiteBalance.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"
#include <wx/spinctrl.h>

#define GAMMAID 8500

class WhiteBalancePanel: public PicProcPanel
{

	public:
		WhiteBalancePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			b->Add(new wxStaticText(this,-1, "white balance", wxDefaultPosition, wxSize(100,20)), flags);
			
			edit = new wxTextCtrl(this, wxID_ANY, p, wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			b->Add(edit, flags);
			
			SetSizerAndFit(b);
			b->Layout();
			SetFocus();
			Bind(wxEVT_TEXT_ENTER,&WhiteBalancePanel::paramChanged, this);
			Refresh();
			Update();
		}

		~WhiteBalancePanel()
		{
			
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(edit->GetLineText(0));
			q->processPic();
			event.Skip();
		}

	private:
		wxTextCtrl *edit;
		//wxSpinCtrlDouble *r,*g,*b;

};

PicProcessorWhiteBalance::PicProcessorWhiteBalance(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorWhiteBalance::createPanel(wxSimplebook* parent)
{
	toolpanel = new WhiteBalancePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorWhiteBalance::processPic(bool processnext) 
{
	double redmult=1.0, greenmult=1.0, bluemult=1.0;
	((wxFrame*) m_display->GetParent())->SetStatusText("white balance...");


	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.whitebalance.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (c == "") {
		std::vector<double> rgbmeans = dib->CalculateChannelMeans();
		redmult = rgbmeans[0] / rgbmeans[1];
		bluemult = rgbmeans[2] / rgbmeans[1];
	}
	dib->ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.gamma.log","0") == "1"))
		log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





