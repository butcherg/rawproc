#include "PicProcessorWhiteBalance.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
//#include "gimage/strutil.h"
#include "gimage/curve.h"
#include <wx/spinctrl.h>
#include <wx/clipbrd.h>
#include "unchecked.xpm"
#include "checked.xpm"

#define WBMANUAL 6400
#define WBAUTO 6401
#define WBPATCH 6402
#define WBCAMERA 6403
#define WBORIGINAL 6404

#define WBRED 6403
#define WBGREEN 6404
#define WBBLUE 6405



class WhiteBalancePanel: public PicProcPanel
{

	public:
		WhiteBalancePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			double rm, gm, bm;
			ptch.x = -1; ptch.y - 1;

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

			g->Add(new wxStaticText(this,wxID_ANY, "red mult:"), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			rmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, spinsize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
			rmult->SetDigits(digits);
			rmult->SetRange(min,max);
			rmult->SetIncrement(increment);
			rmult->SetValue(rm);
			g->Add(rmult, wxGBPosition(0,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

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

			//Operator radio buttons:
			ob = new wxRadioButton(this, WBORIGINAL, "Original:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			ab = new wxRadioButton(this, WBAUTO, "Auto:");
			pb = new wxRadioButton(this, WBPATCH, "Patch:");
			cb = new wxRadioButton(this, WBCAMERA, "Camera:");
			g->Add(ob, wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(ab, wxGBPosition(5,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(pb, wxGBPosition(6,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(cb, wxGBPosition(7,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			ab->SetValue(false);
			ob->Enable(false);
			pb->Enable(false);
			cb->Enable(false);

			//Operator parameters:
			origwb = new wxStaticText(this, wxID_ANY, "(none)");
			g->Add(origwb, wxGBPosition(4,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			autowb = new wxStaticText(this, wxID_ANY, "(none)");
			g->Add(autowb, wxGBPosition(5,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			patch = new wxStaticText(this, wxID_ANY, "(none)", wxDefaultPosition, spinsize);
			g->Add(patch, wxGBPosition(6,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			camera = new wxStaticText(this, wxID_ANY, "(none)", wxDefaultPosition, spinsize);
			g->Add(camera, wxGBPosition(7,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			if (params == "") {
				std::vector<double> rgbmeans = 	((PicProcessorWhiteBalance *)proc)->getPreviousPicProcessor()->getProcessedPic().CalculateChannelMeans();
				autr = rgbmeans[1] / rgbmeans[0];
				autg = 1.0;
				autb = rgbmeans[1] / rgbmeans[2];
				//c = wxString::Format("%f,%f,%f",autr, autg, autb);
				q->setParams(wxString::Format("%f,%f,%f",autr, autg, autb));
				autowb->SetLabel(wxString::Format("%f,%f,%f",autr, autg, autb));
				ab->Enable(true);
			}
			else {
				wxArrayString p = split(params,",");
				orgr = atof(p[0]);
				orgg = atof(p[1]);
				orgb = atof(p[2]);
				origwb->SetLabel(wxString::Format("%f,%f,%f",orgr, orgg, orgb));
				ob->Enable(true);
			}

			std::vector<double> cam_mults = ((PicProcessorWhiteBalance *)proc)->getCameraMultipliers();
			if (cam_mults.size() >= 2) {
				camr = cam_mults[0];
				camg = cam_mults[1];
				camb = cam_mults[2];
				camera->SetLabel(wxString::Format("%f,%f,%f",camr,camg,camb));
				cb->Enable(true);
			}
		
			SetSizerAndFit(g);
			g->Layout();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_SPINCTRLDOUBLE,&WhiteBalancePanel::paramChanged, this);
			Bind(wxEVT_TIMER, &WhiteBalancePanel::OnTimer, this);
			Bind(wxEVT_RADIOBUTTON, &WhiteBalancePanel::OnButton, this);
			Bind(wxEVT_TEXT_ENTER, &WhiteBalancePanel::paramChanged, this);
			Refresh();
			Update();
		}

		~WhiteBalancePanel()
		{
			t->~wxTimer();
		}

		//used by PicProcessorWhiteBalance to initialize panel:
		void setMultipliers(double rm, double gm, double bm)
		{
			rmult->SetValue(rm);
			gmult->SetValue(gm);
			bmult->SetValue(bm);
			//patch->SetLabel(wxString::Format("%f,%f,%f",rm, gm, bm));
			Refresh();
		}
		
		void processPatch()
		{
			//parm tool.whitebalance.patchradius: (float), radius of patch.  Default=1.5
			double radius = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.patchradius","1.5").c_str());

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
				//patch->SetLabel(wxString::Format("%f,%f,%f",rm, gm, bm));
				q->setParams(wxString::Format("%f,%f,%f",rm, gm, bm));
				q->processPic();
				Refresh();
			}
		}

		void processWB(int src)
		{
			switch (src) {
				case WBAUTO:
					q->setParams(wxString::Format("%f,%f,%f",autr, autg, autb));
					break;
				case WBPATCH:
					q->setParams(wxString::Format("%f,%f,%f",patr, patg, patb));
					break;
				case WBCAMERA:
					q->setParams(wxString::Format("%f,%f,%f",camr, camg, camb));
					break;
			}
			q->processPic();
			Refresh();
		}

		void setAutoWB(double rm, double gm, double bm) 
		{
			autr = rm;
			autg = gm;
			autb = bm;
			autowb->SetLabel(wxString::Format("%f,%f,%f",rm,gm,bm));
			//if (ab->GetValue() == true)
			//	processWB(WBAUTO);
			//else
				Refresh();
		}
		
		void setPatch(coord p)
		{
			ptch = p;
			//parm tool.whitebalance.patchradius: (float), radius of patch.  Default=1.5
			double radius = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.patchradius","1.5").c_str());

			unsigned x, y;
			patr=1.0, patg=1.0, patb=1.0;
			double ra, ga, ba;

			std::vector<double> a = ((PicProcessorWhiteBalance *)q)->getPatchMeans(ptch.x, ptch.y, radius);
			if (a.size() < 3) return;
			ra = a[0];
			ga = a[1];
			ba = a[2];
			patr = ga / ra;
			patb = ga / ba;

			patch->SetLabel(wxString::Format("%f,%f,%f",patr, patg, patb));

			pb->Enable(true);
			if (pb->GetValue() == true)
				processWB(WBPATCH);
			else
				Refresh();
		}

		void setCamera(double rm, double gm, double bm) 
		{
			camr = rm; camg = gm; camb = bm;
			camera->SetLabel(wxString::Format("%f,%f,%f",rm,gm,bm));
			cb->Enable(true);
			Refresh();
		}
		
		void paramChanged(wxCommandEvent& event)
		{
			ab->SetValue(false);
			pb->SetValue(false);
			cb->SetValue(false);
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void fooChanged(wxCommandEvent& event)
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
			processWB(event.GetId());
			event.Skip();
		}

	private:
		wxStaticText *origwb, *autowb, *patch, *camera;
		wxRadioButton *ob, *ab, *pb, *cb;
		wxSpinCtrlDouble *rmult, *gmult ,*bmult;
		wxTimer *t;
		coord ptch;
		double orgr, orgg, orgb; //original multipliers, none if new tool
		double camr, camg, camb; //camera 'as-shot' multipliers
		double patr, patg, patb; //patch multipliers
		double autr, autg, autb; //auto multipliers 

};


PicProcessorWhiteBalance::PicProcessorWhiteBalance(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	dib = NULL;
	double redmult=1.0, greenmult=1.0, bluemult=1.0;
	if (command == "") {
		std::vector<double> rgbmeans = 	getPreviousPicProcessor()->getProcessedPic().CalculateChannelMeans();
		redmult = rgbmeans[1] / rgbmeans[0];
		bluemult = rgbmeans[1] / rgbmeans[2];
		c = wxString::Format("%f,%f,%f",redmult, greenmult, bluemult);
		//((WhiteBalancePanel *) toolpanel)->setAutoWB(redmult, greenmult, bluemult);
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
	m_display->Refresh();
	m_display->Update();
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

std::vector<double> PicProcessorWhiteBalance::getCameraMultipliers()
{
	std::vector<double> multipliers;
	std::string cameraWBstring = getPreviousPicProcessor()->getProcessedPic().getInfoValue("LibrawWhiteBalance");
	if (cameraWBstring != "") {
		wxArrayString multstrings = split(wxString(cameraWBstring.c_str()), ",");
		for (int i = 0; i < multstrings.GetCount(); i++) {
			multipliers.push_back(atof(multstrings[i].c_str()));
		}
	}
	return multipliers;
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





