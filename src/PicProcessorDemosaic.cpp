#include "PicProcessorDemosaic.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"


class DemosaicPanel: public PicProcPanel
{

	public:
		DemosaicPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			b->Add(new wxStaticText(this,-1, "demosaic", wxDefaultPosition, wxSize(100,20)), flags);
			////edit = new wxTextCtrl(this, GAMMAID, p, wxDefaultPosition, wxSize(100,20),wxTE_PROCESS_ENTER);
			//b->Add(edit, flags);
			SetSizerAndFit(b);
			b->Layout();
			SetFocus();
			//Bind(wxEVT_TEXT_ENTER,&GammaPanel::paramChanged, this, GAMMAID);
			Refresh();
			Update();
		}

		~DemosaicPanel()
		{
			
		}

		void paramChanged(wxCommandEvent& event)
		{
			//q->setParams(edit->GetLineText(0));
			q->processPic();
			event.Skip();
		}

	private:
		//wxTextCtrl *edit;

};

PicProcessorDemosaic::PicProcessorDemosaic(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorDemosaic::createPanel(wxSimplebook* parent)
{
	toolpanel = new DemosaicPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorDemosaic::processPic(bool processnext) 
{
	Curve ctrlpts;
	((wxFrame*) m_display->GetParent())->SetStatusText("demosaic...");
	//double gamma = atof(c.c_str());
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.demosaic.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyDemosaic(DEMOSAIC_HALF, threadcount);
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.demosaic.log","0") == "1"))
		log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





