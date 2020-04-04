
#include "PicProcessor.h"
#include "PicProcessorBlackWhitePoint.h"
#include "PicProcPanel.h"
#include "myDoubleSlider.h"
#include "myFloatCtrl.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "undo.xpm"
#include "copy.xpm"
#include "paste.xpm"
#include "util.h"

#define BLACKWHITEENABLE 6100
#define BLACKWHITEAUTORECALC 6101
#define BLACKWHITERECALC 6102
#define BLACKWHITESLIDER 6103
#define BLACKWHITESLIDERAUTO 6104
#define BLACKWHITEDATA   6105
#define BLACKWHITEMINWHITE   6106
#define BLACKWHITENORM   6107
#define BLACKWHITECAMERA 6108
#define BLACKWHITECOPY 6109
#define BLACKWHITEPASTE 6110

class BlackWhitePointPanel: public PicProcPanel
{
	public:
		BlackWhitePointPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			int blk=0, wht=255;
			
			//int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str());
			//int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());

			//parm tool.blackwhitepoint.autorecalcdefault: 0/1, default setting for recalc when the tool is added.  Default=0
			int recalcdefault = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.autorecalcdefault","0").c_str());


			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 2); //.Expand();

			SetSize(parent->GetSize());
			
			enablebox = new wxCheckBox(this, BLACKWHITEENABLE, _("black/white:"));
			enablebox->SetValue(true);

			slideb = new wxRadioButton(this, BLACKWHITESLIDER, _("auto/slider:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			datb   = new wxRadioButton(this, BLACKWHITEDATA,   _("data:"));
			camb   = new wxRadioButton(this, BLACKWHITECAMERA, _("camera:"));
			normb  = new wxRadioButton(this, BLACKWHITENORM, _("normalize:"));
			
			bwpoint = new myDoubleSlider(this, wxID_ANY, blk, wht, 0, 255);
			//parm tool.blackwhitepoint.floatlabel: 0|1, if 1, turns label into a fractional value of the maxvalue.  Default=0
			if (myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.floatlabel","0") == "1")
				bwpoint->SetFloatLabel(true);
			recalc = new wxCheckBox(this, BLACKWHITEAUTORECALC, _("auto recalc"));
			if (recalcdefault) recalc->SetValue(true);

			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);

			bwmode = BLACKWHITESLIDER;
			slideb->SetValue(true);
			blk = 0.0;
			wht = 255.0;

			normmin = new myFloatCtrl(this, wxID_ANY, "min: ", 0.0, 3, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			normmax = new myFloatCtrl(this, wxID_ANY, "max: ", 1.0, 3, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));

			gImage &img = proc->getPreviousPicProcessor()->getProcessedPic();
			std::map<std::string,float> s = img.StatsMap();
			double datblk = fmin(fmin(s["rmin"],s["gmin"]),s["bmin"]);
			double datwht = fmax(fmax(s["rmax"],s["gmax"]),s["bmax"]);
			double minwht = fmin(fmin(s["rmax"],s["gmax"]),s["bmax"]);
			int librawblk = atoi(img.getInfoValue("Libraw.Black").c_str());
			double camblk = librawblk / 65536.0; 
			int librawwht = atoi(img.getInfoValue("Libraw.Maximum").c_str());
			double camwht = librawwht / 65536.0; 

			minwhite = new wxCheckBox(this, BLACKWHITEMINWHITE, _("min white:"));
			datvals= new wxStaticText(this, wxID_ANY, wxString::Format(_("black: %f\nwhite: %f"),datblk, datwht));
			datminwht = new wxStaticText(this, wxID_ANY, wxString::Format("%f",minwht));
			
			setControls(params);
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, BLACKWHITECOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, BLACKWHITEPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(chan, wxSizerFlags(0).Right().Border(wxRIGHT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(slideb, flags);
			m->NextRow();
			m->AddRowItem(bwpoint, flags);
			m->NextRow();
			m->AddRowItem(recalc, flags);
			m->AddRowItem(new wxButton(this, BLACKWHITERECALC, "recalc"), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(datb, flags);
			m->AddRowItem(datvals, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "               "),flags);
			m->AddRowItem(minwhite, flags);
			m->AddRowItem(datminwht, flags.CenterVertical());

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(camb, flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, wxString::Format(_("black: %f (%d)\nwhite: %f (%d)"),camblk, librawblk, camwht,librawwht)), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(normb, flags);
			m->NextRow();
			m->AddRowItem(normmin, flags);
			m->AddRowItem(normmax, flags);
			m->End();

			SetSizerAndFit(m);
			m->Layout();
			bwpoint->Refresh();
			Refresh();
			Update();
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_RADIOBUTTON, &BlackWhitePointPanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnButton, this, BLACKWHITERECALC);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnCopy, this, BLACKWHITECOPY);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnPaste, this, BLACKWHITEPASTE);
			Bind(wxEVT_SCROLL_CHANGED, &BlackWhitePointPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &BlackWhitePointPanel::OnThumbTrack, this);
			Bind(wxEVT_CHOICE, &BlackWhitePointPanel::channelChanged, this);
			Bind(wxEVT_CHECKBOX, &BlackWhitePointPanel::OnCheckBox, this, BLACKWHITEAUTORECALC);
			Bind(wxEVT_CHECKBOX, &BlackWhitePointPanel::onEnable, this, BLACKWHITEENABLE);
			Bind(wxEVT_CHECKBOX, &BlackWhitePointPanel::OnMinWhite, this, BLACKWHITEMINWHITE);
			Bind(wxEVT_TIMER, &BlackWhitePointPanel::OnTimer,  this);
			Bind(wxEVT_SET_FOCUS, &BlackWhitePointPanel::handleFocus, this);
			Bind(wxEVT_CHAR_HOOK, &BlackWhitePointPanel::OnKey,  this);
			Thaw();
		}
		
		void setControls(wxString params)
		{
			std::map<std::string,std::string> param = parse_blackwhitepoint(params.ToStdString());
			if (paramexists(param, "channel")) chan->SetStringSelection(wxString(param["channel"]));
			if (param["mode"] == "auto") {
				bwmode = BLACKWHITESLIDERAUTO;
				slideb->SetValue(true);
			}
			else if (param["mode"] == "data") {
				bwmode = BLACKWHITEDATA;
				datb->SetValue(true);
				if (param["minwhite"] == "true") minwhite->SetValue(true);
			}
			else if (param["mode"] == "camera") {
				bwmode = BLACKWHITECAMERA;
				camb->SetValue(true);
			}
			else if (param["mode"] == "norm") {
				bwmode = BLACKWHITENORM;
				normb->SetValue(true);
				normmin->SetFloatValue(atof(param["black"].c_str()));
				normmax->SetFloatValue(atof(param["white"].c_str()));
			}
			else if (param["mode"] == "values") {
				bwmode = BLACKWHITESLIDER;
				slideb->SetValue(true);
				bwpoint->SetLeftValue(atof(param["black"].c_str()));
				bwpoint->SetRightValue(atof(param["white"].c_str()));
			}
			else {
				bwmode = BLACKWHITESLIDER;
				slideb->SetValue(true);
			}
		}

		void handleFocus(wxFocusEvent& event)
		{
			printf("BlackWhitePointPanel: gain focus...\n"); fflush(stdout);
			GetParent()->SetFocus();
			event.Skip();
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
				double blk=0.0, wht=255.0;
				setControls(q->getParams());
				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
			}
			else wxMessageBox(wxString::Format(_("Invalid Paste")));
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			bwmode = event.GetId();
			processBW();
		}

		void OnMinWhite(wxCommandEvent& event)
		{
			processBW();
		}

		void processBW()
		{
			switch (bwmode) {
				case BLACKWHITESLIDERAUTO:
					q->setParams("rgb");
					q->processPic();
					break;
				case BLACKWHITESLIDER:
					q->setParams(wxString::Format("%s,%d,%d",chan->GetString(chan->GetSelection()), bwpoint->GetLeftValue(),bwpoint->GetRightValue()));
					q->processPic();
					break;
				case BLACKWHITEDATA:
					if (minwhite->GetValue())
						q->setParams(wxString::Format("%s,data,minwhite",  chan->GetString(chan->GetSelection())));
					else
						q->setParams(wxString::Format("%s,data",  chan->GetString(chan->GetSelection())));
					q->processPic();
					break;
				case BLACKWHITECAMERA:
					q->setParams("camera");
					q->processPic();
					break;
				case BLACKWHITENORM:
					q->setParams(wxString::Format("norm,%0.3f,%0.3f",normmin->GetFloatValue(),normmax->GetFloatValue()));
					q->processPic();
					break;
			}
		}

		void updateVals(float black, float white, float minwhite)
		{
			datvals->SetLabel(wxString::Format("black: %f\nwhite: %f",black, white));
			datminwht->SetLabel(wxString::Format("%f",minwhite));
			Refresh();
		}

		void OnChanged(wxCommandEvent& event)
		{
			if (bwmode == BLACKWHITESLIDERAUTO) bwmode == BLACKWHITESLIDER;
			if (bwmode == BLACKWHITESLIDER) t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{

		}
		
		void OnCheckBox(wxCommandEvent& event)
		{
			((PicProcessorBlackWhitePoint *) q)->setReCalc(recalc->GetValue());
			processBW();
			event.Skip();
		}

		void OnTimer(wxTimerEvent& event)
		{
			processBW();
			event.Skip();
		}

		void updateSliders()
		{
			wxArrayString p = split(q->getParams(),",");
			bwpoint->SetLeftValue(atoi(p[1].c_str()));
			bwpoint->SetRightValue(atoi(p[2].c_str()));
		}
		
		void channelChanged(wxCommandEvent& event)
		{
			gImage &img = q->getPreviousPicProcessor()->getProcessedPic();
			std::map<std::string,float> s;
			double datblk, datwht, minwht;
			wxString chanstr = chan->GetString(chan->GetSelection());
			switch (bwmode) {
				case BLACKWHITESLIDERAUTO:
					q->setParams(wxString::Format("%s,%d,%d",chan->GetString(chan->GetSelection()),0,255));
					((PicProcessorBlackWhitePoint *) q)->setChannel(chan->GetString(chan->GetSelection()));
					((PicProcessorBlackWhitePoint *) q)->reCalc();
					updateSliders();
					processBW();
					break;
				case BLACKWHITEDATA:
					s = img.StatsMap();
					if (chanstr =="rgb" | chanstr == "tone") {
						datblk = fmin(fmin(s["rmin"],s["gmin"]),s["bmin"]);
						datwht = fmax(fmax(s["rmax"],s["gmax"]),s["bmax"]);
						minwht = fmin(fmin(s["rmax"],s["gmax"]),s["bmax"]);
					}
					else if (chanstr =="red") {
						datblk = s["rmin"];
						datwht = s["rmax"];
						minwht = s["rmax"];
					}
					else if (chanstr =="green") {
						datblk = s["gmin"];
						datwht = s["gmax"];
						minwht = s["gmax"];
					}
					else if (chanstr =="blue") {
						datblk = s["bmin"];
						datwht = s["bmax"];
						minwht = s["bmax"];
					}
					updateVals(datblk, datwht, minwht);
					if (minwhite->GetValue())
						q->setParams(wxString::Format("%s,data,minwhite",chan->GetString(chan->GetSelection())));
					else
						q->setParams(wxString::Format("%s,data",chan->GetString(chan->GetSelection())));
					processBW();
					break;
			}
		}

		void OnButton(wxCommandEvent& event)
		{
			((PicProcessorBlackWhitePoint *) q)->reCalc();
			updateSliders();
			processBW();
			event.Skip();
		}


	private:
		wxChoice *chan;
		wxSlider *black, *white;
		wxCheckBox *recalc,*enablebox, *minwhite;
		wxRadioButton *slideb, *datb, *camb, *normb;
		myDoubleSlider *bwpoint;
		myFloatCtrl *normmin, *normmax;
		wxStaticText *val1, *val2, *datvals, *datminwht;
		wxBitmapButton *btn1, * btn2;
		int bwmode;
		wxTimer t;

};


