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

			g->Add(new wxStaticText(this,wxID_ANY, "white balance: "), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(1,10, wxGBPosition(0,0));

			g->Add(new wxStaticText(this,wxID_ANY, "red mult:"), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			rmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, spinsize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
			rmult->SetDigits(digits);
			rmult->SetRange(min,max);
			rmult->SetIncrement(increment);
			rmult->SetValue(rm);
			g->Add(rmult, wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "green mult:"), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			gmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, spinsize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
			gmult->SetDigits(digits);
			gmult->SetRange(min,max);
			gmult->SetIncrement(increment);
			gmult->SetValue(gm);
			g->Add(gmult, wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "blue mult:"), wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			bmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, spinsize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
			bmult->SetDigits(digits);
			bmult->SetRange(min,max);
			bmult->SetIncrement(increment);
			bmult->SetValue(bm);
			g->Add(bmult, wxGBPosition(4,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(0,10, wxGBPosition(5,0));

			//Operator radio buttons:
			ob = new wxRadioButton(this, WBORIGINAL, "Original:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			ab = new wxRadioButton(this, WBAUTO, "Auto");
			pb = new wxRadioButton(this, WBPATCH, "Patch:");
			cb = new wxRadioButton(this, WBCAMERA, "Camera:");
			g->Add(ob, wxGBPosition(6,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(ab, wxGBPosition(7,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(pb, wxGBPosition(8,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(cb, wxGBPosition(9,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			ab->SetValue(false);
			ob->Enable(false);
			pb->Enable(false);
			cb->Enable(false);

			//Operator parameters:
			origwb = new wxStaticText(this, wxID_ANY, "(none)");
			g->Add(origwb, wxGBPosition(6,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			autowb = new wxStaticText(this, wxID_ANY, "");
			g->Add(autowb, wxGBPosition(7,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			patch = new wxStaticText(this, wxID_ANY, "(none)", wxDefaultPosition, spinsize);
			g->Add(patch, wxGBPosition(8,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			camera = new wxStaticText(this, wxID_ANY, "(none)", wxDefaultPosition, spinsize);
			g->Add(camera, wxGBPosition(9,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			//parse parameters:
			wxArrayString p = split(params,",");

			if (params == "") {
				orgr = 1.0;
				orgg = 1.0;
				orgb = 1.0;
				origwb->SetLabel(wxString::Format("%0.3f,%0.3f,%0.3f",orgr, orgg, orgb));
				setMultipliers(orgr, orgg, orgb);
				ob->Enable(true);
				ob->SetValue(true);
			}
			else if (p[0] == "auto") {
				ab->SetValue(true);
			}
			else if (p[0] == "camera") {

			}
			else if (p[0].Find(".") != wxNOT_FOUND) { //float multipliers
				orgr = atof(p[0]);
				orgg = atof(p[1]);
				orgb = atof(p[2]);
				origwb->SetLabel(wxString::Format("%0.3f,%0.3f,%0.3f",orgr, orgg, orgb));
				setMultipliers(orgr, orgg, orgb);
				ob->Enable(true);
				ob->SetValue(true);
				
			}
			else if (p[0].IsNumber()) { //int patch coordinates, float radius
				patx = atoi(p[0]);
				paty = atoi(p[1]);
				patrad = atof(p[2]);
				patch->SetLabel(wxString::Format("x:%d y:%d r:%0.1f",patx, paty, patrad));
				cb->SetValue(true);
			}
			else 
				wxMessageBox("Error: ill-formed param string");


			camr = 1.0; camg = 1.0; camb = 1.0;
			std::vector<double> cam_mults = ((PicProcessorWhiteBalance *)proc)->getCameraMultipliers();
			if (cam_mults.size() >= 2) {
				camr = cam_mults[0];
				camg = cam_mults[1];
				camb = cam_mults[2];
				camera->SetLabel(wxString::Format("%0.3f,%0.3f,%0.3f",camr,camg,camb));
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
			Refresh();
		}

		void setMultipliers(std::vector<double> mults)
		{
			rmult->SetValue(mults[0]);
			gmult->SetValue(mults[1]);
			bmult->SetValue(mults[2]);
			Refresh();
		}



		void processWB(int src)
		{
			switch (src) {
				case WBAUTO:
					q->setParams("auto");
					break;
				case WBPATCH:
					q->setParams(wxString::Format("patch,%d,%d,%0.1f",patx, paty, patrad));
					break;
				case WBORIGINAL:
					q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",orgr, orgg, orgb));
					setMultipliers(orgr, orgg, orgb);
					break;
				case WBCAMERA:
					q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",camr, camg, camb));
					setMultipliers(camr, camg, camb);
					break;
			}
			q->processPic();
			Refresh();
		}
	
		void setPatch(coord p)
		{
			//parm tool.whitebalance.patchradius: (float), radius of patch.  Default=1.5
			patrad = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.patchradius","1.5").c_str());

			patx = p.x;
			paty = p.y;
			patch->SetLabel(wxString::Format("patch,%d,%d,%0.1f",patx, paty, patrad));

			pb->Enable(true);
			if (pb->GetValue() == true)
				processWB(WBPATCH);
			else
				Refresh();
		}

		void setCamera(double rm, double gm, double bm) 
		{
			camr = rm; camg = gm; camb = bm;
			camera->SetLabel(wxString::Format("%0.3f,%0.3f,%0.3f",rm,gm,bm));
			cb->Enable(true);
			Refresh();
		}
		
		void paramChanged(wxCommandEvent& event)
		{
			ob->SetValue(false);
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
			q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",rmult->GetValue
(), gmult->GetValue(), bmult->GetValue()));
			q->processPic();
			Refresh();
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
		//coord ptch;
		unsigned patx, paty;
		double patrad;
		double orgr, orgg, orgb; //original multipliers, none if new tool
		double camr, camg, camb; //camera 'as-shot' multipliers
		//double autr, autg, autb; //auto multipliers 

};


PicProcessorWhiteBalance::PicProcessorWhiteBalance(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	dib = NULL;
	double redmult=1.0, greenmult=1.0, bluemult=1.0;

	patch.x=0;
	patch.y=0;
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorWhiteBalance::OnLeftDown, this);
}

PicProcessorWhiteBalance::~PicProcessorWhiteBalance()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorWhiteBalance::OnLeftDown, this);
	m_display->SetDrawList("");
}

void PicProcessorWhiteBalance::SetPatchCoord(int x, int y)
{
	dcList = wxString::Format("cross,%d,%d;",x,y);
	m_display->SetDrawList(dcList);
	m_display->Refresh();
	m_display->Update();
	((WhiteBalancePanel *) toolpanel)->setPatch(patch);
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
	SetPatchCoord(patch.x, patch.y);
	event.Skip();
}

void PicProcessorWhiteBalance::createPanel(wxSimplebook* parent)
{
	toolpanel = new WhiteBalancePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}



enum optypes {
	automatic,
	camera,
	imgpatch,
	multipliers
};

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
	std::vector<double> wbmults;
	int patchx, patchy; double patchrad;
	((wxFrame*) m_display->GetParent())->SetStatusText("white balance...");

	optypes optype = automatic; 
	wxArrayString p = split(c, ",");
	if (p.GetCount() > 0) {
		if (p[0].Find(".") != wxNOT_FOUND) {  //float multipliers
			redmult = atof(p[0]);
			greenmult = atof(p[1]);
			bluemult = atof(p[2]);
			optype = multipliers;
		}
		else if (p[0].IsNumber()) { //patch, without 'patch'
			patchx = atoi(p[0]);
			patchy = atoi(p[1]);
			patchrad =  atof(p[2]);
			optype = imgpatch;
		}
		else if (p[0] == "patch"){  //patch, with 'patch' parameter
			patchx = atoi(p[1]);
			patchy = atoi(p[2]);
			patchrad =  atof(p[3]);
			optype = imgpatch;
		}
		else if (p[0] == "auto") { //auto 
			optype = automatic;
		}
	}
	else {
		redmult = 1.0;
		greenmult = 1.0;
		bluemult = 1.0;
		optype = multipliers;
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
	if (optype == multipliers) {
		wbmults = dib->ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
		//wxMessageBox("wb: multipliers");
	}
	else if (optype == imgpatch) {
		wbmults = dib->ApplyWhiteBalance((unsigned) patchx, (unsigned) patchy, patchrad, threadcount);
		//wxMessageBox("wb: patch");
	}
	else {
		wbmults = dib->ApplyWhiteBalance(threadcount);
		//wxMessageBox("wb: auto");
	}
	((WhiteBalancePanel *) toolpanel)->setMultipliers(wbmults);
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.whitebalance.log","0") == "1"))
		log(wxString::Format("tool=whitebalance,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	//((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





