#include "PicProcessor.h"
#include "PicProcessorCACorrect.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "myFloatCtrl.h"
#include "myIntegerCtrl.h"
#include "undo.xpm"
#include "copy.xpm"
#include "paste.xpm"
#include "util.h"

#include <vector>

#define CACORRECTENENABLE 8300
#define CACORRECTAUTOCA 8301
#define CACORRECTAVOIDCS  8302
#define CACORRECTCOPY 8303
#define CACORRECTPASTE 8304

class CACorrectPanel: public PicProcPanel
{
	public:
		CACorrectPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());

			enablebox = new wxCheckBox(this, CACORRECTENENABLE, _("cacorrect:"));
			enablebox->SetValue(true);

			red = new myFloatCtrl(this, wxID_ANY, _("red:"), 0.0, 3);
			blue  = new myFloatCtrl(this, wxID_ANY, _("blue:"), 0.0, 3);
			autoca  = new wxCheckBox(this, CACORRECTAUTOCA, _("auto-correct"));
			iterations = new myIntegerCtrl(this, wxID_ANY, _("iterations:"), 1, 1, 100);
			avoidcs = new wxCheckBox(this, CACORRECTAVOIDCS, _("avoid color-shift"));

			wxArrayString p = split(params,",");
			if (p.size() > 0 && p[0] == "auto") {
				red->Enable(false);
				blue->Enable(false);
				iterations->Enable(true);
				autoca->SetValue(true);
				if (p.size() >= 2) iterations->SetIntegerValue(atoi(p[1].c_str()));
 
			}
			else {
				red->Enable(true);
				blue->Enable(true);
				iterations->Enable(false);
				if (p.size() > 0) red->SetFloatValue(atof(p[0].c_str()));
				if (p.size() >= 2) blue->SetFloatValue(atof(p[1].c_str()));
				autoca->SetValue(false);
			}
			if (params.Find("avoidcs") != wxNOT_FOUND) avoidcs->SetValue(true);

			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, CACORRECTCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, CACORRECTPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(red,flags);
			m->NextRow();
			m->AddRowItem(blue,flags);
			m->NextRow();
			m->AddRowItem(autoca,flags);
			m->AddRowItem(iterations,flags);
			m->NextRow();
			m->AddRowItem(avoidcs,flags);
			m->End();

			SetSizerAndFit(m);
			Refresh();
			t.SetOwner(this);

			Bind(myFLOATCTRL_CHANGE, &CACorrectPanel::OnChanged, this);
			Bind(myFLOATCTRL_UPDATE, &CACorrectPanel::OnThumbTrack, this);
			Bind(myINTEGERCTRL_CHANGE, &CACorrectPanel::OnChanged, this);
			Bind(myINTEGERCTRL_UPDATE, &CACorrectPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &CACorrectPanel::onEnable, this, CACORRECTENENABLE);
			Bind(wxEVT_BUTTON, &CACorrectPanel::OnCopy, this, CACORRECTCOPY);
			Bind(wxEVT_BUTTON, &CACorrectPanel::OnPaste, this, CACORRECTPASTE);
			Bind(wxEVT_CHECKBOX, &CACorrectPanel::OnAutoCA, this, CACORRECTAUTOCA);
			Bind(wxEVT_CHECKBOX, &CACorrectPanel::OnAvoidCS, this, CACORRECTAVOIDCS);
			Bind(wxEVT_TIMER, &CACorrectPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &CACorrectPanel::OnKey,  this);
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

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied command to clipboard: %s"),q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				wxString parms = q->getParams();
				wxArrayString p = split(parms,",");
				if (p[0] == "auto") {
					red->Enable(false);
					blue->Enable(false);
					autoca->SetValue(true);
					if (p.size() >= 2) iterations->SetIntegerValue(atoi(p[1].c_str()));
				}
				else {
					red->Enable(true);
					blue->Enable(true);
					red->SetFloatValue(atof(p[0].c_str()));
					if (p.size() >= 2) blue->SetFloatValue(atof(p[1].c_str()));
					autoca->SetValue(false);
				}
				if (parms.Find("avoidcs") == wxNOT_FOUND) 
					avoidcs->SetValue(false);
				else 
					avoidcs->SetValue(true);

				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
			}
			else wxMessageBox(wxString::Format(_("Invalid Paste")));
		}


		void OnChanged(wxCommandEvent& event)
		{
			if (red->GetFloatValue() < 0.0) red->SetFloatValue(0.0);
			if (blue->GetFloatValue() < 0.0) blue->SetFloatValue(0.0);
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void processCA()
		{
			wxString avoidcolorshift;
			if (avoidcs->GetValue()) avoidcolorshift = ",avoidcs";
			if (autoca->GetValue())
				q->setParams(wxString::Format("auto,%d%s",iterations->GetIntegerValue(), avoidcolorshift));
			else
				q->setParams(wxString::Format("%f,%f%s",red->GetFloatValue(), blue->GetFloatValue(), avoidcolorshift));
			q->processPic();
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			processCA();
			event.Skip();
		}

		void OnTimer(wxTimerEvent& event)
		{
			processCA();
		}

		void OnAutoCA(wxCommandEvent& event) 
		{
			if (autoca->GetValue()) {
				red->Enable(false);
				blue->Enable(false);
				iterations->Enable(true);
			}
			else {
				red->Enable(true);
				blue->Enable(true);
				iterations->Enable(false);
			}
			processCA();
		}

		void OnAvoidCS(wxCommandEvent& event)
		{
			processCA();
		}

	private:
		myFloatCtrl *blue, *red;
		myIntegerCtrl *iterations;
		wxCheckBox *enablebox, *autoca, *avoidcs;
		wxTimer t;

//bool gImage::ApplyCACorrect(const bool autoCA, size_t autoIterations, const double cared, const double cablue, bool avoidColourshift, int threadcount)
};


PicProcessorCACorrect::PicProcessorCACorrect(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorCACorrect::createPanel(wxSimplebook* parent)
{
	toolpanel = new CACorrectPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorCACorrect::processPicture(gImage *processdib) 
{
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.cacorrect.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format(_("cacorrect...")));

	dib = processdib;
	if (!global_processing_enabled) return true;

	double cared = 0.0, cablue =  0.0;
	bool autoca = true, avoidcs = false;
	int iterations = 1;
	wxArrayString p = split(getParams(),",");

	if (p[0] == "auto") {
		if (p.size() >= 2 && p[1] != "avoidcs") iterations = atoi(p[1].c_str());
	}
	else {
		cared = atof(p[0].c_str());
		if (p.size() >= 2) cablue = atof(p[1].c_str());
	}
	if (getParams().Find("avoidcs") != wxNOT_FOUND) avoidcs = true;

	if (global_processing_enabled & processingenabled) {
		mark();
#ifdef USE_LIBRTPROCESS
		dib->ApplyCACorrect(autoca, iterations, cared, cablue, avoidcs, threadcount);
#endif
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.cacorrect.log","0") == "1"))
			log(wxString::Format(_("tool=cacorrect,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));

	}

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");

	return result;
}