PicProcessorBlackWhitePoint::PicProcessorBlackWhitePoint(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	int i;
	double blk, wht;
	wht = 255; blk = 0;
	recalc = false;

	//parm tool.blackwhitepoint.blackthreshold: The percent threshold used by the auto algorithm for the black adjustment. Only used when the blackwhitepoint tool is created. Default=0.05
	double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.05").c_str());
	//parm tool.blackwhitepoint.whitethreshold: The percent threshold used by the auto algorithm for the white adjustment. Only used when the blackwhitepoint tool is created. Default=0.05
	double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.05").c_str());
	//parm tool.blackwhitepoint.whiteinitialvalue: The initial whitepoint setting, or the starting point in the histogram for walking down to the white threshold in auto.  Use to bypass bunched clipped highlights.  Default=255
	long whtinitial = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());
	//parm tool.blackwhitepoint.automode: The calculation mode for auto.  tone=find the points based on the tone; min=find the points based on the min of the channel maxes. Default: tone
	wxString automode = myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.automode","tone").c_str();
	
	if (command == "") {
		std::vector<double> bwpts;
		if (automode=="min")
			bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, "min");
		else
			bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial);
		blk = bwpts[0];
		wht = bwpts[1];
		setParams(wxString::Format("rgb,%d,%d",(unsigned) blk, (unsigned) wht));
	}
	else {
		wxArrayString p = split(command,",");
		if ((p[0] == "rgb") | (p[0] == "red") | (p[0] == "green") | (p[0] == "blue")) {
			if (p.GetCount() == 1) {
				std::vector<double> bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, std::string(p[0].c_str()));
				blk = bwpts[0];
				wht = bwpts[1];
				setParams(wxString::Format("%s,%d,%d",p[0],(unsigned) blk, (unsigned) wht));
			}
			else {
				setParams(wxString::Format("%s,%s,%s",p[0],p[1],p[2]));
				
			}
		}
