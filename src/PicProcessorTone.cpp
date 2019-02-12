#include "PicProcessorTone.h"
#include "PicProcPanel.h"
#include "myRowColumnSizer.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"

#define TONEENABLE 7900
#define TONEID 7901
#define TONEGAMMA 7902
#define TONEREINHARD 7903
#define TONELOG2 7904
#define TONELOGGAM 7905

class TonePanel: public PicProcPanel
{

	public:
		TonePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);


			enablebox = new wxCheckBox(this, TONEENABLE, "tone:");
			enablebox->SetValue(true);

			//All the radio buttons in the same group:
			gamb = new wxRadioButton(this, TONEGAMMA, "Gamma:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			reinb = new wxRadioButton(this, TONEREINHARD, "Reinhard:");
			log2b = new wxRadioButton(this, TONELOG2, "Log2");
			hybloggam = new wxRadioButton(this, TONELOGGAM, "Hybrid Log-Gamma");

			edit = new wxTextCtrl(this, TONEID, p, wxDefaultPosition, wxSize(80,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);

			wxArrayString str;
			str.Add("channel");
			str.Add("luminance");
			reinop = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(80,TEXTCTRLHEIGHT), str);

			wxArrayString p = split(params,",");
			if (p[0] == "gamma") {
				gamb->SetValue(true);
				if (p.GetCount() >=2) 
					edit->SetValue(p[1]);
				else
					edit->SetValue("1.0");
			}
			else {
				edit->SetValue("1.0");
			}
			if (p[0] == "reinhard") {
				reinb->SetValue(true);
				if (p.GetCount() >=2) 
					reinop->SetStringSelection(p[1]);
				else
					reinop->SetSelection(reinop->FindString("channel"));
			}
			else {
				reinop->SetSelection(reinop->FindString("channel"));
			}
			if (p[0] == "log2") {
				log2b->SetValue(true);
			}
			if (p[0] == "loggamma") {
				hybloggam->SetValue(true);
			}

			//Lay out the controls in the panel:
			myRowColumnSizer *m = new myRowColumnSizer(10,3);
			m->AddItem(enablebox, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), wxALIGN_LEFT, 2);
			m->NextRow();
			m->AddItem(gamb, wxALIGN_LEFT);
			m->AddItem(edit, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(reinb, wxALIGN_LEFT);
			m->AddItem(reinop, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(log2b, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(hybloggam, wxALIGN_LEFT);
			SetSizerAndFit(m);


			g->Layout();
			SetFocus();

			Bind(wxEVT_CHECKBOX, &TonePanel::onEnable, this, TONEENABLE);
			Bind(wxEVT_RADIOBUTTON, &TonePanel::OnButton, this);
			Bind(wxEVT_TEXT_ENTER,&TonePanel::paramChanged, this, TONEID);
			Bind(wxEVT_CHOICE, &TonePanel::reinopChanged, this);
			Refresh();
			Update();
		}

		~TonePanel()
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

		void reinopChanged(wxCommandEvent& event)
		{
			if (reinb->GetValue()) {
				q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
				q->processPic();
			}
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			processTone(event.GetId());
			event.Skip();
		}

		void processTone(int src)
		{
			switch (src) {
				case TONEGAMMA:
					q->setParams(wxString::Format("gamma,%0.2f",atof(edit->GetLineText(0))));
					break;
				case TONEREINHARD:
					q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
					break;
				case TONELOG2:
					q->setParams(wxString::Format("log2"));
					break;
				case TONELOGGAM:
					q->setParams(wxString::Format("loggamma"));
					break;

			}
			q->processPic();
			Refresh();
		}



		void paramChanged(wxCommandEvent& event)
		{
			if (gamb->GetValue()) {
				q->setParams(wxString::Format("gamma,%0.2f",atof(edit->GetLineText(0))));
				q->processPic();
			}
			event.Skip();
		}

	private:
		wxTextCtrl *edit;
		wxCheckBox *enablebox;
		wxRadioButton *gamb, *reinb, *log2b, *hybloggam;
		wxChoice *reinop;

};

PicProcessorTone::PicProcessorTone(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorTone::createPanel(wxSimplebook* parent)
{
	toolpanel = new TonePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorTone::processPic(bool processnext) 
{
	wxString d;
	wxArrayString p = split(c,",");
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tone.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());

	if (processingenabled) {
		if (p[0] == "gamma") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: gamma...");
			m_tree->SetItemText(id, "tone:gamma");
			double gamma = atof(p[1].c_str());
			double exponent = 1 / gamma;
			double v = 255.0 * (double)pow((double)255, -exponent);

			mark();
			std::vector<pix>& src = dib->getImageData();
			#pragma omp parallel for num_threads(threadcount)
			for (unsigned i=0; i< src.size(); i++) {
				src[i].r = pow(src[i].r,exponent);
				src[i].g = pow(src[i].g,exponent);
				src[i].b = pow(src[i].b,exponent);				
			}
			d = duration();

		}
		else if (p[0] == "loggamma") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: hybrid log gamma...");
			m_tree->SetItemText(id, "tone:loggamma");

			double a = 0.17883277, b = 0.28466892, c = 0.55991073;
			double rubicon = 1.0/12.0;
			double sqrt3 = sqrt(3);

			mark();
			std::vector<pix>& src = dib->getImageData();
			#pragma omp parallel for num_threads(threadcount)
			for (unsigned i=0; i< src.size(); i++) {
				if (src[i].r > 0.0) if (src[i].r > rubicon) src[i].r = a * log(12*src[i].r - b) + c; else src[i].r = sqrt3 * pow(src[i].r, 0.5); 
				if (src[i].g > 0.0) if (src[i].g > rubicon) src[i].g = a * log(12*src[i].g - b) + c; else src[i].g = sqrt3 * pow(src[i].g, 0.5); 
				if (src[i].b > 0.0) if (src[i].b > rubicon) src[i].b = a * log(12*src[i].b - b) + c; else src[i].b = sqrt3 * pow(src[i].b, 0.5); 
			}
			d = duration();

		}
		else if (p[0] == "log2") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: log2...");
			m_tree->SetItemText(id, "tone:log2");
			GIMAGE_TONEMAP algorithm = LOG2;
			mark();
			dib->ApplyToneMap(algorithm, threadcount);
			d = duration();
		}
		else if (p[0] == "reinhard") {
			((wxFrame*) m_display->GetParent())->SetStatusText("tone: reinhard...");
			m_tree->SetItemText(id, "tone:reinhard");
			GIMAGE_TONEMAP algorithm = REINHARD_CHANNEL;
			if (p[1] == "luminance") algorithm = REINHARD_TONE;

			mark();
			dib->ApplyToneMap(algorithm, threadcount);
			d = duration();
		}

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.tone.log","0") == "1"))
			log(wxString::Format("tool=tone:%s,imagesize=%dx%d,threads=%d,time=%s",p[0],dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





