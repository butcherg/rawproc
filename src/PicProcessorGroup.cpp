#include "PicProcessorGroup.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"

#define GROUPENABLE 8200

class GroupPanel: public PicProcPanel
{

	public:
		GroupPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, GROUPENABLE, "gamma:");
			enablebox->SetValue(true);
			b->Add(enablebox, flags);
			b->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			b->AddSpacer(10);

			SetSizerAndFit(b);
			b->Layout();
			SetFocus();

			Bind(wxEVT_CHECKBOX, &GroupPanel::onEnable, this, GROUPENABLE);
			Refresh();
			Update();
		}

		~GroupPanel()
		{

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


	private:
		wxCheckBox *enablebox;

};

PicProcessorGroup::PicProcessorGroup(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorGroup::createPanel(wxSimplebook* parent)
{
	toolpanel = new GroupPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorGroup::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("gamma...");

	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.group.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());

	if (processingenabled) {
		mark();

		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.group.log","0") == "1"))
			log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





