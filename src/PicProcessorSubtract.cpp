#include "PicProcessorSubtract.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "util.h"
#include "gimage/curve.h"

#define SUBTRACTENABLE 8100
#define SUBTRACTVAL 8101
#define SUBTRACTFILE 8102
#define SUBTRACTCAMERA 8103

class SubtractPanel: public PicProcPanel
{

	public:
		SubtractPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, SUBTRACTENABLE, "subtract:");
			enablebox->SetValue(true);

			subb = new wxRadioButton(this, SUBTRACTVAL, "value:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP); 
			fileb = new wxRadioButton(this, SUBTRACTFILE, "file:", wxDefaultPosition, wxDefaultSize);
			camb = new wxRadioButton(this, SUBTRACTCAMERA, "camera:", wxDefaultPosition, wxDefaultSize);

			subtract = new myFloatCtrl(this, wxID_ANY, atof(p.ToStdString().c_str()), 2);
			darkfile = new wxTextCtrl(this, wxID_ANY, "--", wxDefaultPosition, wxSize(150,TEXTCTRLHEIGHT));
			cam = new wxStaticText(this, wxID_ANY, "--");

			std::map<std::string,std::string> p = proc->paramMap(params.ToStdString(), "value,filename");

			if (p.find("value") != p.end())
				if (p["value"] == "camera") {
					camb->SetValue(true);
				}
				else if (p["value"] == "file") {
					fileb->SetValue(true);
				}
				else {
					subb->SetValue(true);
					subtract->SetFloatValue(atof(p["subtract"].c_str()));
				}
			else 
				subtract->SetFloatValue(0.0);

			
			myRowSizer *m = new myRowSizer();
			m->AddRowItem(enablebox, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(subb, flags);
			m->AddRowItem(subtract, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(fileb, flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, wxID_ANY, "Select"), flags);
			m->AddRowItem(darkfile, flags.CenterVertical());
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(camb, flags.CenterVertical());
			m->AddRowItem(cam, flags);
			m->NextRow();

			m->End();
			SetSizerAndFit(m);
			m->Layout();


			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &SubtractPanel::OnTimer, this);
			Bind(myFLOATCTRL_CHANGE, &SubtractPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &SubtractPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &SubtractPanel::onEnable, this, SUBTRACTENABLE);
			Bind(wxEVT_RADIOBUTTON, &SubtractPanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &SubtractPanel::selectDarkFile, this);
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

		void setCameraVal(float val)
		{
			camval = val;
			cam->SetLabel(wxString::Format("%f",val));
			Refresh();
		}

		void selectDarkFile(wxCommandEvent& event)
		{
			wxFileName fname, pname;
			fname.Assign(wxFileSelector("Select dark file"));
			darkfile->SetValue(fname.GetFullName());
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			submode = event.GetId();
			processSUB();
		}

		void processSUB()
		{
			float evval;
			switch (submode) {
				case SUBTRACTVAL:
					q->setParams(wxString::Format("%f",subtract->GetFloatValue()));
					q->processPic();
					break;
				case SUBTRACTFILE:
					q->setParams(wxString::Format("file,%s",darkfile->GetValue()));
					q->processPic();
					break;
				case SUBTRACTCAMERA:
					q->setParams("camera");
					q->processPic();
					break;

			}
		}

		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("rgb,%0.2f", subtract->GetFloatValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void paramUpdated(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("rgb,%0.2f", subtract->GetFloatValue()));
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
		wxRadioButton *subb, *fileb, *camb;
		wxTextCtrl *darkfile;
		myFloatCtrl *subtract;
		//wxString filename;
		wxStaticText *cam;
		wxTimer *t;
		float camval;
		int submode;

};

PicProcessorSubtract::PicProcessorSubtract(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorSubtract::createPanel(wxSimplebook* parent)
{
	toolpanel = new SubtractPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	((SubtractPanel *) toolpanel)->setCameraVal(atof(getPreviousPicProcessor()->getProcessedPic().getInfoValue("LibrawBlack").c_str()) / 65536.0);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSubtract::processPic(bool processnext) 
{
	double subtract;
	wxFileName fname;
	//gImage darkfile;

	((wxFrame*) m_display->GetParent())->SetStatusText("subtract...");

	std::map<std::string,std::string> p = paramMap(c.ToStdString(), "value,filename");

	if (p["value"] == "camera") {
		m_tree->SetItemText(id, "subtract:camera");
		subtract = atof(getPreviousPicProcessor()->getProcessedPic().getInfoValue("LibrawBlack").c_str()) / 65536.0;
	}
	else if (p["value"] == "file") {
		m_tree->SetItemText(id, "subtract:file");
	}
	else {
		m_tree->SetItemText(id, "subtract:val");
		subtract = atof(p["value"].c_str());
	}

	bool result = false;

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

		if (p["value"] == "file") {
			if (wxFileName::FileExists(wxString(p["filename"]))) {
				gImage darkfile = gImage::loadImageFile(p["filename"].c_str(), "");
				if (darkfile.getWidth() == w & darkfile.getHeight() == h) { 
					std::vector<pix> &dark = darkfile.getImageData();
					#pragma omp parallel for num_threads(threadcount)
					for (unsigned x=0; x<w; x++) {
						for (unsigned y=0; y<h; y++) {
							unsigned pos = x + y*w;
							img[pos].r -= dark[pos].r;
							img[pos].g -= dark[pos].g;
							img[pos].b -= dark[pos].b;
						}
					}
					result = true;
				}
				else result = false;
			}
			else result = false;
		}
		else {
			#pragma omp parallel for num_threads(threadcount)
			for (unsigned x=0; x<w; x++) {
				for (unsigned y=0; y<h; y++) {
					unsigned pos = x + y*w;
					img[pos].r -= subtract;
					img[pos].g -= subtract;
					img[pos].b -= subtract;
				}
			}
			result = true;
		}

		wxString d = duration();

		if (result) 
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.subtract.log","0") == "1"))
				log(wxString::Format("tool=subtract,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





