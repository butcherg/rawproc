
#include "PicProcessor.h"
#include "PicProcessorGray.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "undo.xpm"

#include "util.h"

#define GRAYENABLE  6200
#define REDSLIDER   6201
#define GREENSLIDER 6202
#define BLUESLIDER  6203

class GrayPanel: public PicProcPanel
{

	public:
		GrayPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM).Expand();
			wxGridBagSizer *g = new wxGridBagSizer();

			wxArrayString p = split(params,",");

			rd = atof(p[0]);
			gr = atof(p[1]);
			bl = atof(p[2]);

			enablebox = new wxCheckBox(this, GRAYENABLE, "gray:");
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			g->Add(new wxStaticText(this,wxID_ANY, "red: "), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			red = new wxSlider(this, wxID_ANY, rd*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(red , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",rd), wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "green: "), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			green = new wxSlider(this, wxID_ANY, gr*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(green , wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",gr), wxDefaultPosition, wxSize(30, -1));
			g->Add(val2 , wxGBPosition(3,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			g->Add(new wxStaticText(this,wxID_ANY, "blue: "), wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			blue = new wxSlider(this, wxID_ANY, bl*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(blue , wxGBPosition(4,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val3 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",bl), wxDefaultPosition, wxSize(30, -1));
			g->Add(val3 , wxGBPosition(4,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			val4 = new wxStaticText(this,wxID_ANY, wxString::Format("Total: %2.2f", rd+gr+bl), wxDefaultPosition, wxSize(-1,-1));
			g->Add(val4, wxGBPosition(5,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset RGB proportions to defaults");
			g->Add(btn, wxGBPosition(5,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &GrayPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &GrayPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &GrayPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &GrayPanel::onEnable, this, GRAYENABLE);
			Bind(wxEVT_TIMER, &GrayPanel::OnTimer,  this);		}

		~GrayPanel()
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
			rd = red->GetValue()/100.0;
			gr = green->GetValue()/100.0;
			bl = blue->GetValue()/100.0;
			val1->SetLabel(wxString::Format("%2.2f", rd));
			val2->SetLabel(wxString::Format("%2.2f", gr));
			val3->SetLabel(wxString::Format("%2.2f", bl));
			val4->SetLabel(wxString::Format("Total: %2.2f", rd+gr+bl));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			rd = red->GetValue()/100.0;
			gr = green->GetValue()/100.0;
			bl = blue->GetValue()/100.0;
			val1->SetLabel(wxString::Format("%2.2f", rd));
			val2->SetLabel(wxString::Format("%2.2f", gr));
			val3->SetLabel(wxString::Format("%2.2f", bl));
			val4->SetLabel(wxString::Format("Total: %2.2f", rd+gr+bl));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",red->GetValue()/100.0,green->GetValue()/100.0,blue->GetValue()/100.0));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetredval = atof(myConfig::getConfig().getValueOrDefault("tool.gray.r","0.21").c_str());
			red->SetValue(resetredval*100.0);
			val1->SetLabel(wxString::Format("%2.2f", resetredval));

			double resetgreenval = atof(myConfig::getConfig().getValueOrDefault("tool.gray.g","0.72").c_str());
			green->SetValue(resetgreenval*100.0);
			val2->SetLabel(wxString::Format("%2.2f", resetgreenval));

			double resetblueval = atof(myConfig::getConfig().getValueOrDefault("tool.gray.b","0.07").c_str());
			blue->SetValue(resetblueval*100.0);
			val3->SetLabel(wxString::Format("%2.2f", resetblueval));

			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",red->GetValue()/100.0,green->GetValue()/100.0,blue->GetValue()/100.0));
			q->processPic();
			event.Skip();

		}



	private:
		double rd, gr, bl;

		wxSlider *red, *green, *blue;
		wxStaticText *val1, *val2, *val3, *val4;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer *t;

};


PicProcessorGray::PicProcessorGray(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorGray::createPanel(wxSimplebook* parent)
{
	toolpanel = new GrayPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorGray::processPic(bool processnext) 
{
	if (!global_processing_enabled) return true;
	((wxFrame*) m_display->GetParent())->SetStatusText("gray...");
	wxArrayString cp = split(getParams(),",");
	double r = atof(cp[0]);
	double g = atof(cp[1]);
	double b = atof(cp[2]);
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.gray.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) {
		mark();
		dib->ApplyGray(r, g, b, threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.gray.log","0") == "1"))
			log(wxString::Format("tool=gray,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



