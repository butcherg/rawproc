#include "PicProcessorWhiteBalance.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
//#include "gimage/strutil.h"
#include "gimage/curve.h"
#include "myFloatCtrl.h"
#include <wx/spinctrl.h>
#include <wx/clipbrd.h>
#include "unchecked.xpm"
#include "checked.xpm"
#include "undo.xpm"

#define WBENABLE 6400
#define WBMANUAL 6401
#define WBAUTO 6402
#define WBPATCH 6403
#define WBCAMERA 6404
#define WBORIGINAL 6405

#define WBRED 6406
#define WBGREEN 6407
#define WBBLUE 6408


class WhiteBalancePanel: public PicProcPanel
{

	public:
		WhiteBalancePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			double rm, gm, bm;
			wxSize spinsize(130, TEXTCTRLHEIGHT);
			
			//wxArrayString parm = split(params, ",");

			//parm tool.whitebalance.min: (float), minimum multiplier value.  Default=0.001
			double min = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.min","0.001").c_str());
			//parm tool.whitebalance.max: (float), maximum multiplier value.  Default=3.0
			double max = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.max","3.0").c_str());
			//parm tool.whitebalance.digits: (float), number of fractional digits.  Default=3
			double digits = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.digits","3.0").c_str());
			//parm tool.whitebalance.increment: (float), maximum multiplier value.  Default=0.001
			double increment = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.increment","0.001").c_str());

			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();

