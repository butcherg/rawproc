#include "PicProcessorWhiteBalance.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"
#include <wx/spinctrl.h>
#include <wx/clipbrd.h>

#define WBAUTO 6400
#define WBPATCH 6401

#define WBRED 6402
#define WBGREEN 6403
#define WBBLUE 6404



class WhiteBalancePanel: public PicProcPanel
{

	public:
		WhiteBalancePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			double rm, gm, bm;

			wxSize spinsize(130, -1);

			//parm tool.whitebalance.min: (float), minimum multiplier value.  Default=0.001
			double min = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.min","0.001").c_str());
			//parm tool.whitebalance.max: (float), maximum multiplier value.  Default=3.0
			double max = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.max","3.0").c_str());
			//parm tool.whitebalance.digits: (float), number of fractional digits.  Default=3
			double digits = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.digits","3.0").c_str());
			//parm tool.whitebalance.increment: (float), maximum multiplier value.  Default=0.001
			double increment = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.increment","0.001").c_str());

			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			wxArrayString p = split(params,",");
			rm = atof(p[0]);
			gm = atof(p[1]);
			bm = atof(p[2]);

			g->Add(new wxStaticText(this,wxID_ANY, "red mult:"), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			rmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, spinsize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
			rmult->SetDigits(digits);
			rmult->SetRange(min,max);
			rmult->SetIncrement(increment);
			rmult->SetValue(rm);
			g->Add(rmult, wxGBPosition(0,1), wxGBSpan(1,2), wxEXPAND | wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "green mult:"), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			gmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, spinsize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
			gmult->SetDigits(digits);
			gmult->SetRange(min,max);
			gmult->SetIncrement(increment);
			gmult->SetValue(gm);
			g->Add(gmult, wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "blue mult:"), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			bmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, spinsize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
			bmult->SetDigits(digits);
			bmult->SetRange(min,max);
			bmult->SetIncrement(increment);
			bmult->SetValue(bm);
			g->Add(bmult, wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(0,10, wxGBPosition(3,0));

			g->Add(new wxButton(this, WBAUTO, "Auto"), wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxButton(this, WBPATCH, "Patch"), wxGBPosition(5,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			patch = new wxStaticText(this, wxID_ANY, "(none)", wxDefaultPosition, spinsize);
			g->Add(patch, wxGBPosition(5,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			
			SetSizerAndFit(g);
			g->Layout();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_SPINCTRLDOUBLE,&WhiteBalancePanel::paramChanged, this);
			Bind(wxEVT_TIMER, &WhiteBalancePanel::OnTimer, this);
			Bind(wxEVT_BUTTON, &WhiteBalancePanel::OnButton, this);
			Bind(wxEVT_TEXT_ENTER, &WhiteBalancePanel::paramChanged, this);
			Refresh();
			Update();
		}

		~WhiteBalancePanel()
		{
			t->~wxTimer();
		}

		void setMultipliers(double rm, double gm, double bm)
		{
			rmult->SetValue(rm);
			gmult->SetValue(gm);
			bmult->SetValue(bm);
			Refresh();
		}
		
		void setPatch(coord p)
		{
			ptch = p;
			//patch->SetLabel(wxString::Format("x:%d y:%d r:%0.1f",p.x, p.y, radius));
			patch->SetLabel(wxString::Format("x:%d y:%d",p.x, p.y));
			Refresh();
		}

		void paramChanged(wxCommandEvent& event)
		{
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%f,%f,%f",rmult->GetValue(), gmult->GetValue(), bmult->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			switch (event.GetId()) {
				case WBAUTO:
					q->setParams("");
					q->processPic();
					break;
				case WBPATCH:
					//parm tool.whitebalance.patchradius: (float), radius of patch.  Default=1.5
					double radius = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.patchradius","1.5").c_str());

//					wxTextDataObject cb;
					unsigned x, y;
					double rm=1.0, gm=1.0, bm=1.0;
					double ra, ga, ba;

					if (ptch.x != -1 & ptch.y !=-1) {
						patch->SetLabel(wxString::Format("x:%d y:%d r:%0.1f",ptch.x, ptch.y, radius));
						std::vector<double> a = ((PicProcessorWhiteBalance *)q)->getPatchMeans(ptch.x, ptch.y, radius);
						ra = a[0];
						ga = a[1];
						ba = a[2];
						rm = ga / ra;
						bm = ga / ba;
						rmult->SetValue(rm);
						gmult->SetValue(gm);
						bmult->SetValue(bm);
						q->setParams(wxString::Format("%f,%f,%f",rm, gm, bm));
						q->processPic();
						Refresh();
					}
					break;
			}
			event.Skip();
		}

	private:
		wxStaticText *patch;
		wxSpinCtrlDouble *rmult, *gmult ,*bmult;
		wxTimer *t;
		coord ptch;

};


PicProcessorWhiteBalance::PicProcessorWhiteBalance(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	
	double redmult=1.0, greenmult=1.0, bluemult=1.0;
	if (command == "") {
		std::vector<double> rgbmeans = 	getPreviousPicProcessor()->getProcessedPic().CalculateChannelMeans();
		redmult = rgbmeans[1] / rgbmeans[0];
		bluemult = rgbmeans[1] / rgbmeans[2];
		c = wxString::Format("%f,%f,%f",redmult, greenmult, bluemult);
	}
	patch.x=0;
	patch.y=0;
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorWhiteBalance::OnLeftDown, this);
}

PicProcessorWhiteBalance::~PicProcessorWhiteBalance()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorWhiteBalance::OnLeftDown, this);
	m_display->SetDrawList("");
}

