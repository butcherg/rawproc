#include "PicProcessor.h"
#include "PicProcessorToneMask.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "undo.xpm"
#include "util.h"
#include "gimage/strutil.h"

#include <vector>

#define TONEMASKENABLE 8000

class ToneMaskPanel: public PicProcPanel
{
	public:
		ToneMaskPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxString thr= "128", msk="lower";

			std::map<std::string,std::string> p = parseparams(std::string(params.mb_str()));

			enablebox = new wxCheckBox(this, TONEMASKENABLE, "tone mask:");
			enablebox->SetValue(true);
			b->Add(enablebox, flags);
			b->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			b->AddSpacer(10);

			imagefile = new wxTextCtrl(this, wxID_ANY, wxString(p["image"]), wxDefaultPosition, wxSize(200,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			b->Add(imagefile, flags);
			b->Add(new wxButton(this, wxID_ANY, "Select mask image"), flags);
			b->AddSpacer(10);


			wxArrayString m;
			m.Add("lower");
			m.Add("upper");

			if (p.find("threshold") != p.end())
				thr = wxString(p["threshold"]);

			if (p.find("mask") != p.end()) 
				msk = wxString(p["mask"]);

			threshold = new wxSlider(this, wxID_ANY, atoi(std::string(thr.mb_str()).c_str()), 0, 255, wxPoint(10, 30), wxSize(-1, -1));
			b->Add(threshold , flags.Expand());
			val = new wxStaticText(this,wxID_ANY, thr, wxDefaultPosition, wxSize(30, -1));
			b->Add(val , wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP));
			mask = new wxRadioBox(this, wxID_ANY, "mask:", wxDefaultPosition, wxDefaultSize, m);
			b->Add(mask, flags);

			if (msk == "upper") mask->SetSelection(1);
			if (msk == "lower") mask->SetSelection(0);
			

			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &ToneMaskPanel::SelectMaskImage, this);
			Bind(wxEVT_SCROLL_CHANGED, &ToneMaskPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ToneMaskPanel::OnThumbTrack, this);
			Bind(wxEVT_RADIOBOX, &ToneMaskPanel::OnChanged, this);
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
			val->SetLabel(wxString::Format("%4d", threshold->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%4d", threshold->GetValue()));
		}

		void SelectMaskImage(wxCommandEvent& event)
		{
			wxFileName fname;
			fname.Assign(wxFileSelector("Select mask image"));
			if (fname.FileExists()) {
				imagefile->SetValue(fname.GetFullName());
				q->setParams(wxString::Format("image=%s;threshold=%d;mask=%s",imagefile->GetValue(),threshold->GetValue(),mask->GetString(mask->GetSelection())));
				q->processPic();
			}
		}

		void OnTimer(wxTimerEvent& event)
		{
			if (imagefile->IsEmpty())
				q->setParams(wxString::Format("threshold=%d;mask=%s",threshold->GetValue(),mask->GetString(mask->GetSelection())));
			else
				q->setParams(wxString::Format("image=%s;threshold=%d;mask=%s",imagefile->GetValue(),threshold->GetValue(),mask->GetString(mask->GetSelection())));
			q->processPic();
			event.Skip();
		}

	private:
		wxSlider *threshold;
		wxStaticText *val;
		wxTextCtrl *imagefile;
		wxRadioBox * mask;
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

	bool result = true;
	std::string imagefile;
	bool lowermask=true; 
	int threshold=128;

	std::map<std::string,std::string> p = parseparams(std::string(getParams().mb_str()));
	if (p.find("image") != p.end()) {
		imagefile = p["image"];
		if (!file_exists(imagefile)) return false;
	}
	else return false;
	if (p.find("threshold") != p.end()) threshold = atoi(p["threshold"].c_str());
	if (p.find("mask") != p.end())
		if (p["mask"] == "upper") 
			lowermask=false;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tonemask.cores","0").c_str());

	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("tone mask..."));


	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) {
		mark();

		gImage maskimage = gImage::loadImageFile(imagefile.c_str(),"");
		std::vector<pix> &mask = maskimage.getImageData();
		std::vector<pix> &img  = dib->getImageData();
		int masksize = mask.size();
		if (masksize != img.size()) return false;

		#pragma omp parallel for num_threads(threadcount)
		for (int i=0; i < masksize; i++) {
			int imgtone = ((img[i].r + img[i].g + img[i].b) / 3.0) * 255;
			if (lowermask) 
				if (imgtone < threshold) img[i] = mask[i];
			else
				if (imgtone > threshold) img[i] = mask[i];
		}

		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.tonemask.log","0") == "1"))
			log(wxString::Format("tool=ToneMask,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	}

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}




