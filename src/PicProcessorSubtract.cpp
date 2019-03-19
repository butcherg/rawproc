#include "PicProcessorSubtract.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "util.h"
#include "gimage/curve.h"

#define SUBTRACTENABLE 8100
#define SUBTRACTID 8101

class SubtractPanel: public PicProcPanel
{

	public:
		SubtractPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, SUBTRACTENABLE, "subtract:");
			enablebox->SetValue(true);
			subtract = new myFloatCtrl(this, wxID_ANY, atof(p.ToStdString().c_str()), 2);

			std::map<std::string,std::string> p = proc->paramMap(params.ToStdString(), "subtract");

			if (p.find("subtract") != p.end())
				subtract->SetFloatValue(atof(p["subtract"].c_str()));
			else 
				subtract->SetFloatValue(0.0);

			myRowSizer *m = new myRowSizer();
			m->AddRowItem(enablebox, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(subtract, flags);
			m->End();
			SetSizerAndFit(m);
			m->Layout();


			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &SubtractPanel::OnTimer, this);
			Bind(myFLOATCTRL_CHANGE, &SubtractPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &SubtractPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &SubtractPanel::onEnable, this, SUBTRACTENABLE);
			Refresh();
			Update();
		}

		~SubtractPanel()
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

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%0.2f", subtract->GetFloatValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void paramUpdated(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%0.2f", subtract->GetFloatValue()));
			q->processPic();
			Refresh();
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			q->processPic();
			Refresh();
		}

	private:
		wxCheckBox *enablebox;
		myFloatCtrl *subtract;
		wxTimer *t;

};

PicProcessorSubtract::PicProcessorSubtract(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorSubtract::createPanel(wxSimplebook* parent)
{
	toolpanel = new SubtractPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSubtract::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("subtract...");
	double subtract = atof(c.c_str());
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());

	if (processingenabled) {
		mark();

		std::vector<pix> &img = dib->getImageData();
		unsigned w = dib->getWidth(); unsigned h = dib->getHeight();
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				img[pos].r -= subtract;
				img[pos].g -= subtract;
				img[pos].b -= subtract;
			}
		}

		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.subtract.log","0") == "1"))
			log(wxString::Format("tool=subtract,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