void PicProcessorWhiteBalance::OnLeftDown(wxMouseEvent& event)
{
	if (m_tree->GetItemState(GetId()) != 1) {
		event.Skip();
		return;
	}
	if (event.ShiftDown()) {
		patch = m_display->GetImgCoords();
	}
	else {
		event.Skip();
		return;
	}
	dcList = wxString::Format("cross,%d,%d;",patch.x, patch.y);
	m_display->SetDrawList(dcList);
	//processPic();
	m_display->Refresh();
	((WhiteBalancePanel *) toolpanel)->setPatch(patch);
	event.Skip();
}

void PicProcessorWhiteBalance::createPanel(wxSimplebook* parent)
{
	toolpanel = new WhiteBalancePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

std::vector<double> PicProcessorWhiteBalance::getPatchMeans(int x, int y, float radius)
{
	std::vector<double> means;
	double rsum=0.0, gsum=0.0, bsum=0.0;
	gImage prev = getPreviousPicProcessor()->getProcessedPic();
	int count = 0;
	for (int i=x-radius; i<x+radius; i++) {
		for (int j=y-radius; j<y+radius; j++) {
			pix p = prev.getRGB(i,j);
			rsum += p.r;
			gsum += p.g;
			bsum += p.b;
			count++;
		}
	}
	means.push_back(rsum/(double)count);
	means.push_back(gsum/(double)count);
	means.push_back(bsum/(double)count);
	return means;
}

bool PicProcessorWhiteBalance::processPic(bool processnext) 
{
	double redmult=1.0, greenmult=1.0, bluemult=1.0;
	((wxFrame*) m_display->GetParent())->SetStatusText("white balance...");


	if (c == "") {
		std::vector<double> rgbmeans = 	getPreviousPicProcessor()->getProcessedPic().CalculateChannelMeans();
		redmult = rgbmeans[1] / rgbmeans[0];
		bluemult = rgbmeans[1] / rgbmeans[2];
		setParams(wxString::Format("%f,%f,%f",redmult, greenmult, bluemult));
		((WhiteBalancePanel *) toolpanel)->setMultipliers(redmult, greenmult, bluemult);
	}
	else {
		wxArrayString p = split(c, ",");
		redmult = atof(p[0]);
		greenmult = atof(p[1]);
		bluemult = atof(p[2]);
	}

	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.whitebalance.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.gamma.log","0") == "1"))
		log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