//		setParams(command);
	}
}

void PicProcessorBlackWhitePoint::createPanel(wxSimplebook* parent)
{
	toolpanel = new BlackWhitePointPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

void PicProcessorBlackWhitePoint::setReCalc(bool r)
{
	recalc = r;
	if (recalc) reCalc();
}

void PicProcessorBlackWhitePoint::reCalc()
{
	double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.05").c_str());
	double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.05").c_str());
	long whtinitial = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());

	wxArrayString p = split(getParams(),",");
	std::vector<double> bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, std::string(p[0].c_str()));
	double blk = bwpts[0];
	double wht = bwpts[1];
	setParams(wxString::Format("%s,%d,%d",p[0],(unsigned) blk, (unsigned) wht));
}

bool PicProcessorBlackWhitePoint::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("black/white point..."));
	bool ret = true;
	std::map<std::string,std::string> result;
	dib = processdib;

	//if (!global_processing_enabled) r= true;  //ToDo: get rid of global_processing_enabled

	if (recalc) {
		reCalc();
		((BlackWhitePointPanel *) toolpanel)->updateSliders();
	}

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();
	if (!pstr.empty())
		params = parse_blackwhitepoint(std::string(pstr));
	else
		params = parse_blackwhitepoint("rgb,auto");
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_blackwhitepoint(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(params["error"]);
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, result["treelabel"]);
			m_display->SetModified(true);
			//parm tool.all.log: Turns on logging for all tools.  Default=0
			//parm tool.*.log: Turns on logging for the specified tool.  Default=0
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.log","0") == "1"))
					log(wxString::Format(_("tool=blackwhitepoint,%s,imagesize=%dx%d,threads=%s,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["threadcount"].c_str(),
						result["duration"].c_str())
					);
		}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}


void PicProcessorBlackWhitePoint::displayProcessedPic() 
{
	if (m_display) {
		m_display->SetPic(dib, channel);
//		m_display->SetDrawList(dcList);
	}
}