			enablebox = new wxCheckBox(this, WBENABLE, "white balance:");
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxGBSpan(1,2), wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			g->Add(new wxStaticText(this,wxID_ANY, "red mult:"), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			rmult = new myFloatCtrl(this, wxID_ANY, 1.0, 3, wxDefaultPosition, spinsize);
			g->Add(rmult, wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "green mult:"), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			gmult = new myFloatCtrl(this, wxID_ANY, 1.0, 3, wxDefaultPosition, spinsize);
			g->Add(gmult, wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "blue mult:"), wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			bmult = new myFloatCtrl(this, wxID_ANY, 1.0, 3, wxDefaultPosition, spinsize);
			g->Add(bmult, wxGBPosition(4,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset multipliers to original values");
			g->Add(btn, wxGBPosition(4,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(0,10, wxGBPosition(5,0));

			//Operator radio buttons:
			ob = new wxRadioButton(this, WBORIGINAL, "Multipliers:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
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
			

			//if camera multipliers are available in the metadata:
			camr = 1.0; camg = 1.0; camb = 1.0;
			std::vector<double> cam_mults = ((PicProcessorWhiteBalance *)proc)->getCameraMultipliers();
			if (cam_mults.size() >= 2) {
				camr = cam_mults[0];
				camg = cam_mults[1];
				camb = cam_mults[2];
				camera->SetLabel(wxString::Format("%0.3f,%0.3f,%0.3f",camr,camg,camb));
				cb->Enable(true);
			}

			//parse parameters:
			params.Trim();
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
				ab->Enable(true);
				ab->SetValue(true);
			}
			else if (p[0] == "camera") {
				cb->Enable(true);
				cb->SetValue(true);
			}
			else if (p[0] == "patch") {
				patx = atoi(p[1]);
				paty = atoi(p[2]);
				patrad = atof(p[3]);
				patch->SetLabel(wxString::Format("x:%d y:%d r:%0.1f",patx, paty, patrad));
				pb->Enable(true);
				pb->SetValue(true);
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

			else 
				wxMessageBox("Error: ill-formed param string");



		
			SetSizerAndFit(g);
			g->Layout();
			SetFocus();
			t = new wxTimer(this);

			Bind(wxEVT_TIMER, &WhiteBalancePanel::OnTimer, this);
			Bind(wxEVT_RADIOBUTTON, &WhiteBalancePanel::OnButton, this);
			Bind(myFLOATCTRL_CHANGE,&WhiteBalancePanel::onWheel, this);
			Bind(myFLOATCTRL_UPDATE,&WhiteBalancePanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &WhiteBalancePanel::onEnable, this, WBENABLE);
			Bind(wxEVT_BUTTON, &WhiteBalancePanel::OnReset, this);

			Refresh();
			Update();
		}

		~WhiteBalancePanel()
		{
			t->~wxTimer();
		}
		
		void OnReset(wxCommandEvent& event)
		{
			setMultipliers(orgr, orgg, orgb);
			q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",rmult->GetFloatValue(), gmult->GetFloatValue(), bmult->GetFloatValue()));
			ob->SetValue(true);
			ab->SetValue(false);
			pb->SetValue(false);
			cb->SetValue(false);
			q->processPic();
			
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

		//used by PicProcessorWhiteBalance to initialize panel:
		void setMultipliers(double rm, double gm, double bm)
		{
			rmult->SetFloatValue(rm);
			gmult->SetFloatValue(gm);
			bmult->SetFloatValue(bm);
			Refresh();
		}

		void setMultipliers(std::vector<double> mults)
		{
			rmult->SetFloatValue(mults[0]);
			gmult->SetFloatValue(mults[1]);
			bmult->SetFloatValue(mults[2]);
			Refresh();
		}



		void processWB(int src)
		{
			switch (src) {
				case WBAUTO:
					q->setParams("auto");
					break;
				case WBCAMERA:
					q->setParams("camera");
					break;
				case WBPATCH:
					q->setParams(wxString::Format("patch,%d,%d,%0.1f",patx, paty, patrad));
					break;
				case WBORIGINAL:
					q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",orgr, orgg, orgb));
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

		void clearSelectors()
		{
			ob->SetValue(true);
			ab->SetValue(false);
			pb->SetValue(false);
			cb->SetValue(false);
		}

		void paramChanged(wxCommandEvent& event)
		{
			ob->SetValue(true);
			ab->SetValue(false);
			pb->SetValue(false);
			cb->SetValue(false);
			q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",rmult->GetFloatValue(), gmult->GetFloatValue(), bmult->GetFloatValue()));
			q->processPic();
			Refresh();
		}
		
		void onWheel(wxCommandEvent& event)
		{
			ob->SetValue(true);
			ab->SetValue(false);
			pb->SetValue(false);
			cb->SetValue(false);
			q->setParams(wxString::Format("%0.3f,%0.3f,%0.3f",rmult->GetFloatValue(), gmult->GetFloatValue(), bmult->GetFloatValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->processPic();
			Refresh();
		}

		void OnButton(wxCommandEvent& event)
		{
			processWB(event.GetId());
		}

	private:
		wxStaticText *origwb, *autowb, *patch, *camera;
		wxRadioButton *ob, *ab, *pb, *cb;
		wxCheckBox *enablebox;
		myFloatCtrl *rmult, *gmult ,*bmult;
		wxBitmapButton *btn;
		wxTimer *t;
		unsigned patx, paty;
		double patrad;
		double orgr, orgg, orgb; //original multipliers, none if new tool
		double camr, camg, camb; //camera 'as-shot' multipliers

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
		if (p[0] == "patch"){  //patch, with 'patch' parameter
			patchx = atoi(p[1]);
			patchy = atoi(p[2]);
			patchrad =  atof(p[3]);
			optype = imgpatch;
		}
		else if (p[0] == "auto") { //auto 
			optype = automatic;
		}
		else if (p[0] == "camera") { //camera, to mults
			std::vector<double> mults =  PicProcessorWhiteBalance::getCameraMultipliers();
			if (mults.size() >= 3) {
				redmult = mults[0];
				greenmult = mults[1];
				bluemult = mults[2];
				optype = camera;
			}
			else {
				redmult = 1.0;
				greenmult = 1.0;
				bluemult = 1.0;
				optype = multipliers;
			}
		}
		else if (p[0].Find(".") != wxNOT_FOUND) {  //float multipliers
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

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) {
		mark();
		if (dib->getInfoValue("LibrawMosaiced") == "1") {
			if (dib->getInfoValue("LibrawCFAPattern") != "") {
				wbmults = dib->ApplyCameraWhiteBalance(redmult, greenmult, bluemult, threadcount);
			}
			else {
				wxMessageBox("Error: No bayer pattern available in metadata (LibrawCFAPattern is empty)");
				((WhiteBalancePanel *) toolpanel)->clearSelectors();
				wbmults = {1.0,1.0,1.0};
			}
		}
		else {
			if (optype == multipliers | optype == camera) {
				wbmults = dib->ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
			}
			else if (optype == imgpatch) {
				wbmults = dib->ApplyWhiteBalance((unsigned) patchx, (unsigned) patchy, patchrad, threadcount);
			}
			else if (optype == automatic) {
				wbmults = dib->ApplyWhiteBalance(threadcount);
			}
		}

		wxString d = duration();
		((WhiteBalancePanel *) toolpanel)->setMultipliers(wbmults);


		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.whitebalance.log","0") == "1"))
			log(wxString::Format("tool=whitebalance,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	//((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





